/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#include <iostream>
#include <sstream>

#include <modbus.h>

#include "LibModbusClient.hpp"

using namespace rti::adapter::modbus;

LibModbusClient::LibModbusClient()
{
    modbus_connection_ = nullptr;
}

LibModbusClient::LibModbusClient(const std::string& ip, unsigned int port)
{
    connect(ip, port);
}
LibModbusClient::~LibModbusClient()
{
    disconnect();
}

void LibModbusClient::connect(const std::string& ip, unsigned int port)
{
    // Try to open a new TCP connection to the Modbus device
    modbus_connection_ = modbus_new_tcp(ip.c_str(), (int) port);
    if (modbus_connection_ == nullptr) {
        std::string error(
                "Error initializing Modbus <" + ip + ":" + std::to_string(port)
                + "> " + modbus_strerror(errno));
        throw std::runtime_error(error);
    }
    std::string ip_address_ = ip;
    unsigned int port_number_ = port;

    // If the connection was successfull, connect to it
    if (modbus_connect(modbus_connection()) != 0) {
        std::string error(
                "Error connecting to Modbus server <" + ip + ":"
                + std::to_string(port) + "> " + modbus_strerror(errno));
        throw std::runtime_error(error);
    }
}

void LibModbusClient::disconnect()
{
    // Close the connection
    if (modbus_connection() != NULL) {
        modbus_close(modbus_connection());
        modbus_free(modbus_connection());
        modbus_connection_ = NULL;
    }
}

int LibModbusClient::write_registers(
        uint32_t address,
        uint32_t register_count,
        const std::vector<uint16_t>& registers)
{
    int written_registers = 0;
    std::lock_guard<std::mutex> guard(connection_mutex_);

    // Differentiate when writing 1 ore more registers
    if (register_count == 1) {
        written_registers = modbus_write_register(
                modbus_connection(),
                address,
                registers[0]);
    } else {
        written_registers = modbus_write_registers(
                modbus_connection(),
                address,
                register_count,
                const_cast<uint16_t *>(registers.data()));
    }
    if (written_registers < 1) {
        std::string modbus_error(modbus_strerror(errno));
        throw std::runtime_error("Error writing registers: " + modbus_error);
    }
    return written_registers;
}

int LibModbusClient::read_registers(
        std::vector<uint16_t>& registers,
        uint32_t address,
        uint32_t register_count,
        bool read_input_registers)
{
    int read_registers = 0;
    auto registers_data = reinterpret_cast<uint16_t *>(registers.data());
    std::lock_guard<std::mutex> guard(connection_mutex_);

    // Differentiate when reading input registers or holding registers
    if (read_input_registers) {
        read_registers = modbus_read_input_registers(
                modbus_connection(),
                address,
                register_count,
                registers_data);
    } else {
        read_registers = modbus_read_registers(
                modbus_connection(),
                address,
                register_count,
                registers_data);
    }
    if (read_registers < 1) {
        std::string modbus_error(modbus_strerror(errno));
        throw std::runtime_error("Error reading registers: " + modbus_error);
    }
    return read_registers;
}

int LibModbusClient::write_coils(
        uint32_t address,
        uint32_t register_count,
        const std::vector<uint8_t>& values)
{
    int written_registers = 0;
    std::lock_guard<std::mutex> guard(connection_mutex_);

    // Differentiate when reading 1 or more coils
    if (register_count == 1) {
        written_registers = modbus_write_bit(
                modbus_connection(),
                address,
                values[0] ? TRUE : FALSE);
    } else {
        written_registers = modbus_write_bits(
                modbus_connection(),
                address,
                register_count,
                values.data());
    }
    if (written_registers < 1) {
        std::string modbus_error(modbus_strerror(errno));
        throw std::runtime_error("Error writing coils: " + modbus_error);
    }
    return written_registers;
}

int LibModbusClient::read_coils(
        std::vector<uint8_t>& registers,
        uint32_t address,
        uint32_t register_count,
        bool read_discrete_inputs)
{
    int read_registers = 0;
    std::lock_guard<std::mutex> guard(connection_mutex_);

    // Differentiate when reading discrete inputs or coils
    if (read_discrete_inputs) {
        read_registers = modbus_read_input_bits(
                modbus_connection(),
                address,
                register_count,
                reinterpret_cast<uint8_t *>(registers.data()));
    } else {
        read_registers = modbus_read_bits(
                modbus_connection(),
                address,
                register_count,
                reinterpret_cast<uint8_t *>(registers.data()));
    }
    if (read_registers < 1) {
        std::string modbus_error(modbus_strerror(errno));
        throw std::runtime_error("Error reading coils: " + modbus_error);
    }
    return read_registers;
}

void LibModbusClient::set_response_timeout(uint32_t sec, uint32_t usec) {
    if (modbus_set_response_timeout(modbus_connection(), sec, usec) != 0) {
        std::string error(
                "Error setting response timeout " + std::to_string(sec)
                + "secs, " + std::to_string(usec) + "usecs, to Modbus server <"
                + ip_address_ + ":" + std::to_string(port_number_) + "> "
                + modbus_strerror(errno));
        throw std::runtime_error(error);
    }
}
