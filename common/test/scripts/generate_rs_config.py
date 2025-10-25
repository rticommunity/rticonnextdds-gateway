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
import shutil
import re
from pathlib import Path

parser = argparse.ArgumentParser(description="Generate a Routing Service configuration file for testing.")
parser.add_argument(
    "-C", "--config",
    help="Build configuration to use.",
    default="Release"
)
parser.add_argument(
    "input_file",
    type=Path,
    help="Path to the input RS configuration file."
)
parser.add_argument(
    "output_file",
    type=Path,
    help="Path to the output RS configuration file."
)
args = parser.parse_args()

# If non-Debug configurations, we can just copy the input to output
args.output_file.parent.mkdir(parents=True, exist_ok=True)
if "Debug" not in args.config:
    shutil.copyfile(args.input_file, args.output_file)
else:
    # For Debug configurations, we must add the "d" suffix to all library names.
    # We must scan the input file for "<dll>foo</dll>" entries and modify them to "<dll>food</dll>".
    with open(args.input_file, "r") as infile:
        input_content = infile.read()
    dll_re = re.compile(r"<dll>(.+?)</dll>")
    output_content = re.sub(dll_re, lambda m: f"<dll>{m.group(1)}d</dll>", input_content)
    with open(args.output_file, "w") as outfile:
        outfile.write(output_content)
