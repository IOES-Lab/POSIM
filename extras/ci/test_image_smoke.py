"""Regression tests for failures hidden by launch or Gazebo wrapper exit codes."""

import unittest
import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import Mock, patch

from image_smoke import fatal_log_lines, linked_library, wait_for_entity


class LibraryLinkageTests(unittest.TestCase):
    def test_resolved_path(self):
        self.assertEqual(
            linked_library(
                "  libgz-transport.so.15 => /opt/patched/lib.so (0x123)\n", "libgz-transport.so.15"
            ),
            Path("/opt/patched/lib.so").resolve(),
        )

    def test_missing_library(self):
        with self.assertRaises(RuntimeError):
            linked_library("libgz-transport.so.15 => not found\n", "libgz-transport.so.15")

    def test_wrong_soname(self):
        with self.assertRaises(RuntimeError):
            linked_library(
                "libgz-transport.so.150 => /opt/wrong.so (0x123)\n", "libgz-transport.so.15"
            )

    def test_ambiguous_library(self):
        with self.assertRaises(RuntimeError):
            linked_library(
                "libgz-transport.so.15 => /opt/a.so (0x123)\n" * 2, "libgz-transport.so.15"
            )


class FatalLogTests(unittest.TestCase):
    def test_normal_sigint_is_not_a_crash(self):
        log = (
            "[INFO] signal_handler(signum=2)\n"
            "[ERROR] [gazebo-1]: process has died [exit code -2, cmd='gz']\n"
        )
        self.assertEqual(fatal_log_lines(log), [])

    def test_gazebo_abort_is_not_hidden_by_wrapper_sigint(self):
        log = "[gazebo-1] Aborted\n[ERROR] [gazebo-1]: exit code -2\n"
        self.assertEqual(fatal_log_lines(log), ["[gazebo-1] Aborted"])

    def test_uncaught_ros_context_error(self):
        log = (
            "[gazebo-1] terminate called after throwing an instance of "
            "'rclcpp::exceptions::RCLError'\n"
        )
        self.assertTrue(fatal_log_lines(log))

    def test_signal_and_shell_exit_conventions(self):
        for code in (-11, -6, 134, 139):
            with self.subTest(code=code):
                self.assertTrue(fatal_log_lines(f"process has died [exit code {code}, cmd='gz']"))

    def test_loading_and_python_failures(self):
        for message in (
            "Failed to load system plugin",
            "error while loading shared libraries",
            "Traceback (most recent call last):",
            "Segmentation fault",
        ):
            with self.subTest(message=message):
                self.assertTrue(fatal_log_lines(message))

    def test_other_abnormal_child_exits_are_failures(self):
        for code in (1, 2, 127, 137, 143, -9):
            with self.subTest(code=code):
                self.assertTrue(fatal_log_lines(f"process has died [exit code {code}, cmd='gz']"))


class EntityReadinessTests(unittest.TestCase):
    def test_waits_for_model_not_just_world_control(self):
        proc = Mock()
        proc.poll.return_value = None
        with TemporaryDirectory() as directory:
            out = Path(directory)
            with (
                patch("image_smoke.time.monotonic", return_value=1),
                patch("image_smoke.time.sleep"),
                patch(
                    "image_smoke.capture",
                    side_effect=[(124, ""), (0, 'name: "ground"'), (0, 'name: "camera"')],
                ) as read,
            ):
                self.assertTrue(wait_for_entity(out, "world", "camera", proc, 90))
                self.assertEqual(read.call_count, 3)
            result = json.loads((out / "entity_readiness.json").read_text())
            self.assertEqual(len(result["attempts"]), 3)
            self.assertFalse(result["attempts"][0]["entity_present"])

    def test_absent_model_still_fails_at_original_deadline(self):
        proc = Mock()
        proc.poll.return_value = None
        with TemporaryDirectory() as directory:
            with (
                patch("image_smoke.time.monotonic", side_effect=[89, 90, 90]),
                patch("image_smoke.time.sleep"),
                patch("image_smoke.capture", return_value=(0, 'name: "other"')) as read,
            ):
                self.assertFalse(wait_for_entity(Path(directory), "world", "camera", proc, 90))
                self.assertEqual(read.call_count, 1)
                self.assertEqual(read.call_args.args[-1], 1)

    def test_terminated_launch_is_not_retried(self):
        proc = Mock()
        proc.poll.return_value = 1
        with TemporaryDirectory() as directory:
            with patch("image_smoke.capture") as read:
                self.assertFalse(wait_for_entity(Path(directory), "world", "camera", proc, 90))
                read.assert_not_called()


if __name__ == "__main__":
    unittest.main()
