"""Bounded command handoff, failure handling and request-specific logging."""

import importlib.util
import os
from concurrent.futures import Future
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock

import pytest
import rclpy

SCRIPT = Path(__file__).parents[1] / "scripts" / "ardusub_manual_control.py"
spec = importlib.util.spec_from_file_location(
    "manual_services_under_test", os.environ.get("POSIM_MANUAL_CONTROL_UNDER_TEST", SCRIPT)
)
manual = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manual)


class Client:
    def __init__(self, field):
        self.field = field
        self.ready = True
        self.values = []
        self.futures = []
        self.send_error = None

    def service_is_ready(self):
        return self.ready

    def call_async(self, request):
        if self.send_error:
            raise self.send_error
        self.values.append(getattr(request, self.field))
        future = Future()
        self.futures.append(future)
        return future


@pytest.fixture
def control(monkeypatch):
    rclpy.init()
    node = manual.ArduSubManualControl()
    node.timer.cancel()
    clock = SimpleNamespace(seconds=0.0)
    monkeypatch.setattr(manual, "time", SimpleNamespace(monotonic=lambda: clock.seconds))
    node.mode_client = Client("custom_mode")
    node.arm_client = Client("value")
    log = Mock(spec=node.get_logger())
    node.get_logger = lambda: log
    yield node, clock, log
    node.destroy_node()
    rclpy.shutdown()


def result(ok=True):
    return SimpleNamespace(mode_sent=ok, success=ok)


def test_mode_latest_wins_and_log_matches_actual_request(control):
    node, _, log = control
    node._call_set_mode("STABILIZE")
    node._call_set_mode("ALT_HOLD")
    node._call_set_mode("MANUAL")
    assert node.mode_client.values == ["STABILIZE"]
    node.mode_client.futures[0].set_result(result())
    assert node.mode_client.values == ["STABILIZE", "MANUAL"]
    log.info.assert_called_with("Requested ArduSub mode: STABILIZE")
    node.mode_client.futures[1].set_result(result())
    log.info.assert_called_with("Requested ArduSub mode: MANUAL")


def test_selecting_inflight_mode_cancels_other_pending_mode(control):
    node, _, _ = control
    node._call_set_mode("STABILIZE")
    node._call_set_mode("ALT_HOLD")
    node._call_set_mode("STABILIZE")
    node.mode_client.futures[0].set_result(result())
    assert node.mode_client.values == ["STABILIZE"]


def test_disarm_is_preserved_and_arm_is_not_queued(control):
    node, _, _ = control
    node._call_arm(True)
    node._call_arm(False)
    node._call_arm(True)
    node.arm_client.futures[0].set_result(result())
    assert node.arm_client.values == [True, False]
    node.arm_client.futures[1].set_result(result())
    assert node.arm_client.values == [True, False]


def test_arm_during_pending_disarm_needs_fresh_press(control):
    node, _, _ = control
    node._call_arm(False)
    node._call_arm(True)
    node.arm_client.futures[0].set_result(result())
    assert node.arm_client.values == [False]
    node._call_arm(True)
    assert node.arm_client.values == [False, True]


def test_repeated_disarm_does_not_duplicate_inflight_disarm(control):
    node, _, _ = control
    node._call_arm(False)
    for _ in range(5):
        node._call_arm(False)
    node.arm_client.futures[0].set_result(result())
    assert node.arm_client.values == [False]


@pytest.mark.parametrize("kind", ["mode", "arm"])
@pytest.mark.parametrize("expire_in_tick", [False, True])
def test_waiting_mode_expires_but_disarm_is_preserved(control, kind, expire_in_tick):
    node, clock, log = control
    if kind == "mode":
        node._call_set_mode("STABILIZE")
        node._call_set_mode("ALT_HOLD")
        client = node.mode_client
    else:
        node._call_arm(True)
        node._call_arm(False)
        client = node.arm_client
    clock.seconds = manual.PENDING_COMMAND_TTL_SEC + 0.01
    if expire_in_tick:
        node.tick()
    client.futures[0].set_result(result())
    if kind == "mode":
        assert client.values == ["STABILIZE"]
        assert any("expired" in str(call) for call in log.warning.call_args_list)
    else:
        assert client.values == [True, False]
        assert any("still pending" in str(call) for call in log.warning.call_args_list)


@pytest.mark.parametrize("kind", ["mode", "arm"])
@pytest.mark.parametrize("failure", ["reject", "exception"])
def test_failed_request_is_not_retried_but_fresh_pending_intent_is_sent(control, kind, failure):
    node, _, log = control
    if kind == "mode":
        node._call_set_mode("STABILIZE")
        node._call_set_mode("ALT_HOLD")
        client = node.mode_client
        expected = ["STABILIZE", "ALT_HOLD"]
    else:
        node._call_arm(True)
        node._call_arm(False)
        client = node.arm_client
        expected = [True, False]
    if failure == "reject":
        client.futures[0].set_result(result(False))
    else:
        client.futures[0].set_exception(RuntimeError("fixture failure"))
    assert client.values == expected
    assert log.warning.called
    client.futures[1].set_result(result())
    assert client.values == expected


@pytest.mark.parametrize("kind", ["mode", "arm"])
def test_unavailable_service_is_not_queued_for_later_execution(control, kind):
    node, _, log = control
    client = node.mode_client if kind == "mode" else node.arm_client
    call = (
        (lambda: node._call_set_mode("ALT_HOLD"))
        if kind == "mode"
        else lambda: node._call_arm(True)
    )
    client.ready = False
    call()
    client.ready = True
    node.tick()
    assert client.values == []
    call()
    assert len(client.values) == 1
    assert log.warning.called


@pytest.mark.parametrize("kind", ["mode", "arm"])
def test_synchronous_send_failure_does_not_wedge_client(control, kind):
    node, _, _ = control
    client = node.mode_client if kind == "mode" else node.arm_client
    call = (
        (lambda: node._call_set_mode("ALT_HOLD"))
        if kind == "mode"
        else lambda: node._call_arm(False)
    )
    client.send_error = RuntimeError("send failed")
    call()
    client.send_error = None
    call()
    assert len(client.values) == 1


def test_no_timeout_retry_when_service_never_responds(control):
    node, clock, log = control
    node._call_arm(True)
    node._call_arm(False)
    node._call_set_mode("STABILIZE")
    node._call_set_mode("ALT_HOLD")
    clock.seconds = 10.0
    for _ in range(20):
        node.tick()
    assert node.arm_client.values == [True]
    assert node.mode_client.values == ["STABILIZE"]
    assert node._pending_mode is None
    assert node._pending_disarm is True
    assert log.warning.call_count == 2


def test_very_late_arm_response_is_still_followed_by_disarm(control):
    node, clock, _ = control
    node._call_arm(True)
    node._call_arm(False)
    clock.seconds = 60.0
    node.tick()
    node._call_arm(True)
    node.arm_client.futures[0].set_result(result())
    assert node.arm_client.values == [True, False]
    node.arm_client.futures[1].set_result(result())
    assert node.arm_client.values == [True, False]
