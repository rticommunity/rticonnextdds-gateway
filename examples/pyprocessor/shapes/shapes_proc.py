import os
import sys
import time
from typing import Dict

sys.path.insert(0, os.path.abspath('../../../resource/pyprocessor/modules'))

from rti.routing import proc


class ShapesProcessor(proc.Processor):
    def __init__(self, route: proc.Route, properties: Dict):
        pass

    def on_data_available(self, route: proc.Route) -> None:
        # Use squares as 'leading' input. For each Square instance, get the
        # equivalent instance from the Circle topic
        with route.inputs['Square'].read() as squares:
            for shape in squares:
                if shape.valid_data:
                    # read equivalent existing instance in the Circles Topic
                    selector = shape.info['instance_handle']
                    with route.inputs['Circle'].read(selector) as circles:
                        if len(circles) != 0 and circles[0].valid_data:
                            shape.data['shapesize'] = circles[0].data['y']

                    route.outputs['Triangle'].write(shape.data)
                else:
                    # propagate dispose/unregister instance
                    route.outputs['Triangle'].write(shape)
                    # clear cache
                    route.inputs['Square'].take(dict(instance=shape.info['instance_handle']))


def create_processor(route: proc.Route, properties: Dict) -> proc.Processor:
    return ShapesProcessor(route, properties)
