"""Regression checks for Joy button edges and independent axis sampling."""

import importlib.util
import os
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock

import pytest
import rclpy
from sensor_msgs.msg import Joy

SCRIPT = Path(__file__).parents[1] / "scripts" / "ardusub_manual_control.py"
spec = importlib.util.spec_from_file_location(
    "manual_control_under_test", os.environ.get("POSIM_MANUAL_CONTROL_UNDER_TEST", SCRIPT)
)
manual = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manual)


class TimePoint:
    def __init__(self, seconds):
        self.nanoseconds = int(seconds * 1e9)

    def __sub__(self, other):
        return SimpleNamespace(nanoseconds=self.nanoseconds - other.nanoseconds)


class Clock:
    seconds = 0.0

    def now(self):
        return TimePoint(self.seconds)


@pytest.fixture
def control():
    rclpy.init()
    node = manual.ArduSubManualControl()
    node.timer.cancel()
    clock = Clock()
    node.get_clock = lambda: clock
    node._call_arm = Mock()
    node._call_set_mode = Mock()
    node._publish_manual = Mock()
    yield node, clock
    node.destroy_node()
    rclpy.shutdown()


def joy(buttons=(), axes=()):
    msg = Joy()
    msg.axes = list(axes)
    msg.buttons = [int(i in buttons) for i in range(17)]
    return msg


@pytest.mark.parametrize("source", ["joystick", "keyboard"])
@pytest.mark.parametrize(
    "button,method,value",
    [
        (manual.BTN_ARM_ON, "_call_arm", True),
        (manual.BTN_ARM_OFF, "_call_arm", False),
        (manual.BTN_Z_HOLD_ON, "_call_set_mode", "ALT_HOLD"),
        (manual.BTN_Z_HOLD_OFF, "_call_set_mode", "STABILIZE"),
    ],
)
def test_press_and_release_between_timer_ticks(control, source, button, method, value):
    node, _ = control
    node._update_input_state(source, joy([button]))
    node._update_input_state(source, joy())
    node.tick()
    getattr(node, method).assert_called_once_with(value)


def test_held_button_and_idle_other_source_do_not_repeat(control):
    node, clock = control
    for _ in range(20):
        node.cb_joy(joy([manual.BTN_ARM_ON]))
        node.cb_keyboard_joy(joy())
        node.tick()
    clock.seconds = 1.0
    node.tick()
    node.cb_joy(joy([manual.BTN_ARM_ON]))
    node._call_arm.assert_called_once_with(True)
    node.cb_joy(joy())
    node.cb_joy(joy([manual.BTN_ARM_ON]))
    assert node._call_arm.call_count == 2


def test_button_history_is_per_source(control):
    node, _ = control
    for _ in range(3):
        node.cb_joy(joy([manual.BTN_Z_HOLD_OFF]))
        node.cb_keyboard_joy(joy([manual.BTN_Z_HOLD_OFF]))
    assert node._call_set_mode.call_count == 2
    node.cb_keyboard_joy(joy())
    node.cb_keyboard_joy(joy([manual.BTN_Z_HOLD_OFF]))
    assert node._call_set_mode.call_count == 3


def test_rapid_separate_presses_are_not_collapsed(control):
    node, _ = control
    for _ in range(5):
        node.cb_keyboard_joy(joy([manual.BTN_Z_HOLD_OFF]))
        node.cb_keyboard_joy(joy())
    assert node._call_set_mode.call_count == 5
    node.tick()
    assert node._call_set_mode.call_count == 5


def test_disarm_wins_when_both_arm_buttons_are_pressed(control):
    node, _ = control
    node.cb_joy(joy([manual.BTN_ARM_ON, manual.BTN_ARM_OFF]))
    node._call_arm.assert_called_once_with(False)


def test_scale_uses_edges_and_stays_bounded(control):
    node, _ = control
    for _ in range(20):
        node.cb_joy(joy([manual.BTN_SCALE_UP]))
    assert node.throttle_scale == 0.75
    for _ in range(20):
        node.cb_joy(joy())
        node.cb_joy(joy([manual.BTN_SCALE_UP]))
    assert node.throttle_scale == 1.0
    for _ in range(20):
        node.cb_joy(joy())
        node.cb_joy(joy([manual.BTN_SCALE_DN]))
    assert node.throttle_scale == 0.25


def test_empty_short_and_unmapped_buttons(control):
    node, _ = control
    node.cb_joy(Joy())
    node.cb_joy(joy([16]))
    node.cb_joy(Joy())
    node._call_arm.assert_not_called()
    node._call_set_mode.assert_not_called()


def test_axes_and_timeout_are_unchanged(control):
    node, clock = control
    node.cb_keyboard_joy(joy(axes=[0.0, -0.8, 0.0, 0.0, 0.0, 0.0]))
    node.tick()
    assert node._publish_manual.call_args.args == pytest.approx((400.0, 0.0, 500.0, 0.0))
    clock.seconds = 0.1
    node.cb_joy(joy(axes=[0.0, 0.8, 0.0, 0.0, 0.0, 0.0]))
    node.tick()
    assert node._publish_manual.call_args.args == pytest.approx((-400.0, 0.0, 500.0, 0.0))
    clock.seconds = 0.5
    node.tick()
    node._publish_manual.assert_called_with(0.0, 0.0, 500.0, 0.0)


def test_axis_release_does_not_wait_for_timeout(control):
    node, _ = control
    node.cb_keyboard_joy(joy(axes=[0.0, -0.8, 0.0, 0.0, 0.0, 0.0]))
    node.cb_keyboard_joy(joy())
    node.tick()
    node._publish_manual.assert_called_with(0.0, 0.0, 500.0, 0.0)


def test_held_disarm_blocks_a_new_arm_edge(control):
    node, _ = control
    node.cb_joy(joy([manual.BTN_ARM_OFF]))
    node.cb_joy(joy([manual.BTN_ARM_OFF, manual.BTN_ARM_ON]))
    node._call_arm.assert_called_once_with(False)
