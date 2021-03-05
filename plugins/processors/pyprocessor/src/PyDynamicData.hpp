#ifndef RTI_ROUTING_PY_DICTPRINTFORMAT_HPP_
#define RTI_ROUTING_PY_DICTPRINTFORMAT_HPP_

#include "Python.h"

#include <stack>

#include "dds/core/xtypes/DynamicData.hpp"
#include "dds/core/xtypes/DynamicType.hpp"
#include "dds/core/xtypes/TypeKind.hpp"
#include "NativeUtils.hpp"

namespace rti { namespace routing { namespace py {


class DynamicDataConverter {

public:
    static PyObject * from_dynamic_data(
            const dds::core::xtypes::DynamicData& data);

    static void to_dynamic_data(
            dds::core::xtypes::DynamicData& data,
            PyObject *py_dict);

   static const int64_t BYTES_PER_WCHAR = sizeof (DDS_Wchar) / sizeof (DDS_Char);

private:
    DynamicDataConverter(const dds::core::xtypes::DynamicData& data);
    DynamicDataConverter(PyObject *py_dict, dds::core::xtypes::DynamicData& data);

    class Context {
    public:

        Context(PyObject *object, uint64_t the_index= 0)
                : current(object),
                index(the_index)
        {
        }

        operator PyObject*() const
        {
            return current;


        }

    public:
        PyObject *current;
        uint64_t index;
    };


private:

    template <typename T, typename U>
    void from_native_primitive(
            const dds::core::xtypes::DynamicData &data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info,
            std::function<PyObject*(U)> to_python_object)
    {
        T value = data.value<T>(member_info.member_index());
        PyObjectGuard py_value = to_python_object(static_cast<U> (value));
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

    void from_wstring(
            const dds::core::xtypes::DynamicData &data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info);

    void build_dictionary(
            dds::core::xtypes::DynamicData& data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info);
    void build_dynamic_data(
            dds::core::xtypes::DynamicData& data);

    template <typename T> static
    void to_native_primitive(
            dds::core::xtypes::DynamicData& data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info,
            PyObject *py_value,
            std::function<T(PyObject*) > as_primitve);

    static void to_native_wstring(
            dds::core::xtypes::DynamicData& data,
            const rti::core::xtypes::DynamicDataMemberInfo& member_info,
            PyObject* py_value);

private:
    std::stack<Context> context_stack_;

};

} } }

#endif
