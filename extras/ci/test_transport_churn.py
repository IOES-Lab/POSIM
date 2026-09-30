"""Test that diagnostic outcome classification cannot hide crashes or idle runs."""

import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location(
    "transport_pairs", Path(__file__).parent / "transport_shutdown" / "run_pairs.py"
)
PAIRS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PAIRS)


class TransportChurnTests(unittest.TestCase):
    publisher = "rounds=100 anchor_received=0 churn_received=0"
    subscriber = "rounds=10 anchor_received=100 churn_received=200"

    def test_passing_counts(self):
        self.assertTrue(PAIRS.trial_passed(0, 0, [], self.publisher, self.subscriber))

    def test_subscriber_crash(self):
        self.assertFalse(PAIRS.trial_passed(0, -11, [], self.publisher, self.subscriber))

    def test_publisher_crash(self):
        self.assertFalse(PAIRS.trial_passed(-11, 0, [], self.publisher, self.subscriber))

    def test_timeout_is_not_a_pass(self):
        self.assertFalse(PAIRS.trial_passed(0, 0, ["subscriber"], self.publisher, self.subscriber))

    def test_missing_output(self):
        self.assertFalse(PAIRS.trial_passed(0, 0, [], self.publisher, ""))

    def test_no_churn_delivery(self):
        self.assertFalse(
            PAIRS.trial_passed(
                0,
                0,
                [],
                self.publisher,
                self.subscriber.replace("churn_received=200", "churn_received=0"),
            )
        )


if __name__ == "__main__":
    unittest.main()
