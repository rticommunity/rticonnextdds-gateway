#!/usr/bin/env python3
"""Exercise real DDS -> Routing Service -> Kafka routes in both directions."""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
import time
import uuid
from xml.sax.saxutils import escape


HERE = Path(__file__).resolve().parent


def compose(args, *command, timeout=60):
    return subprocess.run(
        ["docker", "compose", "-p", args.compose_project, "-f", str(HERE / "compose.yaml"), *command],
        check=True, capture_output=True, text=True, timeout=timeout,
    )


def broker_topic(args, topic, delete=False):
    return compose(args, "exec", "-T", "broker", "/opt/kafka/bin/kafka-topics.sh",
                   "--bootstrap-server", "localhost:19092",
                   "--delete" if delete else "--create", "--topic", topic,
                   *([] if delete else ["--partitions", "1", "--replication-factor", "1"]))


def configuration(plugin, json_plugin, bootstrap, topic, domain, binary, outbound, modified=True):
    transform = f"""<transformation plugin_name="plugins::json">
        <input_type_name>{'ShapeType' if outbound else 'RTI::Kafka::Message'}</input_type_name>
        <property><value>
          <element><name>transform_type</name><value>{'serialize' if outbound else 'deserialize'}</value></element>
          <element><name>buffer_member</name><value>payload.data</value></element>
        </value></property></transformation>""" if not binary else ""
    kafka_properties = f"""<property><value>
        <element><name>topic</name><value>{topic}</value></element>
        {'<element><name>payload_format</name><value>binary</value></element>' if modified and binary and outbound else ''}
        {'<element><name>group.id</name><value>' + topic + '</value></element><element><name>auto.offset.reset</name><value>earliest</value></element>' if not outbound else ''}
      </value></property>"""
    type_name = "RTI::Kafka::Message" if binary else "ShapeType"
    endpoints = f"""<dds_input name="{topic}" participant="dds"><registered_type_name>{type_name}</registered_type_name></dds_input>
      <output connection="kafka"><registered_type_name>RTI::Kafka::Message</registered_type_name>{kafka_properties}{transform}</output>""" if outbound else f"""
      <input connection="kafka"><registered_type_name>RTI::Kafka::Message</registered_type_name>{kafka_properties}</input>
      <dds_output name="{topic}" participant="dds"><registered_type_name>{type_name}</registered_type_name>{transform}</dds_output>"""
    return f"""<?xml version="1.0"?>
<dds>
  <plugin_library name="plugins">
    <adapter_plugin name="kafka"><dll>{escape(str(plugin))}</dll><create_function>RTI_RS_Kafka_AdapterPlugin_create</create_function></adapter_plugin>
    <transformation_plugin name="json"><dll>{escape(str(json_plugin))}</dll><create_function>RTI_TSFM_JsonTransformationPlugin_create</create_function></transformation_plugin>
  </plugin_library>
  <types>
    <struct name="ShapeType" extensibility="extensible">
      <member name="color" stringMaxLength="128" type="string" key="true"/>
      <member name="x" type="long"/><member name="y" type="long"/><member name="shapesize" type="long"/>
    </struct>
    <module name="RTI"><module name="Kafka">
      <struct name="MessagePayload" nested="true"><member name="data" sequenceMaxLength="-1" type="byte"/></struct>
      <struct name="Message"><member name="payload" type="nonBasic" nonBasicTypeName="RTI::Kafka::MessagePayload"/></struct>
    </module></module>
  </types>
  <routing_service name="kafka_integration"><domain_route name="domain">
    <participant name="dds"><domain_id>{domain}</domain_id>
      <participant_qos>
        <transport_builtin><mask>UDPv4</mask></transport_builtin>
        <discovery><initial_peers><element>127.0.0.1</element></initial_peers><multicast_receive_addresses/></discovery>
        <property><value><element><name>dds.transport.UDPv4.builtin.parent.allow_interfaces</name><value>127.0.0.1</value></element></value></property>
      </participant_qos>
      <registered_type name="ShapeType" type_name="ShapeType"/>
      <registered_type name="RTI::Kafka::Message" type_name="RTI::Kafka::Message"/>
    </participant>
    <connection name="kafka" plugin_name="plugins::kafka"><property><value>
      <element><name>bootstrap.servers</name><value>{escape(bootstrap)}</value></element>
    </value></property>
      <registered_type name="ShapeType" type_name="ShapeType"/>
      <registered_type name="RTI::Kafka::Message" type_name="RTI::Kafka::Message"/>
    </connection>
    <session name="session"><route name="route">{endpoints}</route></session>
  </domain_route></routing_service>
</dds>"""


def stop(process, timeout=12):
    if process is None or process.poll() is not None:
        return True
    process.send_signal(signal.SIGINT)
    try:
        process.wait(timeout=timeout)
        return True
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
        return False


def records(path):
    text = path.read_text()
    decoder = json.JSONDecoder()
    result = []
    position = 0
    while True:
        start = text.find("{", position)
        if start < 0:
            break
        try:
            value, position = decoder.raw_decode(text, start)
        except json.JSONDecodeError:
            position = start + 1  # DDS diagnostics are retained in the log.
            continue
        if isinstance(value, dict) and ("hex" in value or "color" in value):
            result.append(value)
    return result


def run_case(args, label, plugin, kind, run_id):
    binary, outbound = kind.startswith("bytes"), kind.endswith("out")
    directory = args.artifacts / label / kind
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "USER_QOS_PROFILES.xml").write_text("""<dds>
      <qos_library name="IntegrationIsolation">
        <qos_profile name="Factory" is_default_participant_factory_profile="true">
          <participant_factory_qos><monitoring><enable>false</enable></monitoring></participant_factory_qos>
        </qos_profile>
      </qos_library></dds>""")
    topic = f"gw_it_{run_id}_{label}_{kind}"
    config = directory / "routing.xml"
    config.write_text(configuration(plugin, args.json_plugin, args.bootstrap, topic,
                                    args.domain, binary, outbound, modified=(label == "modified")))
    subscriber = router = publisher = None
    logs = []
    report = {"adapter": label, "scenario": kind, "expected_count": args.count, "passed": False}
    created = False
    try:
        broker_topic(args, topic)
        created = True
        def launch(command, name):
            output = open(directory / f"{name}.stdout", "w")
            error = open(directory / f"{name}.stderr", "w")
            logs.extend([output, error])
            environment = args.router_environment if name == "router" else args.environment
            return subprocess.Popen(command, stdout=output, stderr=error, env=environment, cwd=directory)

        transport = "bytes" if binary else "shape"
        sub_mode = f"{'kafka' if outbound else 'dds'}-sub-{transport}"
        pub_mode = f"{'dds' if outbound else 'kafka'}-pub-{transport}"
        def client(mode):
            return [str(args.client), mode, args.bootstrap, str(args.domain), topic, str(args.count)]

        subscriber = launch(client(sub_mode), "subscriber")
        router = launch([str(args.routing_service), "-cfgFile", str(config),
                         "-cfgName", "kafka_integration", "-verbosity", "LOCAL:WARN"], "router")
        if not outbound:
            deadline = time.monotonic() + 20
            while time.monotonic() < deadline:
                if '"ready":true' in (directory / "subscriber.stdout").read_text():
                    break
                if router.poll() is not None or subscriber.poll() is not None:
                    raise RuntimeError("DDS subscriber or Routing Service exited during discovery")
                time.sleep(0.1)
            else:
                raise RuntimeError("DDS matching timed out")
        publisher = launch(client(pub_mode), "publisher")
        deadline = time.monotonic() + 40
        while publisher.poll() is None:
            if router.poll() is not None:
                raise RuntimeError("Routing Service exited before publishing completed")
            if time.monotonic() > deadline:
                raise RuntimeError("Publisher timed out")
            time.sleep(0.1)
        if publisher.returncode != 0:
            raise RuntimeError("Publisher failed; see publisher.stderr and publisher.stdout")
        subscriber.wait(timeout=40)
        report["publisher_exit"] = publisher.returncode
        report["subscriber_exit"] = subscriber.returncode
        received = records(directory / "subscriber.stdout")
        report["received_count"] = len(received)
        values = []
        raw_values = [bytes.fromhex(item["hex"]) for item in received] if outbound else []
        report["trailing_nul_count"] = sum(raw.endswith(b"\0") for raw in raw_values) if not binary else 0
        strict_json = True
        for item in received:
            if binary:
                values.append(item["hex"])
            elif outbound:
                raw = bytes.fromhex(item["hex"])
                try:
                    values.append(json.loads(raw))
                except json.JSONDecodeError:
                    strict_json = False
                    # Diagnose semantic preservation without allowing the test
                    # to pass when strict consumers reject the wire payload.
                    values.append(json.loads(raw.rstrip(b"\0")))
            else:
                values.append(item)
        expected = [((b"" if i == 0 else bytes([10, 4, i, 0, 255, 0 if i % 2 == 0 else 128])).hex())
                    if binary else {"color": "BLUE", "x": i, "y": i * 2, "shapesize": 30}
                    for i in range(args.count)]
        report["payloads_match"] = values == expected
        if not binary and outbound:
            report["strict_json"] = strict_json
        report["passed"] = (publisher.returncode == 0 and subscriber.returncode == 0
                            and values == expected and strict_json)
        if not report["passed"]:
            report["error"] = ("JSON contains a trailing NUL rejected by strict consumers" if not strict_json
                               else "Process failure, missing/duplicate samples, or payload mismatch")
    except (RuntimeError, ValueError, KeyError, subprocess.SubprocessError) as error:
        report["error"] = str(error)
    finally:
        if publisher is not None:
            stop(publisher, 2)
        if subscriber is not None:
            stop(subscriber, 2)
        report["shutdown_completed"] = stop(router)
        if not report["shutdown_completed"]:
            report["passed"] = False
            report["error"] = "Routing Service shutdown timed out"
        if router is not None:
            report["router_exit"] = router.returncode
            if router.returncode != 0:
                report["passed"] = False
                report.setdefault("error", "Routing Service exited unsuccessfully")
        for log in logs:
            log.close()
        if created:
            try:
                broker_topic(args, topic, delete=True)
            except subprocess.SubprocessError as error:
                report["topic_cleanup_error"] = str(error)
                report["passed"] = False
        (directory / "result.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report), flush=True)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--client", type=Path, required=True)
    parser.add_argument("--modified", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--json-plugin", type=Path, required=True)
    parser.add_argument("--routing-service", type=Path, required=True)
    parser.add_argument("--sdk-dir", type=Path)
    parser.add_argument("--architecture")
    parser.add_argument("--bootstrap", default="127.0.0.1:19092")
    parser.add_argument("--domain", type=int, default=173)
    parser.add_argument("--count", type=int, default=20)
    parser.add_argument("--artifacts", type=Path, required=True)
    parser.add_argument("--compose-project", default="piddock-kafka-integration")
    parser.add_argument("--start-broker", action="store_true")
    parser.add_argument("--scenario", action="append", choices=("json_out", "json_in", "bytes_out", "bytes_in"))
    parser.add_argument("--variant", choices=("both", "modified", "baseline"), default="both")
    args = parser.parse_args()
    args.environment = dict(os.environ)
    # The native Routing Service executable needs the environment normally set
    # by rtisetenv; configure it explicitly for subprocesses and retain overrides.
    sdk = args.sdk_dir or args.routing_service.resolve().parents[4]
    architecture = args.architecture or args.routing_service.parent.name
    args.environment.setdefault("NDDSHOME", str(sdk))
    if (sdk / "rti_license.dat").exists():
        args.environment.setdefault("RTI_LICENSE_FILE", str(sdk / "rti_license.dat"))
    sdk_libraries = str(sdk / "lib" / architecture)
    for variable in ("DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"):
        args.environment[variable] = sdk_libraries + os.pathsep + args.environment.get(variable, "")
    args.router_environment = dict(args.environment)
    application_libraries = str(sdk / "resource" / "app" / "lib" / architecture)
    for variable in ("DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"):
        args.router_environment[variable] = application_libraries + os.pathsep + args.environment[variable]
    if not 1 <= args.count <= 255:
        parser.error("--count must be between 1 and 255")
    if args.variant == "baseline" and not args.baseline:
        parser.error("--variant baseline requires --baseline")
    args.artifacts = args.artifacts.resolve()
    args.artifacts.mkdir(parents=True, exist_ok=True)
    reports = []
    run_id = uuid.uuid4().hex[:10]
    try:
        if args.start_broker:
            compose(args, "up", "-d", "--wait", timeout=150)
        variants = [("baseline", args.baseline)] if args.baseline and args.variant != "modified" else []
        if args.variant != "baseline":
            variants.append(("modified", args.modified))
        for label, plugin in variants:
            for kind in args.scenario or ("json_out", "json_in", "bytes_out", "bytes_in"):
                reports.append(run_case(args, label, plugin, kind, run_id))
    finally:
        if args.start_broker:
            compose(args, "down", timeout=45)
    (args.artifacts / "summary.json").write_text(json.dumps(reports, indent=2) + "\n")
    # Baseline defects are negative controls; modified cases must all pass.
    checked = [r for r in reports if r["adapter"] == "modified" or args.variant == "baseline"]
    return 0 if checked and all(r["passed"] for r in checked) else 1


if __name__ == "__main__":
    raise SystemExit(main())
