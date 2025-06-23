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

#include <mutex>

#include <modbus.h>

#include "ModbusClient.hpp"

#define MODBUS_CONNECTION_INITIALIZER \
    {                                 \
        nullptr, "", 0                \
    }

namespace rti { namespace adapter { namespace modbus {

/**
 * @class LibModbusClient
 *
 * @brief implementation of the ModbusClient.
 *
 * This class implements all the functions in ModbusClient for libmodbus
 */

class LibModbusClient : public ModbusClient {
public:

    static const uint8_t MODBUS_DEFAULT_DEVICE_ID = MODBUS_TCP_SLAVE;

    /**
     * @brief Default constructor
     */
    LibModbusClient();

    /**
     * @brief Parametrized constructor
     * @details constructor that creates a libmodbus connextion
     * based on the IP and port provided
     *
     * @see connect
     */
    LibModbusClient(const std::string& ip, uint16_t port);

    ~LibModbusClient();

    /**
     * @brief Creates a new libmodbus connection (modbus_connection_)
     * and connects to it.
     * @param ip IP of the modbus device
     * @param port Port of the modbus device to connect
     *
     * @see disconnect
     */
    void connect(const std::string& ip, uint16_t port);


    /**
     * @brief Finalizes the libmodbus connection (modbus_connection_)
     *
     * @see connect
     */
    void disconnect();

    /**
     * @brief modbus_connection getter
     */
    modbus_t *modbus_connection()
    {
        return modbus_connection_;
    }

    /**
     * @brief Sets a modbus device ID to read data from. This is used when
     * reading data from a modbus RTU thought a modbus gateway.
     * @param slave_id the slave ID of the modbus RTU device.
     *
     * @see get_slave_id
     */
    void set_slave_id(uint8_t slave_id);

    /**
     * @brief Gets the modbus device ID that is set to this connection.
     * @param slave_id the slave ID of the modbus RTU device.
     *
     * @see set_slave_id
     */
    int get_slave_id();


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
     * @see read_registers write_coils read_coils
     */
    int write_registers(
            uint32_t address,
            uint32_t register_count,
            const std::vector<uint16_t>& registers);

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
     * @see write_registers write_coils read_coils
     */
    int read_registers(
            std::vector<uint16_t>& registers,
            uint32_t address,
            uint32_t register_count,
            bool read_input_registers);

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
     * @see write_registers read_registers read_coils
     */
    int write_coils(
            uint32_t address,
            uint32_t register_count,
            const std::vector<uint8_t>& values);

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
     * @see write_registers read_registers write_coils
     */
    int read_coils(
            std::vector<uint8_t>& values,
            uint32_t address,
            uint32_t register_count,
            bool read_discrete_inputs);

    /**
     * @brief Sets the response timeout for write/read in libmodbus.
     * @param sec Seconds of the response timout.
     * @param usec Microseconds of the response timout.
     */
    void set_response_timeout(uint32_t sec, uint32_t usec);

    /**
     * @brief Translate a uint32 into an array of uint16
     * @param [out] dest destination as a C array
     * @param origin number to translate
     */
    static void int32_to_int16(uint16_t *dest, int32_t origin)
    {
        MODBUS_SET_INT32_TO_INT16(dest, 0, origin);
    }
    /**
     * @brief Translate a uint64 into an array of uint16
     * @param [out] dest destination as a C array
     * @param origin number to translate
     */
    static void int64_to_int16(uint16_t *dest, int64_t origin)
    {
        MODBUS_SET_INT64_TO_INT16(dest, 0, origin);
    }

    /**
     * @brief Translate an array of uint16 into a uint32
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_int32(uint32_t &dest, uint16_t *origin)
    {
        dest = MODBUS_GET_INT32_FROM_INT16(origin, 0);
    }
    /**
     * @brief Translate an array of uint16 into an int64
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_int64(uint64_t &dest, uint16_t *origin)
    {
        dest = MODBUS_GET_INT64_FROM_INT16(origin, 0);
    }
    /**
     * @brief Translate a float with byte order ABCD into an array of int16
     * @param [out] dest destination as a C array
     * @param origin number to translate
     */
    static void float_to_int16_abcd(uint16_t *dest, float origin)
    {
        modbus_set_float_abcd(origin, dest);
    }
    /**
     * @brief Translate a float with byte order BADC into an array of int16
     * @param [out] dest destination  as a C array
     * @param origin number to translate
     */
    static void float_to_int16_badc(uint16_t *dest, float origin)
    {
        modbus_set_float_badc(origin, dest);
    }
    /**
     * @brief Translate a float with byte order CDAB into an array of int16
     * @param [out] dest destination as a C array
     * @param origin number to translate
     */
    static void float_to_int16_cdab(uint16_t *dest, float origin)
    {
        modbus_set_float_cdab(origin, dest);
    }
    /**
     * @brief Translate a float with byte order DCBA into an array of int16
     * @param [out] dest destination as a C array
     * @param origin number to translate
     */
    static void float_to_int16_dcba(uint16_t *dest, float origin)
    {
        modbus_set_float_dcba(origin, dest);
    }
    /**
     * @brief Translate an array of int16 into a float with byte order ABCD
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_float_abcd(float &dest, uint16_t *origin)
    {
        dest = modbus_get_float_abcd(origin);
    }
    /**
     * @brief Translate an array of int16 into a float with byte order BADC
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_float_badc(float &dest, uint16_t *origin)
    {
        dest = modbus_get_float_badc(origin);
    }
    /**
     * @brief Translate an array of int16 into a float with byte order CDAB
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_float_cdab(float &dest, uint16_t *origin)
    {
        dest = modbus_get_float_cdab(origin);
    }
    /**
     * @brief Translate an array of int16 into a float with byte order DCBA
     * @param [out] dest destination
     * @param origin number to translate as a C array of uint16_t
     */
    static void int16_to_float_dcba(float &dest, uint16_t *origin)
    {
        dest = modbus_get_float_dcba(origin);
    }

private:
    modbus_t *modbus_connection_;
    std::mutex connection_mutex_;
    std::string ip_address_ = "";
    uint16_t port_number_ = 0;
};

}}}  // namespace rti::adapter::modbus
