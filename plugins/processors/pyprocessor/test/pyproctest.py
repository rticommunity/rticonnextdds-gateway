import os
import sys
import json
import pprint
import inspect
import time

sys.path.insert(0, os.path.abspath('..'))

from rti.routing import proc
from rti.routing import service

port_info_map = {
    'Input1': {
        'index': 0,
        'name': 'Input1',
        'stream_info': {
            'stream_name': 'Example ComplexType',
            'type_name': 'ComplexType'
        }
    },
    'Input2': {
        'index': 1,
        'name': 'Input2',
        'stream_info': {
            'stream_name': 'Other topic',
            'type_name': 'ComplexType'
        }
    },
    'Output1': {
        'index': 0,
        'name': 'Output1',
        'stream_info': {
            'stream_name': 'Example ComplexType',
            'type_name': 'ComplexType'
        }
    },
    'Output2': {
        'index': 1,
        'name': 'Output2',
        'stream_info': {
            'stream_name': 'Other topic',
            'type_name': 'ComplexType'
        }
    }
}

class TestProcessor(proc.Processor):
    def __init__(self, route, properties):
        self.event_counts = {
            'on_input_enabled' : 0,
            'on_output_enabled' : 0,
            'on_input_disabled': 0,
            'on_output_disabled' : 0,
            'on_start' : 0,
            'on_stop' : 0,
            'on_run' : 0,
            'on_pause' : 0,
            'on_data_available': 0,
            'on_periodic_action': 0
        }
        assert len(properties) == 2
        assert properties['rti.routing.proc.py.test.prop1'] == 'value1'
        assert properties['rti.routing.proc.py.test.prop2'] == 'value2'

    def __del__(self):
        for event in self.event_counts:
            print('TestProcessor::__del__: checking event=' + event)
            assert self.event_counts[event] > 0

    def on_input_enabled(self, route, input):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_input_enabled:' + str(input.info))
        assert port_info_map[input.info['name']] == input.info

    def on_output_enabled(self, route, output):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_output_enabled:' + str(output.info))
        assert port_info_map[output.info['name']] == output.info

    def on_start(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_start')
        check_ports(route, route.inputs)
        check_ports(route, route.outputs)

    def on_run(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        check_ports(route, route.inputs)
        check_ports(route, route.outputs)


    def on_data_available(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_data_available')
        samples = route.inputs[0].read()
        for sample in samples:
            pprint.PrettyPrinter(indent=4).pprint(sample.data)
            json.dumps(sample.data)

    def on_periodic_action(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_periodic_action')
        check_ports(route, route.inputs)
        check_ports(route, route.outputs)

    def on_pause(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_pause')
        check_ports(route, route.inputs)
        check_ports(route, route.outputs)

    def on_stop(self, route):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_stop')
        check_empty_port_iterator(route.inputs)
        check_empty_port_iterator(route.outputs)

    def on_input_disabled(self, route, input):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_input_disabled:' + str(input.info))
        assert port_info_map[input.info['name']] == input.info
        check_empty_port_iterator(route.inputs)
        check_empty_port_iterator(route.outputs)
        assert input == route[input.info['name']]

    def on_output_disabled(self, route, output):
        self.event_counts[inspect.currentframe().f_code.co_name] += 1
        print('TestProcessor::on_output_disabled:' + str(output.info))
        assert port_info_map[output.info['name']] == output.info
        check_empty_port_iterator(route.inputs)
        check_empty_port_iterator(route.outputs)
        assert output == route[output.info['name']]


def find_port(stream_ports, name):
    for port in stream_ports:
        if port_info_map[name] == port.info['name']:
            return port
        else:
            return None


def check_empty_port_iterator(stream_ports):
    port_count = 0
    for port in stream_ports:
        port_count += 1
    assert port_count == 0

def check_ports(route, stream_ports):
    assert len(stream_ports) == 2
    port_count = 0
    for port  in stream_ports:
        assert port.info['index'] == port_count
        assert port_info_map[port.info['name']] == port.info
        assert port == route[port.info['name']]
        port_count += 1
    assert port_count == 2



if __name__== "__main__":
    service.RoutingService.init()

    service = service.RoutingService(
            cfg_file='../test/RsPyTest.xml',
            service_name='RsPyTest',
            domain_id_base=195,
            service_verbosity=4)
    service.start()

    time.sleep(5)


