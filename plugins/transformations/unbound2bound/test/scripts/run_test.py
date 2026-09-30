#  (c) 2026 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
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
import os
import signal
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from rtigateway_test import TestCase, TestProcess

# Name of the test case
test = "tsfm_unbounded2bounded"

def extra_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--reject", choices=["extra", "mismatch"],
        help="Verify route creation rejects an invalid schema."
    )

def stop_process_tree(process: subprocess.Popen, force: bool = False) -> None:
    if TestProcess.is_windows():
        subprocess.run(
            ["taskkill", "/F", "/T", "/PID", str(process.pid)],
            check=False, timeout=5
        )
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL if force else signal.SIGTERM)
        except ProcessLookupError:
            pass

# Parse command-line arguments
args = TestCase.parse_args(test, domain_id=83, extra_args=extra_args)

# Rename test based on arguments
os.environ["U2B_DOMAIN"] = str(args.domain_id)
os.environ["NDDS_QOS_PROFILES"] = str(args.test_dir / f"{test}_test_qos.xml")

# Plugin's build directory
plugin_dir = args.test_dir.parent

TestProcess.extend_path(args.config, [
    plugin_dir,
    args.test_dir,
])

# Routing Service process
routing_service = TestCase.routingservice(args.test_dir / f"{test}.xml")

# Plugin tester process
tester = TestCase.tester(f"{test}_integration", 1, args.domain_id)

# Run the test case
if args.reject:
    tree = ET.parse(args.test_dir / f"{test}.xml")
    if args.reject == "extra":
        structure = tree.find("./types/module/struct[@name='unbounddata']")
        ET.SubElement(structure, "member", name="extra", type="long")
        expected = "expected exactly one member"
    else:
        member = tree.find("./types/module/struct[@name='BoundData']/member[@name='data']")
        member.set("type", "double")
        expected = "numeric element kind must match input"
    with tempfile.TemporaryDirectory(prefix="unbounded2bounded-") as directory:
        config = os.path.join(directory, "reject.xml")
        tree.write(config)
        with subprocess.Popen(
            ["rtiroutingservice", "-cfgFile", config, "-cfgName", "TestService"],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            start_new_session=not TestProcess.is_windows()
        ) as process:
            try:
                output, _ = process.communicate(timeout=min(args.timeout, 5))
            except subprocess.TimeoutExpired:
                stop_process_tree(process)
                try:
                    output, _ = process.communicate(timeout=5)
                except subprocess.TimeoutExpired:
                    stop_process_tree(process, force=True)
                    output, _ = process.communicate(timeout=5)
            finally:
                stop_process_tree(process, force=True)
        print(output)
        if "Unbounded2Bounded schema:" not in output or expected not in output:
            raise RuntimeError("Expected schema rejection was not logged")
else:
    TestCase.run(test, testers=[tester], support=[routing_service], timeout=args.timeout)
