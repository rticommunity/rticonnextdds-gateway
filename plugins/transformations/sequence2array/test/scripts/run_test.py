#  (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
#
#  RTI grants Licensee a license to use, modify, compile, and create
#  derivative works of the software solely for use with RTI Connext DDS.
#  Licensee may redistribute copies of the software provided that all such
#  copies are subject to this license.
#  The software is provided "as is", with no warranty of any type, including
#  any warranty for fitness for any purpose. RTI is under no obligation to
#  maintain or support the software.  RTI shall not be liable for any
#  incidental or consequential damages arising out of the use or inability to
#  use the software.
import argparse
from rtigateway_test import TestCase, TestProcess

# Name of the test case
test = "tsfm_seq2array"

def extra_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "-n", "--test-number",
        type=int,
        required=True,
        help="Integration test to run."
    )

# Parse command-line arguments
args = TestCase.parse_args(test, extra_args=extra_args)

# Rename test based on arguments
test = f"{test}_inttest{args.test_number}"

# Plugin's build directory
plugin_dir = args.test_dir.parent

TestProcess.extend_path(args.config, [
    plugin_dir,
    args.test_dir,
])

# Routing Service process
routing_service = TestCase.routingservice(f"{test}.xml")

# Plugin tester process
tester = TestCase.tester(test, 1, args.domain_id)

# Run the test case
TestCase.run(test,
    testers=[tester],
    support=[routing_service],
    timeout=args.timeout,
    qos_profile="tsfm_seq2array_test_qos.xml"
)
