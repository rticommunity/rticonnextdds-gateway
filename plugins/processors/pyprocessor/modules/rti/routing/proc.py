from abc import ABC, abstractmethod
from enum import IntEnum


class Processor(ABC):
    """ Processor interface definition.

    Provides a way to process Route events and  control the data forwarding
    process.

    A Processor receives event notifications from its Route owner in the form
    of operation callbacks. Each event occurrence will be dispatched to the
    Processor by calling the corresponding method.

    Each dispatching method has a signature according to the event kind and
    data it is associated with. Each event is associated with a Route state;
    hence limitations and constraints may apply individually to each method.

    Note that throwing an exception while processing any of the event
    notifications is allowed. In this situation, the Route owner will reject
    the events and none of the associated post conditions will be applied,
    including state transitions.

    Multi-threading safety:
    Partially Safe All operations on a concrete Processor object are safe and
    always serialized on a given Session. Operations on different Processor
    objects may be called concurrently if they belong to different Routes.
    """

    @abstractmethod
    def __init__(self, route, properties):
        """ Creates a Route Processor.
        :abstractmethod:

        This function is called when the Route containing the Processor is enabled.

        :param route: owner that contains this Processor
        :type route: Route
        :param dict properties: Configuration properties for the Processor.
         These properties corresponds to the properties specified within the
         tag ``<processor>``.
        """
        NotImplemented

    def on_input_enabled(self, route, input):
        """  Notification of the INPUT_ENABLED event.
        :abstractmethod:

        This operation is called when an Input enabled event occurs that affects
        any of the inputs contained in the owner Route.

        This operation is called right after the affected Input has been enabled.

        :param route: owner that contains this Processor
        :type route: Route
        :param input:  The just enabled Input
        :type input: Input
        """
        NotImplemented

    def on_input_disabled(self, route, input):
        """ Notification of the INPUT_DISABLED event.
        :abstractmethod:

        This operation is called when an INPUT_DISABLED event occurs that
        affects any of the inputs contained in the owner Route.

        This operation is called right before the affected Input is disabled.

        :param route: owner that contains this Processor
        :type route: Route
        :param input:  The Input about to be disabled.
        :type input: Input
        """
        NotImplemented

    def on_output_enabled(self, route, output):
        """  Notification of the OUTPUT_ENABLED event.
        :abstractmethod:

        This operation is called when an Output enabled event occurs that affects
        any of the outputs contained in the owner Route.

        This operation is called right after the affected Output has been enabled.

        :param route: owner that contains this Processor
        :type route: Route
        :param output:  The just enabled Output.
        :type output: Output
        """
        NotImplemented

    def on_output_disabled(self, route, output):
        """ Notification of the OUTPUT_DISABLED event.
        :abstractmethod:

        This operation is called when an OUTPUT_DISABLED event occurs that
        affects any of the outputs contained in the owner Route.

        This operation is called right before the affected Output is disabled.

        :param route: owner that contains this Processor
        :type route: Route
        :param output:  The Output about to be disabled.
        :type output: Output
        """
        NotImplemented

    def on_start(self, route):
        """ Notification of the ROUTE_STARTED event.
        :abstractmethod:

        This operation is called right before the Route enters the STARTED
        state. At the time this operation is called, all the inputs and
        outputs within the Route are enabled.

        :param route: owner that contains this Processor
        :type route: Route
        """
        NotImplemented

    def on_stop(self, route):
        """ Notification of the Route stopped event.
        :abstractmethod:

        This operation is called right before the Route enters the STOPPED
        state. At the time this operation is called, all the inputs and
        outputs within the Route are still enabled.

        :param route: owner that contains this Processor
        :type route: Route
        """
        NotImplemented

    def on_run(self, route):
        """  Notification of the Route RUN event.
        :abstractmethod:

        This operation is called right before the Route enters the RUNNING
        state. This operation is called after the Route went to STARTED after
        a successful notification to this Processor,

        If the Route was manually paused before via an Administration call,
        this operation will not be called until a manual run operation is performed.


        :param route: owner that contains this Processor
        :type route: Route
        """
        NotImplemented

    def on_pause(self, route):
        """ Notification of the Route paused event.
        :abstractmethod:

        This operation is called right before the Route enters the PAUSED
        state. At the time this operation is called, all the inputs and
        outputs within the Route are still enabled.

        :param route: owner that contains this Processor
        :type route: Route
        """
        NotImplemented

    def on_data_available(self, route):
        """  Notification of the Route DATA_AVAILABLE event.

        :abstractmethod:

        This operation is called each time any of the inputs contained in
        the owner Route is notified about new data. Notifications of this
        event can occur only while the Route is in the RUNNING state.

        Implementations are allowed to access any of the Input and Output of
        the owner Route to read and write data, respectively.

        :param route: owner that contains this Processor
        :type route: Route


        """
        NotImplemented

    def on_periodic_event(self, route):
        """ Notification of the Route periodic action event.
        :abstractmethod:

        This operation is called periodically at the rate specified in the
        parent Session of the Route owner.

        Periodic notifications can occur only while the Route is in the
        RUNNING state.

        Implementations are allowed to access any of the Input and Output of
        the owner Route to read and write data, respectively.

        :param route: owner that contains this Processor
        :type route: Route
        """
        NotImplemented


class Route:
    """ Representation of the Route object that owns a Processor """

    @property
    def inputs(self):
        """ Returns an iterator to the enabled Input objects within this Route

        Note that this iterator can be used only when the Route is started.
        Otherwise, the iterator will be finished.
        """
        NotImplemented

    @property
    def outputs(self):
        """ Returns an iterator to the enabled Output objects within this Route

        Note that this iterator can be used only when the Route is started.
        Otherwise, the iterator will be finished.
        """
        NotImplemented

class Input:
    """Generic Representation of a Route's input"""

    @property
    def info(self):
        """ Returns a dictionary containing information that uniquely describes this
        Input.

        The dictionary contains the following key items:

        .. code-block:: Python

            {
                "index" : <int>,
                "name": <str>,
                "stream_info" : {
                    "stream_name" : <str>,
                    "type_name":  <str>
                }
            }

        where
            - ``index``: The access index of this Input
            - ``name`` : The name of this Input, as specified in the XML
              configuration
            - ``stream_info``:
                - ``stream_name``: Name of the stream this input reads
                   data from
                - ``type_name``: Registered name of the type associated to the
                  stream and data
        """
        NotImplemented

    def take(self, selector={}):
        """ Returns all the available samples in this Input.

        This operation will call the take operation on the underlying StreamReader.

        Note that this operation will remove all the taken samples from the
        StreamReader's cache.

        An optional selector as a dictionary can be provided in order to read
        subset of data (e.g., a particular instance). The selector can
        contain the following key items:

         .. code-block:: Python

            {
                "sample_state" : <int>,
                "view_state" : <int>,
                "instance_state" : <int>,
                "max_samples" : <int>,
                "instance" : {<bool>, <Bytes>},
                "next_instance" :{<bool>, <Bytes>},
                "filter" : {
                    "expression" : <str>
                }
            }

         where
            - ``sample_state``: The sample read state as DDS_SampleStateKind
            - ``view_state`` : the instance view state as DDS_ViewStateKind
            - ``instance_state`` : the instance view state as DDS_InstanteStateKind
            - ``max_samples`` : Choose to only read/take up to a maximum number of samples.
            - ``instance`` :
                - ``valid:``: Indicates whether the handle is valid or not
                - ``value``: A 16-byte list representing a handle to the instance
                  to read/take. Setting this field causes the read/take to access
                  only samples belonging the single
                  specified instance handle. The read/take may
                  operation may fail if the handle does not
                  correspond to an existing data-object known to this Input.
            - ``next_instance``:
                - ``valid``: Indicates whether the handle is valid or not
                - `value``: A 16-byte list representing a handle to the
                  instance from which to read next. The subsequent read or take
                  operation to access only samples belonging a single instance
                  whose handle is considered 'next' after the provided handle.
                  The accessed samples will all belong to the 'next' instance with
                  handle 'greater' than the specified previous handle that has
                  available samples. An invalid handle can be provided as
                  "less than" any valid handle so the read/take will return the
                  samples for the instance that has the smallest handle among a
                  all the instances that contain available samples.

                  Note that it is possible to provide a handle that does not c
                  correspond to an instance currently managed by the underlying
                  StreamReader.
            - ``filter``: A dictionary that represents a content filter
                - ``expression``: An expression selection a subset of data based
                  on its content.

        :param dict selector: Ooptional. A dictionary that represents a selector of a
         a subset of the data to be read.

        :returns: A LoanedSamples object.
        :rtype: List of Sample
        """
        NotImplemented

    def read(self, selector={}):
        """ Returns all the available samples in this Input.

        Similar to take() except that the read samples will remain in the
        undelying StreamReader's cache

        """
        NotImplemented

class Output:
    """Generic Representation of a Route's output"""

    @property
    def info(self):
        """ Returns a dictionary containing information that uniquely describes this
        Output.

        The dictionary contains the following key items:

        .. code-block:: Python

            {
                "index" : <int>,
                "name": <str>,
                "stream_info" : {
                    "stream_name" : <str>,
                    "type_name":  <str>
                }
            }

        where
            - ``index``: The access index of this output
            - ``name`` : The name of this Input, as specified in the XML
              configuration
            - ``stream_info``:
                - ``stream_name``: Name of the stream this output reads
                   data from
                - ``type_name``: Registered name of the type associated to the
                  stream and data
        """

        NotImplemented

    def write(self, sample):
        """
        Writes the specified data and info sample in this output.

        This operation will call the write operation on the underlying StreamWriter.

        If an Sample as` parameter is provided, the underlying StreamWriter::write
        implementation may use the ``info`` portion to propagate some of the
        metadata elements.

        :param sample: An object representing a Sample or a data dictionary. If
         a dictionary, it must represent the sample data in the same format as
         in Sample.data
        :type sample: dict or Sample


        """
        NotImplemented


class LoanedSamples:
    """ Representation of a list of  samples loaned from an Input after
        a read or take operation.

        This class abstracts a list of Sample objects. The loane is returned
        once the object is collected by the Python GC.

        Note that this object must outlive any of its contained Sample objects.
        If your Processor implementation keeps a reference to a loaned Sample
        object and access it after the loaned is destroyed, a crash will occur.

        If you want to keep a Sample object for future access, keeps its data or
        info portion instead.

    """

class Sample:
    """  Representation of sample objects returned by a read/take operation.

    It's composed of two items, the sample data and sample info (metadata).

    :attribute: valid_data (bool)
     Indicates whether the data portion of the sample has valid readable
     content.

    :attribute: data (dict) User-data portion of the sample.

    Represented as dictionary where the key is the  member name as string
    and the value is the member value. The member values can be:

    - Long: For all size of signed an unsigned integers, as well as enums
    - Float: For float32 and float64
    - Unicode: for string and wide-string
    - List: for arrays and sequences
    - Dictionary: For complex member

    For example consider the following type in IDL:

    .. code-block:: C

        struct OtherType {
            int16 m_short;
            string m_string
        };

        struct MyType{
            int64 m_long;
            int32 m_array[10]
            OtherType m_other;
        };

    A sample of ``MyType`` would map to the following dictionary:

    .. code-block:: Python

        {
            "m_long" : <int>,
            "m_array": [<integer_0>, ... ,<integer_9>]
            "m_other" : {
                "m_short" : <int>,
                "m_string":  <str>
            }
        }

    Mutable.

    :attribute: info (dict) Metadata portion of the sample (as DDS_SampleInfo).

    Represented as dictionary where the key is the  member name as string
    and the value is the member value.

    Supported members are:

    .. code-block:: Python

        {
            "instance_handle" : {<bool>, <Bytes>},
            "publication_handle": {<bool>, <Bytes>},
            "sample_state" : <int>,
            "view_state" : <int>,
            "instance_state" : <int>,
            "valid_data" : <int>,
            "flag" : <int>,
            "original_publication_virtual_sequence_number" : {<int>, <int>},
            "original_publication_virtual_guid" : [<int>],
            "related_original_publication_virtual_sequence_number" :{ <int>, <int>},
            "related_original_publication_virtual_guid" : [<int>],
            "reception_sequence_number" :{<int>, <int>},
            "publication_sequence_number" : {<int>, <int>},
            "reception_timestamp" : {<int>, <int>},
            "source_timestamp" : {<int>, <int>}
        }

    For information about each key of the ``info`` dictionary, see
    RTI Connext DDS User's Manual.

    """

    def __getitem__(self, name):
        """ Provides access to the specified member of the data portion of
        this Sample.

        This is equivalent to calling Sample.data[name]

        """
        NotImplemented


