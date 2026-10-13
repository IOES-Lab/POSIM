#!/usr/bin/env python3
import math
import time

import rclpy
from mavros_msgs.msg import ManualControl, State
from mavros_msgs.srv import CommandBool, SetMode
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Joy
from std_msgs.msg import Bool

AXIS_SWAY = 0
AXIS_FWD = 1
AXIS_YAW = 4
AXIS_HEAVE = 5

BTN_Z_HOLD_OFF = 0
BTN_SCALE_UP = 1
BTN_SCALE_DN = 2
BTN_Z_HOLD_ON = 3
BTN_ARM_OFF = 8
BTN_ARM_ON = 9

DEADZONE = 0.08
DEADZONE_HEAVE = 0.18
RATE_HZ = 20.0
TIMEOUT_SEC = 0.3
PENDING_COMMAND_TTL_SEC = 2.0

STABILIZE_MODE = "STABILIZE"
DEPTH_HOLD_MODE = "ALT_HOLD"
MAX_MANUAL = 1000.0
THROTTLE_NEUTRAL = 500.0
THROTTLE_RANGE = 500.0

THROTTLE_LEVELS = [0.25, 0.50, 0.75, 1.00]
THROTTLE_DEFAULT_INDEX = 1


def dz(value, deadzone):
    return 0.0 if abs(value) < deadzone else value


def clamp(value, lo, hi):
    return lo if value < lo else hi if value > hi else value


def resolve_namespace(value):
    cleaned = str(value).strip()
    if not cleaned:
        return ""

    cleaned = cleaned.strip("/")
    return f"/{cleaned}" if cleaned else ""


class ArduSubManualControl(Node):
    """Convert Joy messages to MAVROS ManualControl for ArduSub SITL."""

    def __init__(self):
        super().__init__("ardusub_manual_control")

        self.declare_parameter("model_name", "bluerov2")
        self.declare_parameter("joystick_topic", "/joy")
        self.declare_parameter("keyboard_topic", "/keyboard/joy")
        self.declare_parameter("mavros_namespace", "mavros")

        self.model_name = (
            self.get_parameter("model_name").get_parameter_value().string_value or "bluerov2"
        )
        self.joystick_topic = (
            self.get_parameter("joystick_topic").get_parameter_value().string_value or "/joy"
        )
        self.keyboard_topic = (
            self.get_parameter("keyboard_topic").get_parameter_value().string_value
            or "/keyboard/joy"
        )

        mavros_namespace = (
            self.get_parameter("mavros_namespace").get_parameter_value().string_value or "mavros"
        )
        mavros_ns = resolve_namespace(mavros_namespace)

        manual_control_topic = f"{mavros_ns}/manual_control/send"
        state_topic = f"{mavros_ns}/state"
        arming_service = f"{mavros_ns}/cmd/arming"
        mode_service = f"{mavros_ns}/set_mode"
        armed_topic = f"/model/{self.model_name}/control/armed"

        state_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
        )

        self.create_subscription(Joy, self.joystick_topic, self.cb_joy, 10)
        self.create_subscription(Joy, self.keyboard_topic, self.cb_keyboard_joy, 10)
        self.create_subscription(State, state_topic, self.cb_state, state_qos)

        self.pub_manual = self.create_publisher(ManualControl, manual_control_topic, 10)
        self.pub_armed = self.create_publisher(Bool, armed_topic, state_qos)

        self.arm_client = self.create_client(CommandBool, arming_service)
        self.mode_client = self.create_client(SetMode, mode_service)

        self.input_state = {
            "joystick": {
                "axes": [],
                "buttons": [],
                "activity_time": None,
            },
            "keyboard": {
                "axes": [],
                "buttons": [],
                "activity_time": None,
            },
        }
        self.last_axes = []

        self.connected = False
        self.armed = False
        self.current_mode = ""

        self.throttle_index = THROTTLE_DEFAULT_INDEX
        self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]

        self._last_warn_sec = {}
        self._mode_future = None
        self._arm_future = None
        self._mode_in_flight = None
        self._arm_in_flight = None
        self._pending_mode = None
        self._pending_disarm = False
        self._disarm_warn_at = None

        self._publish_armed_state()
        self.timer = self.create_timer(1.0 / RATE_HZ, self.tick)

        self.get_logger().info(
            "ardusub_manual_control started | "
            f"model={self.model_name}, joy={self.joystick_topic}, "
            f"keyboard={self.keyboard_topic}, mavros_ns={mavros_ns or '/'} | "
            "mode control: external/QGC respected (no forced STABILIZE)"
        )

    def cb_joy(self, msg):
        self._update_input_state("joystick", msg)

    def cb_keyboard_joy(self, msg):
        self._update_input_state("keyboard", msg)

    def cb_state(self, msg):
        was_connected = self.connected
        previous_mode = self.current_mode
        previous_armed = self.armed

        self.connected = bool(msg.connected)
        self.armed = bool(msg.armed)
        self.current_mode = msg.mode

        if self.connected and not was_connected:
            self.get_logger().info("MAVROS connected to ArduSub")
        if previous_mode != self.current_mode and self.current_mode:
            self.get_logger().info(f"ArduSub mode: {self.current_mode}")
        if previous_armed != self.armed:
            self.get_logger().info("ArduSub armed" if self.armed else "ArduSub disarmed")

        self._publish_armed_state()

    def _update_input_state(self, source, msg):
        state = self.input_state[source]
        previous_buttons = state["buttons"]
        state["axes"] = list(msg.axes)
        state["buttons"] = list(msg.buttons)

        if any(abs(axis) > 1e-6 for axis in state["axes"]) or any(state["buttons"]):
            state["activity_time"] = self.get_clock().now()

        # A keyboard pulse can be pressed and released between two 20 Hz ticks.
        # Consume button edges here, keeping history separate for each source.
        self._handle_button_edges(previous_buttons, state["buttons"])

    def _select_active_input(self):
        now = self.get_clock().now()
        active_state = None
        active_time_ns = -1

        for state in self.input_state.values():
            activity_time = state["activity_time"]
            if activity_time is None:
                continue

            age = (now - activity_time).nanoseconds * 1e-9
            if age > TIMEOUT_SEC:
                continue

            if activity_time.nanoseconds > active_time_ns:
                active_state = state
                active_time_ns = activity_time.nanoseconds

        return active_state

    def _get_axis(self, idx):
        return float(self.last_axes[idx]) if idx < len(self.last_axes) else 0.0

    def _publish_armed_state(self):
        msg = Bool()
        msg.data = self.armed
        self.pub_armed.publish(msg)

    def _publish_manual(self, x, y, z, r):
        msg = ManualControl()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.x = float(x)
        msg.y = float(y)
        msg.z = float(z)
        msg.r = float(r)
        msg.buttons = 0
        msg.buttons2 = 0
        msg.enabled_extensions = 0
        self.pub_manual.publish(msg)

    def _warn_throttled(self, key, message, period_sec=2.0):
        now_sec = self.get_clock().now().nanoseconds * 1e-9
        last_sec = self._last_warn_sec.get(key, -math.inf)
        if now_sec - last_sec >= period_sec:
            self.get_logger().warning(message)
            self._last_warn_sec[key] = now_sec

    def _expire_pending_commands(self):
        now = time.monotonic()
        if self._pending_mode is not None and now >= self._pending_mode[1]:
            self._pending_mode = None
            self.get_logger().warning("Pending mode request expired; press the mode button again")
        if self._disarm_warn_at is not None and now >= self._disarm_warn_at:
            self._disarm_warn_at = None
            self.get_logger().warning(
                "Disarm still pending; waiting for the earlier arming response"
            )

    def _call_set_mode(self, mode):
        self._expire_pending_commands()
        if self._mode_future is not None:
            # Keep only the latest intent, not a backlog of mode changes.
            self._pending_mode = (
                None
                if mode == self._mode_in_flight
                else (mode, time.monotonic() + PENDING_COMMAND_TTL_SEC)
            )
            return

        if not self.mode_client.service_is_ready():
            self._warn_throttled(
                "set_mode", "Waiting for MAVROS set_mode service; press the mode button again"
            )
            return

        request = SetMode.Request()
        request.custom_mode = mode
        try:
            future = self.mode_client.call_async(request)
        except Exception as exc:
            self.get_logger().warning(f"Could not send mode request {mode}: {exc}")
            return
        self._mode_in_flight = mode
        self._mode_future = future
        future.add_done_callback(lambda completed: self._on_mode_response(completed, mode))

    def _on_mode_response(self, future, mode):
        if future is not self._mode_future:
            return
        self._mode_future = None
        self._mode_in_flight = None
        try:
            response = future.result()
            if response.mode_sent:
                self.get_logger().info(f"Requested ArduSub mode: {mode}")
            else:
                self.get_logger().warning(f"ArduSub rejected mode request: {mode}")
        except Exception as exc:
            self.get_logger().warning(f"Set mode request {mode} failed: {exc}")

        self._expire_pending_commands()
        pending = self._pending_mode
        self._pending_mode = None
        if pending is not None:
            self._call_set_mode(pending[0])

    def _call_arm(self, arm):
        self._expire_pending_commands()
        if self._arm_future is not None:
            if arm:
                # Never arm later as a side effect of an older request completing.
                self._warn_throttled(
                    "arm_busy",
                    "Arming request pending; arm was not queued. Press again when ready",
                )
            elif self._arm_in_flight and not self._pending_disarm:
                # Do not expire disarm: the older arm request may still complete late.
                self._pending_disarm = True
                self._disarm_warn_at = time.monotonic() + PENDING_COMMAND_TTL_SEC
            return

        if not self.arm_client.service_is_ready():
            self._warn_throttled(
                "arming", "Waiting for MAVROS arming service; press the arm/disarm button again"
            )
            return

        request = CommandBool.Request()
        request.value = arm
        try:
            future = self.arm_client.call_async(request)
        except Exception as exc:
            self.get_logger().warning(f"Could not send arm={arm} request: {exc}")
            return
        self._arm_in_flight = arm
        self._arm_future = future
        future.add_done_callback(lambda completed: self._on_arm_response(completed, arm))

    def _on_arm_response(self, future, arm):
        if future is not self._arm_future:
            return
        self._arm_future = None
        self._arm_in_flight = None
        try:
            response = future.result()
            if not response.success:
                self.get_logger().warning(f"ArduSub rejected arm={arm} request")
        except Exception as exc:
            self.get_logger().warning(f"Arm request arm={arm} failed: {exc}")

        self._expire_pending_commands()
        disarm_pending = self._pending_disarm
        self._pending_disarm = False
        self._disarm_warn_at = None
        if disarm_pending:
            self._call_arm(False)

    def _handle_button_edges(self, previous, current):
        def down(buttons, index):
            return index < len(buttons) and buttons[index] == 1

        def pressed(index):
            return down(current, index) and not down(previous, index)

        # Disarm takes precedence if a message contains both arm buttons.
        if pressed(BTN_ARM_OFF):
            self._call_arm(False)
        elif pressed(BTN_ARM_ON) and not down(current, BTN_ARM_OFF):
            self._call_arm(True)

        if pressed(BTN_Z_HOLD_ON):
            self._call_set_mode(DEPTH_HOLD_MODE)
        if pressed(BTN_Z_HOLD_OFF):
            self._call_set_mode(STABILIZE_MODE)

        if pressed(BTN_SCALE_UP) and self.throttle_index < len(THROTTLE_LEVELS) - 1:
            self.throttle_index += 1
            self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]
            self.get_logger().info(f"manual XY/yaw scale = {self.throttle_scale:.2f}")

        if pressed(BTN_SCALE_DN) and self.throttle_index > 0:
            self.throttle_index -= 1
            self.throttle_scale = THROTTLE_LEVELS[self.throttle_index]
            self.get_logger().info(f"manual XY/yaw scale = {self.throttle_scale:.2f}")

    def tick(self):
        self._expire_pending_commands()
        active_input = self._select_active_input()
        if active_input is None:
            self.last_axes = []
            self._publish_manual(0.0, 0.0, THROTTLE_NEUTRAL, 0.0)
            return

        self.last_axes = active_input["axes"]

        forward = -dz(self._get_axis(AXIS_FWD), DEADZONE) * MAX_MANUAL
        forward *= self.throttle_scale

        sway = -dz(self._get_axis(AXIS_SWAY), DEADZONE) * MAX_MANUAL
        sway *= self.throttle_scale

        heave = THROTTLE_NEUTRAL
        heave += -dz(self._get_axis(AXIS_HEAVE), DEADZONE_HEAVE) * THROTTLE_RANGE

        yaw = -dz(self._get_axis(AXIS_YAW), DEADZONE) * MAX_MANUAL
        yaw *= self.throttle_scale

        self._publish_manual(
            clamp(forward, -MAX_MANUAL, MAX_MANUAL),
            clamp(sway, -MAX_MANUAL, MAX_MANUAL),
            clamp(heave, 0.0, MAX_MANUAL),
            clamp(yaw, -MAX_MANUAL, MAX_MANUAL),
        )


def main():
    rclpy.init()
    node = ArduSubManualControl()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
