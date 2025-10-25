/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/
#include "BaseTester.hpp"

namespace rti {
namespace gateway {
namespace test {

void BaseTester::test_up(const int32_t domain_id)
{
    // Configure participant factory to create participatns disabled
    dds::domain::qos::DomainParticipantFactoryQos factory_qos;
    factory_qos << dds::core::policy::EntityFactory::ManuallyEnable();
    dds::domain::DomainParticipant::participant_factory_qos(factory_qos);

    dds_participant_ = this->create_dds_participant(domain_id);
    if (nullptr == dds_participant_) {
        throw std::runtime_error("Failed to create the DDS DomainParticipant");
    }

    dds_subscriber_ = this->create_dds_subscriber();
    if (nullptr == dds_subscriber_) {
        throw std::runtime_error("Failed to create the DDS Subscriber");
    }

    dds_publisher_ = this->create_dds_publisher();
    if (nullptr == dds_publisher_) {
        throw std::runtime_error("Failed to create the DDS Publisher");
    }
}


void BaseTester::test_down()
{
    if (nullptr != dds_participant_) {
        dds_participant_->close_contained_entities();
        dds_participant_->close();
    }
}

dds::domain::DomainParticipant
BaseTester::create_dds_participant(const int32_t domain_id)
{
    dds::domain::DomainParticipant participant(domain_id);
    std::cout
        << "[" << tester_id_ << "] "
        << "Created DomainParticipant in domain " << domain_id << std::endl;
    return participant;
}

dds::pub::Publisher
BaseTester::create_dds_publisher()
{
    return dds::pub::Publisher(dds_participant_);
}

dds::sub::Subscriber
BaseTester::create_dds_subscriber()
{
    return dds::sub::Subscriber(dds_participant_);
}

void
BaseTester::run() {
    throw std::runtime_error("Not implemented");
}

} // namespace test
} // namespace gateway
} // namespace rti
