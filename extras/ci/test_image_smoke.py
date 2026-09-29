"""Regression tests for failures hidden by launch or Gazebo wrapper exit codes."""

import unittest

from image_smoke import fatal_log_lines


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


if __name__ == "__main__":
    unittest.main()
