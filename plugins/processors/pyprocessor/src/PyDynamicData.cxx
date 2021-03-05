/*
 * (c) 2019 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

#include "Python.h"

#include <stack>
#include <string>
#include <memory>

#include "dds/core/xtypes/StructType.hpp"
#include "dds/core/xtypes/UnionType.hpp"
#include "dds/core/xtypes/MemberType.hpp"
#include "dds/core/xtypes/AliasType.hpp"

#include "PyDynamicData.hpp"
#include "NativeUtils.hpp"


using namespace dds::core::xtypes;

namespace rti { namespace routing { namespace py {

DynamicDataConverter::DynamicDataConverter(
        PyObject *py_dict,
        dds::core::xtypes::DynamicData& data)
{
    context_stack_.push(py_dict);
    build_dynamic_data(data);
}

DynamicDataConverter::DynamicDataConverter(const dds::core::xtypes::DynamicData& data)
{
    context_stack_.push(PyDict_New());
    dds::core::xtypes::DynamicData& casted_data =
            const_cast<dds::core::xtypes::DynamicData&>(data);
    for (int i = 0; i < data.member_count(); i++) {
        if (data.member_exists(i+1)) {
            build_dictionary(casted_data, data.member_info(i + 1));
        }
    }
}

PyObject* DynamicDataConverter::from_dynamic_data(
        const dds::core::xtypes::DynamicData& data)
{
    DynamicDataConverter converter(data);
    return converter.context_stack_.top();
}


template<>
void DynamicDataConverter::from_native_primitive<std::string, const char*>(
        const dds::core::xtypes::DynamicData &data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        std::function<PyObject*(const char*) > to_python_object)
{

    std::string value = data.value<std::string>(member_info.member_index());
    PyObjectGuard py_value = to_python_object(value.c_str());
    if (PyDict_Check(context_stack_.top())) {
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                py_value.get()) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter:"
                    + std::string(RTI_FUNCTION_NAME)
                    + "error member=" + member_info.member_name().to_std_string());
        }
    } else {
        assert(PyList_Check(context_stack_.top()));
        if (PyList_SetItem(
                context_stack_.top(),
                context_stack_.top().index,
                py_value.get()) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error element="
                    + std::to_string(context_stack_.top().index));
        }
        py_value.release();
    }
}

void DynamicDataConverter::from_wstring(
        const dds::core::xtypes::DynamicData &data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info)
{
    using rti::core::xtypes::LoanedDynamicData;
    static int32_t UTF16_BYTE_ORDER = 0;

    DDS_Wchar *wstring_value = NULL;
    DDS_UnsignedLong wstring_size = 0;
    if (DDS_DynamicData_get_wstring(
            &(data.native()),
            &wstring_value,
            &wstring_size,
            NULL,
            member_info.member_index()) != DDS_RETCODE_OK) {
        throw dds::core::Error(
                    "DynamicDataConverter:"
                    + std::string(RTI_FUNCTION_NAME)
                    + "error reading member=" + member_info.member_name().to_std_string());
    }
    PyObjectGuard py_wstring = PyUnicode_DecodeUTF16(
            (const char *) wstring_value,
            wstring_size * BYTES_PER_WCHAR,
            member_info.member_name().c_str(),
            &UTF16_BYTE_ORDER);
    if (py_wstring.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                    "DynamicDataConverter:"
                    + std::string(RTI_FUNCTION_NAME)
                    + "error decoding UTF-16 from member=" + member_info.member_name().to_std_string());
    }
    DDS_Wstring_free(wstring_value);
    if (PyDict_Check(context_stack_.top())) {
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                py_wstring.get()) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter:"
                    + std::string(RTI_FUNCTION_NAME)
                    + "error member=" + member_info.member_name().to_std_string());
        }
    } else {
        assert(PyList_Check(context_stack_.top()));
        if (PyList_SetItem(
                context_stack_.top(),
                context_stack_.top().index,
                py_wstring.get()) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error element="
                    + std::to_string(context_stack_.top().index));
        }
        py_wstring.release();
    }
}

void DynamicDataConverter::build_dictionary(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info)
{
    using rti::core::xtypes::LoanedDynamicData;
    using rti::core::xtypes::DynamicDataMemberInfo;

    switch (member_info.member_kind().underlying()) {
    case TypeKind::STRUCTURE_TYPE:
    {
        PyObject *py_dict = PyDict_New();
        PyObjectGuard py_guard = py_dict;
        if (py_dict == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error creating dictionary");
        }

        if (PyDict_Check(context_stack_.top())) {
            if (PyDict_SetItemString(
                    context_stack_.top(),
                    member_info.member_name().c_str(),
                    py_dict) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dictionary: error member="
                        + member_info.member_name().to_std_string()
                        + " of type=struct");
            }
        } else {
            assert(PyList_Check(context_stack_.top()));
            if (PyList_SetItem(
                    context_stack_.top(),
                    context_stack_.top().index,
                    py_dict) != 0) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dictionary: error member="
                        + member_info.member_name().to_std_string()
                        + " at index=" + std::to_string(context_stack_.top().index));
            }
            py_guard.release();
        }


        context_stack_.push(py_dict);

        LoanedDynamicData loaned_member =
                data.loan_value(member_info.member_index());
        for (int i = 0; i < loaned_member.get().member_count(); i++) {
            build_dictionary(
                    loaned_member.get(),
                    loaned_member.get().member_info(i + 1));
        }

        context_stack_.pop();
    }
        break;

    case TypeKind::WSTRING_TYPE:

        from_wstring(data, member_info);

        break;

    case TypeKind::SEQUENCE_TYPE:
    {
        PyObjectGuard py_list = PyList_New(member_info.element_count());
        if (py_list.get() == NULL) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error creating list");
        }
        if (PyDict_SetItemString(
                context_stack_.top(),
                member_info.member_name().c_str(),
                py_list.get()) != 0) {
            PyErr_Print();
            throw dds::core::Error(
                    "DynamicDataConverter::build_dictionary: error member="
                    + member_info.member_name().to_std_string()
                    + " of type=sequence");
        }

        context_stack_.push(py_list.get());

        LoanedDynamicData loaned_array =
                data.loan_value(member_info.member_name());
        for (uint32_t i = 0; i < member_info.element_count(); i++) {
            build_dictionary(
                    loaned_array.get(),
                    loaned_array.get().member_info(i + 1));
            ++(context_stack_.top().index);
        }

        context_stack_.pop();
    }
        break;

    case TypeKind::ARRAY_TYPE:
    {
        std::vector<uint32_t> dimension_indexes;
        std::vector<uint32_t> dimensions;
        uint32_t dimension_count = 0;
        LoanedDynamicData loaned_array =
                data.loan_value(member_info.member_index());
        const ArrayType& array_type =
                static_cast<const ArrayType &> (loaned_array.get().type());
        dimension_count = array_type.dimension_count();
        dimension_indexes.resize(dimension_count);
        dimensions.resize(dimension_count);
        for (uint32_t j = 0; j < dimension_count; j++) {
            dimensions[j] = array_type.dimension(j);
        }

        uint32_t element_count = 0;
        while (element_count < member_info.element_count()) {
            for (uint32_t j = 0; j < dimension_count; j++) {

                if ((j < dimension_count - 1)
                        && dimension_indexes[j+1] != 0) {
                    continue;
                }

                if (dimension_indexes[j] == 0) {
                    PyObject *py_list = PyList_New(dimensions[j]);
                    PyObjectGuard py_guard = py_list;
                    if (py_list == NULL) {
                        PyErr_Print();
                        throw dds::core::Error(
                                "DynamicDataConverter::build_dictionary: error creating list");
                    }
                    if (PyDict_Check(context_stack_.top())) {
                        if (PyDict_SetItemString(
                                context_stack_.top(),
                                member_info.member_name().c_str(),
                                py_list) != 0) {
                            PyErr_Print();
                            throw dds::core::Error(
                                    "DynamicDataConverter::build_dictionary: error member="
                                    + member_info.member_name().to_std_string()
                                    + " of type=array");
                        }
                    } else {
                        assert(PyList_Check(context_stack_.top()));
                        if (PyList_SetItem(
                                context_stack_.top(),
                                context_stack_.top().index,
                                py_list) != 0) {
                            PyErr_Print();
                            throw dds::core::Error(
                                    "DynamicDataConverter::build_dictionary: error member="
                                    + member_info.member_name().to_std_string()
                                    + " at index=" + std::to_string(context_stack_.top().index));
                        }
                        py_guard.release();
                    }
                    context_stack_.push(py_list);
                }
            }


            build_dictionary(
                    loaned_array.get(),
                    loaned_array.get().member_info(element_count + 1));
            ++(context_stack_.top().index);

            ++dimension_indexes[dimension_count - 1];
            for (int64_t j = dimension_count - 1; j >= 0; j--) {
                if (dimension_indexes[j] == dimensions[j]) {
                    if (j > 0) {
                        ++dimension_indexes[j - 1];
                    }
                    dimension_indexes[j] = 0;
                    context_stack_.pop();
                    ++(context_stack_.top().index);
                }
            }

            ++element_count;
        }
    }
        break;


    case TypeKind::BOOLEAN_TYPE:

        from_native_primitive<DDS_Boolean, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::CHAR_8_TYPE:

        from_native_primitive<DDS_Char, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_8_TYPE:

        from_native_primitive<uint8_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::INT_16_TYPE:

        from_native_primitive<int16_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_16_TYPE:

        from_native_primitive<uint16_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::INT_32_TYPE:

        from_native_primitive<int32_t, DDS_Long>(
                data,
                member_info,
                PyLong_FromLong);

        break;

    case TypeKind::UINT_32_TYPE:

        from_native_primitive<uint32_t, DDS_UnsignedLong>(
                data,
                member_info,
                PyLong_FromUnsignedLong);

        break;

    case TypeKind::INT_64_TYPE:
    case TypeKind::ENUMERATION_TYPE:

        from_native_primitive<int64_t, DDS_LongLong>(
                data,
                member_info,
                PyLong_FromLongLong);

        break;

    case TypeKind::UINT_64_TYPE:

        from_native_primitive<uint64_t, DDS_UnsignedLongLong>(
                data,
                member_info,
                PyLong_FromUnsignedLong);

        break;

    case TypeKind::STRING_TYPE:

        from_native_primitive<std::string, const char*>(
                data,
                member_info,
                PyUnicode_FromString);

        break;

    case TypeKind::FLOAT_32_TYPE:
    {
        from_native_primitive<float_t, double>(
                data,
                member_info,
                PyFloat_FromDouble);
    }

        break;

    case TypeKind::FLOAT_64_TYPE:
    {
        from_native_primitive<double, double>(
                data,
                member_info,
                PyFloat_FromDouble);
    }

        break;

    default:
        std::string message =
                "unsupported type for member="
                + member_info.member_name().to_std_string()
                + ". Skipping deserialization.";
        DDSLog_logWithFunctionName(
                RTI_LOG_BIT_WARN,
                "DynamicDataConverter::build_dictionary",
                &RTI_LOG_ANY_s,
                message.c_str());
    }

}

void DynamicDataConverter::to_dynamic_data(
        dds::core::xtypes::DynamicData& data,
        PyObject* py_dict)
{
    DynamicDataConverter converter(py_dict, data);
}

template <>
void DynamicDataConverter::to_native_primitive<uint64_t>(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject* py_value,
        std::function<uint64_t(PyObject*) > as_primitve)

{
    using dds::core::xtypes::TypeKind;

    uint64_t primitive_value = as_primitve(py_value);

    switch (member_info.member_kind().underlying()) {

    case TypeKind::CHAR_8_TYPE:
        data.value<DDS_Char>(
                member_info.member_index(),
                (DDS_Char) primitive_value);
        break;
    case TypeKind::UINT_8_TYPE:
        data.value<uint8_t>(
                member_info.member_index(),
                (uint8_t) primitive_value);
        break;
    case TypeKind::INT_16_TYPE:
        data.value<int16_t>(
                member_info.member_index(),
                (int16_t) primitive_value);
        break;
    case TypeKind::UINT_16_TYPE:
        data.value<uint16_t>(
                member_info.member_index(),
                (uint16_t) primitive_value);
        break;
    case TypeKind::INT_32_TYPE:
    case TypeKind::ENUMERATION_TYPE:
        data.value<int32_t>(
                member_info.member_index(),
                (int32_t) primitive_value);
        break;
    case TypeKind::UINT_32_TYPE:
        data.value<uint32_t>(
                member_info.member_index(),
                (uint32_t) primitive_value);
        break;

    case TypeKind::INT_64_TYPE:
        data.value<int64_t>(
                member_info.member_index(),
                (int64_t) primitive_value);
        break;
    case TypeKind::UINT_64_TYPE:
        data.value<uint64_t>(
                member_info.member_index(),
                (uint64_t) primitive_value);

        break;

    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member id="
                + std::to_string(member_info.member_index()));
    }
}

template <>
void DynamicDataConverter::to_native_primitive<double>(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject* py_value,
        std::function<double(PyObject*) > as_primitve)

{
    using dds::core::xtypes::TypeKind;

    double primitive_value = as_primitve(py_value);

    switch (member_info.member_kind().underlying()) {

    case TypeKind::FLOAT_32_TYPE:
        data.value<float_t>(
                member_info.member_index(),
                (float_t) primitive_value);
        break;

    case TypeKind::FLOAT_64_TYPE:
        data.value<double>(
                member_info.member_index(),
                (double) primitive_value);
        break;

    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member id="
                + std::to_string(member_info.member_index()));
    }
}


template <>
void DynamicDataConverter::to_native_primitive<const char*>(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject* py_value,
        std::function<const char*(PyObject*) > as_primitve)

{
    using dds::core::xtypes::TypeKind;

    const char* primitive_value = as_primitve(py_value);

    switch (member_info.member_kind().underlying()) {

    case TypeKind::STRING_TYPE:
        data.value<std::string>(
                member_info.member_index(),
                primitive_value);
        break;

    default:
        throw dds::core::InvalidArgumentError(
                "inconsistent input value for member id="
                + std::to_string(member_info.member_index()));
    }
}

void DynamicDataConverter::to_native_wstring(
        dds::core::xtypes::DynamicData& data,
        const rti::core::xtypes::DynamicDataMemberInfo& member_info,
        PyObject* py_value)

{
    assert(PyUnicode_Check(py_value));

     /* iterate wstring */
    DDS_Wchar *wstring_value = PyUnicode_2BYTE_DATA(py_value);
    DDS_ReturnCode_t ret_code = DDS_DynamicData_set_wstring(
            &(data.native()),
            NULL,
            member_info.member_index(),
            wstring_value);
    if (ret_code != DDS_RETCODE_OK) {
        throw dds::core::Error(
                "set_wstring: error setting wstring member id="
                + std::to_string(member_info.member_index()));
    }
}


uint32_t find_member_id_and_type(
        rti::core::xtypes::DynamicDataMemberInfo& info,
        const StructType& struct_type,
        const std::string& member_name)
{
    uint32_t index = 0;
    if (struct_type.has_parent()) {
        index = find_member_id_and_type(info, struct_type.parent(), member_name);
        if (index != dds::core::xtypes::StructType::INVALID_INDEX) {
            return info.member_index();
        }
    }

    for (int i = 0; i < struct_type.member_count(); i++) {
        ++info.native().member_id;
        if (struct_type.member(i).name() == member_name) {
            info.native().member_kind =
                    DDS_TypeCode_kind(&struct_type.member(i).type().native(), NULL);
            return info.member_index();
        }
    }

    return dds::core::xtypes::StructType::INVALID_INDEX;
}

void DynamicDataConverter::build_dynamic_data(
        dds::core::xtypes::DynamicData& data)
{
    using rti::core::xtypes::LoanedDynamicData;
    rti::core::xtypes::DynamicDataMemberInfo aux_minfo;

    assert(PyDict_Check(context_stack_.top())
            || PyList_Check(context_stack_.top()));


    /* iterate dict or list */
    PyObjectGuard top = PySequence_Fast(
            (PyObject*) context_stack_.top(),
            "");
    for (uint64_t i = 0; i < PySequence_Fast_GET_SIZE(top.get());  i++) {
        PyObject *entry = PySequence_Fast_GET_ITEM(top.get(), i);
        PyObject *value = NULL;

        aux_minfo.native().member_id = 0;
        // of top is a dict, the entry is the key
        if (PyDict_Check(context_stack_.top())) {
            value = PyDict_GetItem(context_stack_.top(), entry);
            const char *member_name = PyUnicode_AsUTF8(entry);
            if (member_name == NULL) {
                PyErr_Print();
                throw dds::core::Error(
                        "DynamicDataConverter::build_dynamic_data: key is not a member name");
            }
            const StructType struct_type =
                    static_cast<const StructType &>(data.type());
            aux_minfo.native().member_name = member_name;
            if (find_member_id_and_type(aux_minfo, struct_type, member_name)
                    == dds::core::xtypes::StructType::INVALID_INDEX) {
                std::string message =
                        "member="
                        + std::string(member_name)
                        + " not present in output type="
                        + struct_type.name();
                DDSLog_logWithFunctionName(
                        RTI_LOG_BIT_WARN,
                        "DynamicDataConverter::build_dictionary",
                        &RTI_LOG_ANY_s,
                        message.c_str());
                continue;
            }
        } else {
            // if the top is a list, the entry is the value
            value = entry;
            const CollectionType& collection_type =
                    static_cast<const CollectionType &>(data.type());
            aux_minfo.native().member_kind = DDS_TypeCode_kind(
                    &collection_type.content_type().native(),
                    NULL);
            aux_minfo.native().member_id = context_stack_.top().index  + i + 1;
        }

        if (PyDict_Check(value)) {
            LoanedDynamicData loaned_member =
                    data.loan_value(aux_minfo.member_index());
            context_stack_.push(value);
            build_dynamic_data(loaned_member);
            context_stack_.pop();
        } else if (aux_minfo.member_kind() == TypeKind::WSTRING_TYPE
                && PyBytes_Check(value)) {
            /* this is a special case because we had to treat WSTRING as any array
             * of int16 but in DynamicData this is not an array
             */
            to_native_wstring(data, aux_minfo, value);
        } else if (PyList_Check(value)) {
            bool top_is_list = PyList_Check(context_stack_.top());
            uint32_t index_offset = 0;

            /* One of the issues with the DynamicData API is that a
             * multidimensional array is accessed with a single loaned member
             * that represents a sequence of all the elements (e.g., for a
             * matrix, that would be m by n).
             *
             * Nevertheless, the Python dictionary contains the proper
             * representation (list of lists) so we need to:
             * - loan only the first array member (subsequent attempts will fail)
             * - Push down information about the offset within the loaned array
             *   that the element should be set, which is given by the total
             *   number of elements iterated from the current list (top) point
             *   of view (e.g, in a matrix, that would be the number of columns)
             */
            if (top_is_list) {
                index_offset = (PyList_GET_SIZE(value) * i);
            }
            context_stack_.push(Context(value, index_offset));

            build_dynamic_data(
                    top_is_list
                    ? data
                    : data.loan_value(aux_minfo.member_index()));

            context_stack_.pop();
        } else if (PyLong_Check(value)) {
            to_native_primitive<uint64_t>(
                    data,
                    aux_minfo,
                    value,
                    PyLong_AsUnsignedLongLong);
        } else if (PyFloat_Check(value)) {
            to_native_primitive<double>(
                    data,
                    aux_minfo,
                    value,
                    PyFloat_AsDouble);
        } else if (PyUnicode_Check(value)) {
            to_native_primitive<const char*>(
                    data,
                    aux_minfo,
                    value,
                    PyUnicode_AsUTF8);
        }

    }
}


} } }
