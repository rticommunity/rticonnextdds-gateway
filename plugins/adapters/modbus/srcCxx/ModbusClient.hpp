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

#pragma once

#include <vector>

namespace rti { namespace adapter { namespace modbus {
class ModbusClient {
public:
    virtual void connect(const std::string& ip, uint16_t port) = 0;
    virtual void disconnect() = 0;

    /**
     * @brief Writes 1 or more registers to a modbus device.
     * @param address Address of the register/s that will be written in the
     * modbus device
     * @param register_count The number of register (starting at address) that
     * will be written in the modbus device
     * @param registers An array which contains the values that are written
     * to the modbus device.
     *
     * @return the number of registers written
     *
     * @see read_registers
     */
    virtual int write_registers(
            uint32_t address,
            uint32_t size,
            const std::vector<uint16_t>& registers) = 0;

    /**
     * @brief Writes 1 or more coils to a modbus device.
     * @param address Address of the coil/s that will be written in the
     * modbus device
     * @param register_count The number of coils (starting at address) that
     * will be written in the modbus device
     * @param values An array which contains the coils that are written
     * to the modbus device.
     *
     * @return the number of coils written
     *
     * @see read_coils
     */
    virtual int write_coils(
            uint32_t address,
            uint32_t size,
            const std::vector<uint8_t>& values) = 0;

    /**
     * @brief Reads 1 or more registers from a modbus device.
     * @param [out] registers array that will contain the registers that have
     * been read from a modbus device
     * @param address Address of the register/s that will be read in the modbus
     * device
     * @param register_count The number of register (starting at address) that
     * will be read in the modbus device
     * @param read_input_registers a flag that select whether we are reading
     * input registers (read-only) or holding registers
     *
     * @return the number of registers read
     *
     * @see write_registers
     */
    virtual int read_registers(
            std::vector<uint16_t>& registers,
            uint32_t address,
            uint32_t size,
            bool read_input_registers) = 0;
    /**
     * @brief Reads 1 or more coils from a modbus device.
     * @param [out] values array that will contain the coils that have
     * been read from a modbus device
     * @param address Address of the coil/s that will be read in the modbus
     * device
     * @param register_count The number of coil (starting at address) that
     * will be read in the modbus device
     * @param read_discrete_inputs a flag that select whether we are reading
     * discrete inputs (read-only) or coils
     *
     * @return the number of coils read
     *
     * @see write_coils
     */
    virtual int read_coils(
            std::vector<uint8_t>& values,
            uint32_t address,
            uint32_t size,
            bool read_discrete_inputs) = 0;

    /**
     * @brief Sets the response timeout for write/read operations.
     * @param sec Seconds of the response timout.
     * @param usec Microseconds of the response timout.
     */
    virtual void set_response_timeout(uint32_t sec, uint32_t usec) = 0;

};

}}}  // namespace rti::adapter::modbus
