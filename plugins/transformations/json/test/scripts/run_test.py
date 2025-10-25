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
from rtigateway_test import TestCase, TestProcess

# Name of the test case
test = "tsfm_json"

# Parse command-line arguments
args = TestCase.parse_args(test)

# Plugin's build directory
plugin_dir = args.test_dir.parent

TestProcess.extend_path(args.config, [
    plugin_dir,
    args.test_dir,
])

# DDS message repeater process
msg_repeater = TestProcess(
    ["tsfm_json_repeater", "--id", 2, "--domain", args.support_domain_id],
    ignore_rc=True)

# Routing Service process
routing_service = TestCase.routingservice("tsfm_json_test.xml")

# Plugin tester process
tester = TestCase.tester("tsfm_json_tester", 1, args.domain_id)

# Run the test case
TestCase.run(test,
    testers=[tester],
    support=[msg_repeater, routing_service],
    timeout=args.timeout,
    qos_profile="tsfm_json_test_qos.xml"
)
