/*
* (c) Copyright, Real-Time Innovations, 2022.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <iostream>

#include <dds/pub/ddspub.hpp>
#include <rti/util/util.hpp>      // for sleep()
#include <rti/config/Logger.hpp>  // for logging
// alternatively, to include all the standard APIs:
//  <dds/dds.hpp>
// or to include both the standard APIs and extensions:
//  <rti/rti.hpp>
//
// For more information about the headers and namespaces, see:
//    https://community.rti.com/static/documentation/connext-dds/7.3.0/doc/api/connext_dds/api_cpp2/group__DDSNamespaceModule.html
// For information on how to use extensions, see:
//    https://community.rti.com/static/documentation/connext-dds/7.3.0/doc/api/connext_dds/api_cpp2/group__DDSCpp2Conventions.html

#include "application.hpp"  // for command line parsing and ctrl-c
#include "testPublisher.hpp"

void run_publisher_application(unsigned int domain_id, unsigned int sample_count)
{
    // DDS objects behave like shared pointers or value types
    // (see https://community.rti.com/best-practices/use-modern-c-types-correctly)

    // Start communicating in a domain, usually one participant per application
    dds::domain::DomainParticipant participant(domain_id);

    // Create a Topic with a name and a datatype
    dds::topic::Topic<MyTypeWithSequences> topic(participant, "MyTypeWithSequencesTopic");

    // Create a Publisher
    dds::pub::Publisher publisher(participant);

    // Create a DataWriter with default QoS
    dds::pub::DataWriter<MyTypeWithSequences> writer(publisher, topic);

    std::unique_ptr<MyTypeWithSequences> data(new MyTypeWithSequences());
    for (unsigned int samples_written = 0;
        !application::shutdown_requested && samples_written < sample_count;
        samples_written++) {
        // create auxiliary struct value
        auto structValue1 = NewStruct1(
                "test" + std::to_string(samples_written),
                static_cast<double>(samples_written),
                static_cast<int8_t>(samples_written),
                NewEnum1::ENUM_VALUE_1);
        switch(samples_written % 3) {
            case 0: structValue1.member4(NewEnum1::ENUM_VALUE_1);
            break;
            case 1: structValue1.member4(NewEnum1::ENUM_VALUE_2);
            break;
            default: structValue1.member4(NewEnum1::ENUM_VALUE_3);
            break;
        }

        // create member1 of the data to sent
        std::vector<NewStruct1> member1;
        member1.push_back(structValue1);
        member1.push_back(structValue1);
        member1.push_back(structValue1);

        data->member1(member1);

        // create member2 of the data to sent
        std::vector<NewEnum1> member2;
        member2.push_back(structValue1.member4());
        member2.push_back(structValue1.member4());
        member2.push_back(structValue1.member4());

        data->member2(member2);


        // create auxiliary union value
        auto unionValue1 = NewUnion1();
        switch(samples_written % 3) {
            case 0: unionValue1.NewMember1(static_cast<uint16_t>(samples_written));
            break;
            case 1: unionValue1.NewMember2(samples_written % 2 == 0? false : true);
            break;
            default: unionValue1.NewMember3(member1);
            break;
        }

        // create member3 of the data to sent
        std::vector<NewUnion1> member3;
        member3.push_back(unionValue1);
        member3.push_back(unionValue1);
        member3.push_back(unionValue1);

        data->member3(member3);

        // create member4 of the data to sent
        std::array<NewStruct1, 2> member4;
        member4[0] = structValue1;
        member4[1] = structValue1;

        data->member4(member4);

        // create member5 of the data to sent
        data->member5(structValue1);

        // create member6 of the data to sent
        data->member6(static_cast<uint64_t>(samples_written));

        // create member7 of the data to sent
        data->member7(structValue1.member4());

        std::cout << "Writing MyTypeWithSequences, count " << samples_written << std::endl;

        writer.write(*data);

        // Send once every second
        rti::util::sleep(dds::core::Duration(1));
    }
}

int main(int argc, char *argv[])
{

    using namespace application;

    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_publisher_application(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_publisher_application(): " << ex.what()
        << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
