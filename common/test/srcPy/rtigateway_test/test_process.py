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
import os
import subprocess
import platform
from pathlib import Path

from .test_log import TestLog as log

class TestProcess:
    def __init__(self, command: list, ignore_rc: bool=False) -> None:
        self._command = list(map(str, command))
        self._ignore_rc = ignore_rc
        self._process = None

    def __repr__(self) -> str:
        return " ".join(self._command)

    @classmethod
    def platform(cls) -> str:
        return platform.system()

    @classmethod
    def is_windows(cls) -> bool:
        return cls.platform() == "Windows"

    @classmethod
    def is_linux(cls) -> bool:
        return cls.platform() == "Linux"

    @classmethod
    def is_macos(cls) -> bool:
        return cls.platform() == "Darwin"

    @classmethod
    def extend_path(cls, config: str, paths: list) -> None:
        paths = [Path(p).resolve() for p in paths]

        if cls.is_linux():
            env_vars = ["PATH", "LD_LIBRARY_PATH"]
        elif cls.is_macos():
            env_vars = ["PATH", "DYLD_LIBRARY_PATH"]
        elif cls.is_windows():
            env_vars = ["PATH"]
            # MSBUILD uses separate directories for each configuration
            paths += [p / config for p in paths]
        else:
            raise RuntimeError(f"Unsupported test platform: {cls.platform()}")

        for env_var in env_vars:
            current_paths = [
                Path(value).resolve()
                for value in os.environ.get(env_var, "").split(os.pathsep)
            ]
            new_paths = [*paths, *(p for p in current_paths if p not in paths)]
            os.environ[env_var] = os.pathsep.join(map(str, new_paths))
            log.info("extended {}: {}", env_var, os.environ[env_var])

    def _rc(self, rc: int) -> int:
        log.info("process exited with code {}: {}", rc, self)
        if rc != 0 and self._ignore_rc:
            log.info("non-zero return code ({}) ignored: {}", rc, self)
            return 0
        return rc

    def start(self) -> subprocess.Popen:
        if self._process is not None:
            raise RuntimeError("Process already started.")
        log.info("start process: {}", self)
        if self.is_windows():
            # On windows, run as a shell process to get PATH resolution
            self._process = subprocess.Popen(
                " ".join(self._command),
                shell=True,
                creationflags=subprocess.CREATE_NEW_PROCESS_GROUP
            )
        else:
            # On POSIX set start_new_session to True to create a new process group
            self._process = subprocess.Popen(self._command, start_new_session=True)
        return self._process

    def wait(self, timeout: float = None, kill: bool=True) -> int:
        if self._process is None:
            raise RuntimeError("Process not started.")
        try:
            log.info("wait for process: {}", self)
            rc = self._process.wait(timeout)
            return self._rc(rc)
        except subprocess.TimeoutExpired:
            log.error("timeout ({}s) expired waiting for process: {}", timeout, self)
            if kill:
                return self.kill()
            raise

    def stop(self) -> None:
        if self._process is None or self._process.poll() is not None:
            return
        log.info("stop process: {}", self)
        if self.is_windows():
            # Always forcefully terminate on Windows, to make sure that we don't
            # leave orphaned processes and cause the test process to hang.
            subprocess.run(["taskkill", "/F", "/T", "/PID", str(self._process.pid)])
        else:
            os.killpg(os.getpgid(self._process.pid), subprocess.signal.SIGTERM)

    def kill(self) -> int:
        if self._process is None or self._process.poll() is not None:
            return
        log.warning("kill process: {}", self)
        if self.is_windows():
            subprocess.run(["taskkill", "/F", "/T", "/PID", str(self._process.pid)])
        else:
            os.killpg(os.getpgid(self._process.pid), subprocess.signal.SIGKILL)
        try:
            rc = self._process.wait(timeout=5.0)
            return self._rc(rc)
        except subprocess.TimeoutExpired:
            log.error("timeout expired waiting for killed process: {}", self)
            raise
