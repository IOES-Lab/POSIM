"""Websocket/ROS lifecycle regressions; no simulator or physical hardware."""

import asyncio
import importlib.util
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock

import pytest
from websockets.exceptions import ConnectionClosedError

spec = importlib.util.spec_from_file_location(
    "ws_lifecycle_under_test", Path(__file__).parents[1] / "scripts/ws_to_joy.py"
)
server = importlib.util.module_from_spec(spec)
spec.loader.exec_module(server)


class Messages:
    def __init__(self, messages, failure=None):
        self.messages = iter(messages)
        self.failure = failure

    def __aiter__(self):
        return self

    async def __anext__(self):
        try:
            return next(self.messages)
        except StopIteration:
            if self.failure is not None:
                raise self.failure
            raise StopAsyncIteration


@pytest.fixture
def node(monkeypatch):
    # Only supported logger methods are exposed: warn() must never be used.
    log = Mock(spec=["info", "warning", "error"])
    n = SimpleNamespace(get_logger=lambda: log, publish_payload=Mock())
    monkeypatch.setattr(server.rclpy, "ok", lambda: True)
    return n, log


def run(messages, node, stopped=False):
    async def body():
        stopping = asyncio.Event()
        if stopped:
            stopping.set()
        await server.relay_messages(messages, node, stopping)

    asyncio.run(body())


def test_valid_payload_is_unchanged(node):
    n, log = node
    run(Messages(['{"axes":[0,0.5],"buttons":[0,1]}']), n)
    n.publish_payload.assert_called_once_with({"axes": [0, 0.5], "buttons": [0, 1]})
    log.error.assert_not_called()


def test_bad_json_warns_and_next_input_survives(node):
    n, log = node
    run(Messages(["not-json", '{"axes":[0]}']), n)
    log.warning.assert_called_once()
    n.publish_payload.assert_called_once_with({"axes": [0]})


def test_stopped_server_drops_buffered_messages(node):
    n, _ = node
    run(Messages(["{}"] * 100), n, stopped=True)
    n.publish_payload.assert_not_called()


def test_invalid_context_drops_buffered_messages(node, monkeypatch):
    n, log = node
    monkeypatch.setattr(server.rclpy, "ok", lambda: False)
    run(Messages(["{}"] * 100), n)
    n.publish_payload.assert_not_called()
    log.error.assert_not_called()


def test_signal_callback_can_run_during_backlog(node):
    n, _ = node

    async def body():
        stopping = asyncio.Event()
        asyncio.get_running_loop().call_soon(stopping.set)
        await server.relay_messages(Messages(["{}"] * 10000), n, stopping)

    asyncio.run(body())
    n.publish_payload.assert_not_called()


def test_live_publish_error_is_not_silenced(node):
    n, log = node
    n.publish_payload.side_effect = [ValueError("bad axes"), None]
    run(Messages(["{}", '{"axes":[]}']), n)
    assert n.publish_payload.call_count == 2
    log.error.assert_called_once()


def test_context_shutdown_race_does_not_log_or_continue(node, monkeypatch):
    n, log = node
    ok = [True]
    monkeypatch.setattr(server.rclpy, "ok", lambda: ok[0])

    def publish(_):
        ok[0] = False
        raise RuntimeError("publisher context invalid")

    n.publish_payload.side_effect = publish
    run(Messages(["{}"] * 100), n)
    n.publish_payload.assert_called_once()
    log.error.assert_not_called()


def test_dropped_connection_is_expected(node):
    n, log = node
    run(Messages([], ConnectionClosedError(None, None)), n)
    log.error.assert_not_called()


def test_bind_failure_cleans_node_and_context(monkeypatch):
    events = []
    n = SimpleNamespace(
        ws_host="127.0.0.1", ws_port=8765, destroy_node=lambda: events.append("destroy")
    )
    monkeypatch.setattr(server.rclpy, "init", lambda **kw: events.append(kw))
    monkeypatch.setattr(server.rclpy, "try_shutdown", lambda: events.append("shutdown"))
    monkeypatch.setattr(server, "JoyWebSocketServer", lambda: n)

    def fail(*args, **kwargs):
        raise OSError("port busy")

    monkeypatch.setattr(server.websockets, "serve", fail)
    with pytest.raises(OSError, match="port busy"):
        asyncio.run(server.main_async())
    assert events[0]["signal_handler_options"] == server.SignalHandlerOptions.NO
    assert events[-2:] == ["destroy", "shutdown"]


def test_node_creation_failure_cleans_context(monkeypatch):
    shutdown = Mock()
    monkeypatch.setattr(server.rclpy, "init", Mock())
    monkeypatch.setattr(server.rclpy, "try_shutdown", shutdown)
    monkeypatch.setattr(server, "JoyWebSocketServer", Mock(side_effect=RuntimeError("node fail")))
    with pytest.raises(RuntimeError, match="node fail"):
        asyncio.run(server.main_async())
    shutdown.assert_called_once()
