#include "Python.h"

#include "dds_c/dds_c_string.h"
#include "NativeUtils.hpp"

namespace rti { namespace routing { namespace py {


/*
 * --- From Native ------------------------------------------------------------
 */

#define PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(DATA, MEMBER) \
{\
    PyObjectGuard py_guard = from_native((DATA).MEMBER); \
    if (PyDict_SetItemString( \
            py_dict.get(), \
            #MEMBER, \
            py_guard.get()) == -1) {\
        PyErr_Print();\
        throw dds::core::Error("from_native: error setting member=" #MEMBER);\
    }\
}

PyObject* from_native(
        DDS_Boolean boolean_value)
{
    return PyLong_FromLong(boolean_value ? 1 : 0);
}

PyObject* from_native(
        DDS_Long long_value)
{
    return PyLong_FromLong(long_value);
}

PyObject* from_native(
        const RTICdrOctet *byte_array,
        int32_t size)
{
    PyObject* py_bytes = PyBytes_FromStringAndSize(
            (const char *) byte_array,
            size);
    if (py_bytes == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating byte array");
    }

    return py_bytes;
}

PyObject* from_native(
        const struct RTI_RoutingServiceProperties *properties)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("error creating Python dictionary");
    }

    for (int i = 0; i < properties->count; i++) {
        PyObjectGuard value = Py_BuildValue("s", properties->properties[i].value);
        if (value.get() == NULL) {
            PyErr_Print();
            throw dds::core::Error("error creating string value");
        }

        if (PyDict_SetItemString(
                py_dict.get(),
                properties->properties[i].name,
                value.get()) == -1) {
            PyErr_Print();
            throw dds::core::Error("error inserting property element into dictionary");
        }
    }

    return py_dict.release();
}

PyObject* from_native(const DDS_InstanceHandle_t& handle)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    PyObjectGuard py_value = PyLong_FromLong(handle.isValid);
    if (PyDict_SetItemString(
            py_dict.get(),
            "valid",
            py_value.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting valid element");
    }

    PyObjectGuard py_bytes = from_native(
            handle.keyHash.value,
            handle.keyHash.length);
    if (py_bytes.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "key_hash",
            py_bytes.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting key_hash element");
    }

    return py_dict.release();
}

PyObject* from_native(const DDS_SequenceNumber_t& sn)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    PyObjectGuard py_low = PyLong_FromUnsignedLong(sn.low);
    if (PyDict_SetItemString(
            py_dict.get(),
            "low",
            py_low.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'low' element");
    }
    PyObjectGuard py_high = PyLong_FromLong(sn.high);
    if (PyDict_SetItemString(
            py_dict.get(),
            "high",
            py_high.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'high' element");
    }

    return py_dict.release();
}


PyObject* from_native(const DDS_GUID_t& guid)
{
    return from_native(guid.value, DDS_GUID_LENGTH);
}

PyObject* from_native(
        const DDS_SampleIdentity_t& identity)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    PyObjectGuard py_list = from_native(identity.writer_guid);
    if (py_list.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python list");
    }
    if (PyDict_SetItemString(
            py_dict.get(),
            "writer_guid",
            py_list.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'writer_guid' element");
    }

    PyObjectGuard py_sn = from_native(identity.sequence_number);
    if (PyDict_SetItemString(
            py_dict.get(),
            "sequence_number",
            py_sn.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'sequence_number' element");
    }

    return py_dict.release();
}

PyObject* from_native(DDS_SampleStateKind state)
{
    return PyLong_FromLong(state);
}

PyObject* from_native(DDS_ViewStateKind state)
{
    return PyLong_FromLong(state);
}


PyObject* from_native(DDS_InstanceStateMask state)
{
    return PyLong_FromLong(state);
}

PyObject* from_native(const DDS_Time_t& time)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    PyObjectGuard py_sec = PyLong_FromLong(time.sec);
    if (PyDict_SetItemString(
            py_dict.get(),
            "low",
            py_sec.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'sec' element");
    }
    PyObjectGuard py_nanos = PyLong_FromUnsignedLong(time.nanosec);
    if (PyDict_SetItemString(
            py_dict.get(),
            "high",
            py_nanos.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error("from_native: error setting 'nanosec' element");
    }

    return py_dict.release();
}

PyObject* from_native(const RTI_RoutingServiceStreamInfo& info)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }
    RTI_PY_ADD_DICT_ITEM_MEMBER(
            py_dict.get(),
            info,
            stream_name,
            PyUnicode_FromString);
    RTI_PY_ADD_DICT_ITEM_MEMBER(
            py_dict.get(),
            info.type_info,
            type_name,
            PyUnicode_FromString);

    return py_dict.release();
}


/*
 * --- To Native -------------------------------------------------------------
 */

#define PY_NATIVE_UTILS_TO_NATIVE_MEMBER(DATA, MEMBER, TYPE) \
{\
    PyObject *py_item = PyDict_GetItemString(py_dict, #MEMBER); \
    if (py_item != NULL) { \
        if (!Py ## TYPE ## _Check(py_item)) {\
            throw dds::core::Error("to_native: unexpected type of member="#MEMBER);\
        }\
        to_native((DATA).MEMBER, py_item); \
    }\
}


DDS_Long to_native(
        DDS_Long& dest,
        PyObject* py_value)
{
    if (!PyLong_Check(py_value)) {
        throw dds::core::Error("to_native: object is not a Long");
    }

    dest = (DDS_Long) PyLong_AsLong(py_value);
    return dest;
}

DDS_UnsignedLong to_native(
        DDS_UnsignedLong& dest,
        PyObject* py_value)
{
    if (!PyLong_Check(py_value)) {
        throw dds::core::Error("to_native: object is not a Long");
    }

    dest = (DDS_UnsignedLong) PyLong_AsLong(py_value);
    return dest;
}

DDS_Boolean to_native(
        DDS_Boolean& dest,
        PyObject* py_value)
{
    if (!PyLong_Check(py_value)) {
        throw dds::core::Error("to_native: object is not a long");
    }

    dest = PyLong_AsLong(py_value) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
    return dest;
}

DDS_SequenceNumber_t to_native(
        DDS_SequenceNumber_t& dest,
        PyObject* py_dict)
{
    if (!PyDict_Check(py_dict)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(dest, low, Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(dest, high, Long);

    return dest;
}

DDS_Time_t to_native(
        DDS_Time_t& dest,
        PyObject* py_dict)
{
    if (!PyDict_Check(py_dict)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(dest, sec, Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(dest, nanosec, Long);

    return dest;
}

DDS_InstanceHandle_t& to_native(DDS_InstanceHandle_t& dest, PyObject* py_handle)
{
    if (!PyDict_Check(py_handle)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    PyObject *py_valid = PyDict_GetItemString(py_handle, "valid");
    if (py_valid == NULL) {
        throw dds::core::Error("to_native: member=valid not found");
    } else if (!PyLong_Check(py_valid)) {
        throw dds::core::Error("to_native: member=valid is not an integer");
    }
    dest.isValid = (int32_t) PyLong_AsLong(py_valid);

    PyObject *py_key_hash = PyDict_GetItemString(py_handle, "key_hash");
    if (py_key_hash == NULL) {
        throw dds::core::Error("to_native: member=key_hash not found");
    } else if (!PyBytes_Check(py_key_hash)) {
        throw dds::core::Error("to_native: member=key_hash is not a list");
    } else if (PyBytes_GET_SIZE(py_key_hash) != MIG_RTPS_KEY_HASH_MAX_LENGTH) {
         throw dds::core::Error(
                 "to_native: member=key_hash list must have size="
                 + std::to_string(MIG_RTPS_KEY_HASH_MAX_LENGTH));
    }

    memcpy(
            dest.keyHash.value,
            PyBytes_AsString(py_key_hash),
            MIG_RTPS_KEY_HASH_MAX_LENGTH);

    return dest;
}


DDS_SampleStateKind to_native(
        DDS_SampleStateKind& dest,
        PyObject* py_state)
{
    if (!PyLong_Check(py_state)) {
        throw dds::core::Error("to_native: object is not a long");
    }

    dest = (DDS_SampleStateKind) PyLong_AsUnsignedLong(py_state);
    return dest;
}

DDS_ViewStateKind to_native(
        DDS_ViewStateKind& dest,
        PyObject* py_state)
{
    if (!PyLong_Check(py_state)) {
        throw dds::core::Error("to_native: object is not a long");
    }

    dest = (DDS_ViewStateKind) PyLong_AsUnsignedLong(py_state);
    return dest;
}

DDS_InstanceStateKind to_native(
        DDS_InstanceStateKind& dest,
        PyObject* py_state)
{
    if (!PyLong_Check(py_state)) {
        throw dds::core::Error("to_native: object is not a long");
    }

    dest = (DDS_InstanceStateKind) PyLong_AsUnsignedLong(py_state);
    return dest;
}


DDS_GUID_t& to_native(
        DDS_GUID_t& dest,
        PyObject* py_guid)
{
    if (!PyBytes_Check(py_guid)) {
        throw dds::core::Error("to_native: object is not a byte array");
    } else if (PyBytes_GET_SIZE(py_guid) != MIG_RTPS_KEY_HASH_MAX_LENGTH) {
        throw dds::core::Error("to_native: invalid array size="
                + std::to_string(PyBytes_GET_SIZE(py_guid))
                + ". Expected size="
                + std::to_string(MIG_RTPS_KEY_HASH_MAX_LENGTH));
    }

    memcpy(
            dest.value,
            PyBytes_AsString(py_guid),
            MIG_RTPS_KEY_HASH_MAX_LENGTH);

    return dest;
}

char* to_native(char*& dest, PyObject* py_name)
{
    if (!PyUnicode_Check(py_name)) {
        throw dds::core::Error("to_native: object is not a long");
    }

    DDS_String_replace(&dest, PyUnicode_AsUTF8(py_name));

    return dest;
}


RTI_RoutingServiceProperties& to_native(
        struct RTI_RoutingServiceProperties& dest,
        PyObject *py_dict)
{
    if (!PyDict_Check(py_dict)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }


    PyObjectGuard keys = PyDict_Keys(py_dict);
    if (!RTIOsapiHeap_allocateArray(
            &dest.properties,
            PyList_Size(keys.get()),
            RTI_RoutingServiceNameValue)) {
        throw dds::core::Error("to_native: error allocating name-value array");
    }
    dest.count = PyList_Size(keys.get());

    for (int32_t i = 0; i <  dest.count; i++) {
        PyObject *name = PyList_GetItem(keys.get(), i);
        if (PyUnicode_Check(name)) {
            throw dds::core::Error("to_native: keys are not string");
        }
        PyObject *value = PyDict_GetItem(py_dict, name);
        if (PyUnicode_Check(value)) {
            throw dds::core::Error("to_native: values are not string");
        }

        dest.properties[i].name = DDS_String_dup(PyUnicode_AsUTF8(name));
        dest.properties[i].value = DDS_String_dup(PyUnicode_AsUTF8(value));
    }

    return dest;
}

/*
 * --- PySampleInfoConverter --------------------------------------------------
 */

PyObject* from_native(const DDS_SampleInfo& info)
{
    PyObjectGuard py_dict = PyDict_New();
    if (py_dict.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error("from_native: error creating Python dictionary");
    }

    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, instance_handle);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, publication_handle);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, sample_state);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, view_state);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, instance_state);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, valid_data);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, flag);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, original_publication_virtual_sequence_number);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, original_publication_virtual_guid);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, related_original_publication_virtual_sequence_number);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, related_original_publication_virtual_guid);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, reception_sequence_number);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, publication_sequence_number);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, reception_timestamp);
    PY_NATIVE_UTILS_FROM_NATIVE_MEMBER(info, source_timestamp);

    return py_dict.release();
}

DDS_SampleInfo& to_native(
        DDS_SampleInfo& info,
        PyObject* py_dict)
{
    if (!PyDict_Check(py_dict)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            instance_handle,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            sample_state,
            Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            view_state,
            Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            instance_state,
            Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            valid_data,
            Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            flag,
            Long);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            original_publication_virtual_sequence_number,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            original_publication_virtual_guid,
            Bytes);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            related_original_publication_virtual_sequence_number,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            related_original_publication_virtual_guid,
            Bytes);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            reception_sequence_number,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            publication_sequence_number,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            reception_timestamp,
            Dict);
    PY_NATIVE_UTILS_TO_NATIVE_MEMBER(
            info,
            source_timestamp,
            Dict);

    return info;
}


} } }

