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
from typing import Callable
import argparse
import os
from pathlib import Path

from .test_log import TestLog as log
from .test_process import TestProcess

class TestCase:
    @classmethod
    def parse_args(cls,
            test: str,
            timeout: float=60.0,
            domain_id: int=0,
            support_domain_id: int=1,
            extra_args: Callable[[argparse.ArgumentParser], None]=None) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description=f"Run RTI Gateway test case: {test}")
        parser.add_argument(
            "--test-dir",
            type=Path,
            default=Path.cwd(),
            help="Directory containing the test to run."
        )
        parser.add_argument(
            "-C", "--config",
            default="Release",
            help="Build configuration to test."
        )
        parser.add_argument(
            "-T", "--timeout",
            type=float,
            default=timeout,
            help="Timeout for the test execution in seconds."
        )
        parser.add_argument(
            "-D", "--domain-id",
            type=int,
            default=domain_id,
            help="DDS Domain ID to use for the test."
        )
        parser.add_argument(
            "-S", "--support-domain-id",
            type=int,
            default=support_domain_id,
            help="Additional DDS Domain ID to use for testing."
        )
        if extra_args is not None:
            extra_args(parser)
        return parser.parse_args()

    @classmethod
    def routingservice(cls,
            config_file: Path,
            config_name: str="TestService",
            extra_args: list = None,
            ) -> TestProcess:
        # On windows, process exists with code 1 even on success
        return TestProcess([
            "rtiroutingservice",
            "-cfgFile", config_file,
            "-cfgName", config_name,
            *(extra_args or [])
        ], ignore_rc=TestProcess.is_windows())

    @classmethod
    def tester(cls,
            tester_name: str,
            tester_id: int,
            domain_id: int=0,
            extra_args: list = None,
            ) -> TestProcess:
        return TestProcess([
            tester_name,
            "--id", tester_id,
            "--domain", domain_id,
            *(extra_args or [])
        ])

    @classmethod
    def run(cls,
            test: str,
            testers: list,
            support: list = None,
            timeout: float = 60.0,
            qos_profile: str = None
            ) -> None:
        log.set_prefix(f"[{test}] ")

        if qos_profile is not None:
            os.environ["NDDS_QOS_PROFILES"] = qos_profile

        failed = []

        support = support or []

        try:
            log.info("starting test case: {}", test)
            
            for service in support:
                try:
                    service.start()
                    log.info("support service started: {}", service)
                except Exception as e:
                    log.error("Failed to start support service {}: {}", service, e)
                    failed.append((service, e))
                    raise

            for tester in testers:
                try:
                    tester.start()
                    log.info("tester started: {}", tester)
                except Exception as e:
                    log.error("Failed to start tester {}: {}", tester, e)
                    failed.append((tester, e))
                    raise

            for tester in reversed(testers):
                try:
                    rc = tester.wait(timeout=timeout, kill=True)
                    if rc != 0:
                        raise RuntimeError(f"Tester exited with non-zero code: {rc}")
                    log.info("tester completed: {}", tester)
                except Exception as e:
                    log.error("Tester {} failed: {}", tester, e)
                    failed.append((tester, e))
                    raise

            for service in reversed(support):
                try:
                    service.stop()
                    rc = service.wait(timeout=timeout, kill=True)
                    if rc != 0:
                        raise RuntimeError(f"Support service exited with non-zero code: {rc}")
                    log.info("support service stopped: {}", service)
                except Exception as e:
                    failed.append((service, e))
                    log.error("Support service {} failed: {}", service, e)
                    raise

        except Exception:
            error_messages = []
            for item, error in failed:
                error_messages.append(f"Process {item} failed with error: {error}")
            raise RuntimeError("\n".join(error_messages)) from None

        finally:
            log.info("cleaning up test case: {}", test)
            for tester in testers:
                try:
                    tester.kill()
                    log.info("tester killed: {}", tester)
                except Exception as e:
                    log.error("Failed to kill tester {}: {}", tester, e)
            for service in support:
                try:
                    service.kill()
                    log.info("support service killed: {}", service)
                except Exception as e:
                    log.error("Failed to kill support service {}: {}", service, e)
            log.info("test case complete: {} ({})", test, "FAILED" if failed else "PASSED")
