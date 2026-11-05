"""Regression coverage: signal lifecycle, real ROS context, unchanged data path."""

import importlib.util
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import Mock, patch

import rclpy
import rclpy.executors
from rclpy.executors import ExternalShutdownException

ROOT = Path(__file__).resolve().parents[1]


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, ROOT / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


navigation = load("fixed_navigation", "external_navigation.py")


class ShutdownTests(unittest.TestCase):
    def setUp(self):
        self.handlers = {s: signal.getsignal(s) for s in (signal.SIGINT, signal.SIGTERM)}

    def tearDown(self):
        for signum, handler in self.handlers.items():
            self.assertEqual(signal.getsignal(signum), handler)

    def lifecycle(self, stage, signum, failure=False):
        events = []
        context = Mock()
        context.ok.return_value = True
        node = Mock()
        executor = Mock()

        def step(name, result=None):
            def act(*args, **kwargs):
                events.append(name)
                if name == stage:
                    if failure:
                        raise RuntimeError("genuine failure")
                    for _ in range(5):
                        signal.raise_signal(signum)
                if name == "spin":
                    # Enter cleanup even when testing a signal during cleanup.
                    signal.raise_signal(signum)
                return result

            return act

        context.try_shutdown.side_effect = step("context_cleanup")
        node.destroy_node.side_effect = step("node_cleanup")
        executor.shutdown.side_effect = step("executor_cleanup")
        executor.spin_once.side_effect = step("spin")
        with patch.object(navigation, "Context", return_value=context), patch.object(
            navigation.rclpy, "init", side_effect=step("init")
        ) as init, patch.object(
            navigation, "Navigation", side_effect=step("node", node)
        ), patch.object(
            navigation, "SingleThreadedExecutor", side_effect=step("executor", executor)
        ):
            if failure:
                with self.assertRaisesRegex(RuntimeError, "genuine failure"):
                    navigation.main()
            else:
                navigation.main()
            init.assert_called_once_with(
                args=None,
                context=context,
                signal_handler_options=navigation.SignalHandlerOptions.NO,
            )
        return events

    def test_signals_all_lifecycle_stages(self):
        for stage in (
            "init",
            "node",
            "executor",
            "spin",
            "executor_cleanup",
            "node_cleanup",
            "context_cleanup",
        ):
            for signum in (signal.SIGINT, signal.SIGTERM):
                with self.subTest(stage=stage, signum=signum):
                    events = self.lifecycle(stage, signum)
                    self.assertEqual(events[-1], "context_cleanup")
                    if stage == "init":
                        self.assertNotIn("node", events)
                    elif stage == "node":
                        self.assertNotIn("executor", events)
                        self.assertEqual(events[-2:], ["node_cleanup", "context_cleanup"])
                    else:
                        self.assertEqual(
                            events[-3:], ["executor_cleanup", "node_cleanup", "context_cleanup"]
                        )

    def test_genuine_failures_propagate_and_cleanup_continues(self):
        for stage in ("init", "node", "executor", "spin", "executor_cleanup", "node_cleanup"):
            with self.subTest(stage=stage):
                events = self.lifecycle(stage, signal.SIGINT, failure=True)
                self.assertEqual(events[-1], "context_cleanup")

    def test_real_context_can_be_run_twice_without_default_context(self):
        default_context = rclpy.get_default_context()
        for _ in range(2):
            contexts = []
            original_init = navigation.rclpy.init

            def capture(*args, **kwargs):
                contexts.append(kwargs["context"])
                return original_init(*args, **kwargs)

            def stop(*args, **kwargs):
                signal.raise_signal(signal.SIGINT)

            with patch.object(navigation.rclpy, "init", side_effect=capture), patch.object(
                navigation.SingleThreadedExecutor, "spin_once", side_effect=stop
            ):
                navigation.main()
            self.assertEqual(len(contexts), 1)
            self.assertIsNot(contexts[0], default_context)
            self.assertFalse(contexts[0].ok())
        self.assertIs(rclpy.get_default_context(), default_context)
        self.assertFalse(default_context.ok())

    def test_external_shutdown_on_real_context(self):
        contexts = []
        original_init = navigation.rclpy.init

        def capture(*args, **kwargs):
            contexts.append(kwargs["context"])
            return original_init(*args, **kwargs)

        def stop(*args, **kwargs):
            contexts[0].try_shutdown()
            raise ExternalShutdownException()

        with patch.object(navigation.rclpy, "init", side_effect=capture), patch.object(
            navigation.SingleThreadedExecutor, "spin_once", side_effect=stop
        ):
            navigation.main()
        self.assertFalse(contexts[0].ok())

    def test_immediate_process_stop_and_repeated_signals(self):
        for signum in (signal.SIGINT, signal.SIGTERM):
            for repeat in range(3):
                with self.subTest(signal=signum, repeat=repeat), tempfile.TemporaryFile(
                    mode="w+"
                ) as log:
                    process = subprocess.Popen(
                        [sys.executable, str(ROOT / "external_navigation.py")],
                        stdout=log,
                        stderr=subprocess.STDOUT,
                        start_new_session=True,
                    )
                    try:
                        deadline = time.monotonic() + 15
                        while time.monotonic() < deadline:
                            log.seek(0)
                            if "SITL navigation uses simulated" in log.read():
                                break
                            self.assertIsNone(process.poll(), "node exited before readiness")
                            time.sleep(0.001)
                        else:
                            self.fail("node readiness timeout")
                        for _ in range(5):
                            try:
                                os.kill(process.pid, signum)
                            except ProcessLookupError:
                                break
                            time.sleep(0.002)
                        self.assertEqual(process.wait(timeout=5), 0)
                        log.seek(0)
                        text = log.read()
                        self.assertNotIn("Traceback", text)
                        self.assertNotIn("Exception ignored", text)
                    finally:
                        if process.poll() is None:
                            process.kill()
                            process.wait()

    def test_cli_handles_late_signals_after_main_returns(self):
        code = """
import os, runpy, signal, sys
from unittest.mock import patch
from rclpy.executors import SingleThreadedExecutor
def stop(*args, **kwargs):
    os.kill(os.getpid(), signal.SIGINT)
with patch.object(SingleThreadedExecutor, 'spin_once', stop):
    runpy.run_path(sys.argv[1], run_name='__main__')
for signum in (signal.SIGINT, signal.SIGTERM):
    for _ in range(20):
        os.kill(os.getpid(), signum)
print('late signals handled')
"""
        result = subprocess.run(
            [sys.executable, "-c", code, str(ROOT / "external_navigation.py")],
            capture_output=True,
            text=True,
            timeout=15,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("late signals handled", result.stdout)
        self.assertNotIn("Traceback", result.stderr)

    def test_signal_inside_real_executor_constructor(self):
        constructor = rclpy.executors.SignalHandlerGuardCondition
        for signum in (signal.SIGINT, signal.SIGTERM):
            with self.subTest(signal=signum):
                unraisable = []
                completed = []

                def interrupt(*args, **kwargs):
                    os.kill(os.getpid(), signum)
                    result = constructor(*args, **kwargs)
                    completed.append(True)
                    return result

                with patch.object(
                    rclpy.executors, "SignalHandlerGuardCondition", interrupt
                ), patch.object(sys, "unraisablehook", unraisable.append):
                    navigation.main()
                self.assertEqual(completed, [True])
                self.assertEqual(unraisable, [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
