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
test = "adapt_modbus_inttest1"

# Parse command-line arguments
args = TestCase.parse_args(test)

# Plugin's build directory
plugin_dir = args.test_dir.parent.parent

# Base Gateway build directory
build_dir = plugin_dir.parent.parent.parent

TestProcess.extend_path(args.config, [
    plugin_dir,
    plugin_dir / "utilities",
    args.test_dir,
    # Add build directories of dependencies on Windows
    *([
        build_dir / "third-party" / "libmodbus" / args.config,
    ] if TestProcess.is_windows() else [])
])

# Modbus Server process
modbus_server = TestProcess(["modbusserver"], ignore_rc=True)

# Routing Service process
routing_service = TestCase.routingservice("RSConfigTest.xml", "RSModbusAdapterTest")

# Plugin tester process
tester = TestProcess(["modbus_integration_test1"])

# Run the test case
TestCase.run(test,
    testers=[tester],
    support=[modbus_server, routing_service],
    timeout=args.timeout
)
