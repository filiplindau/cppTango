// **********************************************************************
//
// Generated by the ORBacus IDL-to-C++ Translator
//
// Copyright (c) 2000
// Object Oriented Concepts, Inc.
// Billerica, MA, USA
//
// All Rights Reserved
//
// **********************************************************************

// Version: 4.0.3

#include <OB/CORBAServer.h>
#include <idl/tango_skel.h>

#ifndef OB_INTEGER_VERSION
#   error No ORBacus version defined!
#endif

#ifndef OB_NO_VERSION_CHECK
#   if (OB_INTEGER_VERSION != 4000300L)
#       error ORBacus version mismatch!
#   endif
#endif

//
// IDL:Tango:1.0
//

//
// IDL:Tango/CallBack:1.0
//
CORBA::Boolean
POA_Tango::CallBack::_is_a(const char* type)
    throw(CORBA::SystemException)
{
    const char** _ob_ids = Tango::CallBack::_OB_staticIds();
    for(CORBA::ULong _ob_i = 0 ; _ob_ids[_ob_i] != 0 ; _ob_i++)
        if(strcmp(type, _ob_ids[_ob_i]) == 0)
            return true;

    return false;
}

CORBA::RepositoryId
POA_Tango::CallBack::_primary_interface(const PortableServer::ObjectId&,
                                        PortableServer::POA_ptr)
{
    return CORBA::string_dup(Tango::CallBack::_OB_staticIds()[0]);
}

Tango::CallBack_ptr
POA_Tango::CallBack::_this()
{
    CORBA::Object_var obj = _OB_createReference();
    Tango::CallBack_var result = Tango::CallBack::_narrow(obj);
    return result._retn();
}

OB::DirectStubImpl_ptr
POA_Tango::CallBack::_OB_createDirectStubImpl(PortableServer::POA_ptr poa,
                                              const PortableServer::ObjectId& oid)
{
    return new OBDirectStubImpl_Tango::CallBack(poa, oid, this);
}

void
POA_Tango::CallBack::_OB_dispatch(OB::Upcall_ptr _ob_up)
{
    _OB_dispatchBase(_ob_up);
}

//
// IDL:Tango/Device/name:1.0
//
void
POA_Tango::Device::_OB_get_name(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    CORBA::String_var _ob_r = name();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_out -> write_string(_ob_r.in());
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/description:1.0
//
void
POA_Tango::Device::_OB_get_description(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    CORBA::String_var _ob_r = description();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_out -> write_string(_ob_r.in());
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/state:1.0
//
void
POA_Tango::Device::_OB_get_state(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    Tango::DevState _ob_r = state();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_out -> write_ulong((CORBA::ULong)_ob_r);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/status:1.0
//
void
POA_Tango::Device::_OB_get_status(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    CORBA::String_var _ob_r = status();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_out -> write_string(_ob_r.in());
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/command_inout:1.0
//
void
POA_Tango::Device::_OB_op_command_inout(OB::Upcall_ptr _ob_up)
{
    OB::StrForStruct _ob_a0;
    CORBA::Any _ob_a1;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    _ob_a0 = _ob_in -> read_string();
    _ob_in -> read_any(_ob_a1);
    _OB_postUnmarshal(_ob_up);
    CORBA::Any_var _ob_r = command_inout(_ob_a0, _ob_a1);
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_out -> write_any(_ob_r.in());
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/get_attribute_config:1.0
//
void
POA_Tango::Device::_OB_op_get_attribute_config(OB::Upcall_ptr _ob_up)
{
    Tango::DevVarStringArray _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_in -> read_ulong();
    _ob_a0.length(_ob_len0);
    char* * _ob_buf0 = _ob_a0.get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        CORBA::string_free(_ob_buf0[_ob_i0]);
        _ob_buf0[_ob_i0] = 0;
        _ob_buf0[_ob_i0] = _ob_in -> read_string();
    }
    _OB_postUnmarshal(_ob_up);
    Tango::AttributeConfigList_var _ob_r = get_attribute_config(_ob_a0);
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    CORBA::ULong _ob_len1 = _ob_r.in().length();
    _ob_out -> write_ulong(_ob_len1);
    Tango::AttributeConfig const * _ob_buf1 = _ob_r.in().get_buffer();
    for(CORBA::ULong _ob_i1 = 0 ; _ob_i1 < _ob_len1 ; _ob_i1++)
    {
        _ob_buf1[_ob_i1]._OB_marshal(_ob_out);
    }
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/set_attribute_config:1.0
//
void
POA_Tango::Device::_OB_op_set_attribute_config(OB::Upcall_ptr _ob_up)
{
    Tango::AttributeConfigList _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_in -> read_ulong();
    _ob_a0.length(_ob_len0);
    Tango::AttributeConfig * _ob_buf0 = _ob_a0.get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        Tango::AttributeConfig::_OB_unmarshal(_ob_buf0[_ob_i0], _ob_in);
    }
    _OB_postUnmarshal(_ob_up);
    set_attribute_config(_ob_a0);
    _OB_preMarshal(_ob_up);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/read_attributes:1.0
//
void
POA_Tango::Device::_OB_op_read_attributes(OB::Upcall_ptr _ob_up)
{
    Tango::DevVarStringArray _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_in -> read_ulong();
    _ob_a0.length(_ob_len0);
    char* * _ob_buf0 = _ob_a0.get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        CORBA::string_free(_ob_buf0[_ob_i0]);
        _ob_buf0[_ob_i0] = 0;
        _ob_buf0[_ob_i0] = _ob_in -> read_string();
    }
    _OB_postUnmarshal(_ob_up);
    Tango::AttributeValueList_var _ob_r = read_attributes(_ob_a0);
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    CORBA::ULong _ob_len1 = _ob_r.in().length();
    _ob_out -> write_ulong(_ob_len1);
    Tango::AttributeValue const * _ob_buf1 = _ob_r.in().get_buffer();
    for(CORBA::ULong _ob_i1 = 0 ; _ob_i1 < _ob_len1 ; _ob_i1++)
    {
        _ob_buf1[_ob_i1]._OB_marshal(_ob_out);
    }
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/write_attributes:1.0
//
void
POA_Tango::Device::_OB_op_write_attributes(OB::Upcall_ptr _ob_up)
{
    Tango::AttributeValueList _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_in -> read_ulong();
    _ob_a0.length(_ob_len0);
    Tango::AttributeValue * _ob_buf0 = _ob_a0.get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        Tango::AttributeValue::_OB_unmarshal(_ob_buf0[_ob_i0], _ob_in);
    }
    _OB_postUnmarshal(_ob_up);
    write_attributes(_ob_a0);
    _OB_preMarshal(_ob_up);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/ping:1.0
//
void
POA_Tango::Device::_OB_op_ping(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    ping();
    _OB_preMarshal(_ob_up);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/black_box:1.0
//
void
POA_Tango::Device::_OB_op_black_box(OB::Upcall_ptr _ob_up)
{
    CORBA::Long _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    _ob_a0 = _ob_in -> read_long();
    _OB_postUnmarshal(_ob_up);
    Tango::DevVarStringArray_var _ob_r = black_box(_ob_a0);
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_r.in().length();
    _ob_out -> write_ulong(_ob_len0);
    const char* const *_ob_buf0 = _ob_r.in().get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        _ob_out -> write_string(_ob_buf0[_ob_i0]);
    }
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/info:1.0
//
void
POA_Tango::Device::_OB_op_info(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    Tango::DevInfo_var _ob_r = info();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_r.in()._OB_marshal(_ob_out);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/command_list_query:1.0
//
void
POA_Tango::Device::_OB_op_command_list_query(OB::Upcall_ptr _ob_up)
{
    _OB_preUnmarshal(_ob_up);
    _OB_postUnmarshal(_ob_up);
    Tango::DevCmdInfoList_var _ob_r = command_list_query();
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    CORBA::ULong _ob_len0 = _ob_r.in().length();
    _ob_out -> write_ulong(_ob_len0);
    Tango::DevCmdInfo const * _ob_buf0 = _ob_r.in().get_buffer();
    for(CORBA::ULong _ob_i0 = 0 ; _ob_i0 < _ob_len0 ; _ob_i0++)
    {
        _ob_buf0[_ob_i0]._OB_marshal(_ob_out);
    }
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device/command_query:1.0
//
void
POA_Tango::Device::_OB_op_command_query(OB::Upcall_ptr _ob_up)
{
    OB::StrForStruct _ob_a0;
    OB::InputStreamImpl* _ob_in = _OB_preUnmarshal(_ob_up);
    _ob_a0 = _ob_in -> read_string();
    _OB_postUnmarshal(_ob_up);
    Tango::DevCmdInfo_var _ob_r = command_query(_ob_a0);
    OB::OutputStreamImpl* _ob_out = _OB_preMarshal(_ob_up);
    _ob_r.in()._OB_marshal(_ob_out);
    _OB_postMarshal(_ob_up);
}

//
// IDL:Tango/Device:1.0
//
CORBA::Boolean
POA_Tango::Device::_is_a(const char* type)
    throw(CORBA::SystemException)
{
    const char** _ob_ids = Tango::Device::_OB_staticIds();
    for(CORBA::ULong _ob_i = 0 ; _ob_ids[_ob_i] != 0 ; _ob_i++)
        if(strcmp(type, _ob_ids[_ob_i]) == 0)
            return true;

    return false;
}

CORBA::RepositoryId
POA_Tango::Device::_primary_interface(const PortableServer::ObjectId&,
                                      PortableServer::POA_ptr)
{
    return CORBA::string_dup(Tango::Device::_OB_staticIds()[0]);
}

Tango::Device_ptr
POA_Tango::Device::_this()
{
    CORBA::Object_var obj = _OB_createReference();
    Tango::Device_var result = Tango::Device::_narrow(obj);
    return result._retn();
}

OB::DirectStubImpl_ptr
POA_Tango::Device::_OB_createDirectStubImpl(PortableServer::POA_ptr poa,
                                            const PortableServer::ObjectId& oid)
{
    return new OBDirectStubImpl_Tango::Device(poa, oid, this);
}

void
POA_Tango::Device::_OB_dispatch(OB::Upcall_ptr _ob_up)
{
    static const char* _ob_names[] =
    {
        "_get_description",
        "_get_name",
        "_get_state",
        "_get_status",
        "black_box",
        "command_inout",
        "command_list_query",
        "command_query",
        "get_attribute_config",
        "info",
        "ping",
        "read_attributes",
        "set_attribute_config",
        "write_attributes"
    };
    static const CORBA::ULong _ob_numNames = 14;

    switch(_OB_findOperation(_ob_up, _ob_names, _ob_numNames))
    {
    case 0: // _get_description
        _OB_get_description(_ob_up);
        return;

    case 1: // _get_name
        _OB_get_name(_ob_up);
        return;

    case 2: // _get_state
        _OB_get_state(_ob_up);
        return;

    case 3: // _get_status
        _OB_get_status(_ob_up);
        return;

    case 4: // black_box
        _OB_op_black_box(_ob_up);
        return;

    case 5: // command_inout
        _OB_op_command_inout(_ob_up);
        return;

    case 6: // command_list_query
        _OB_op_command_list_query(_ob_up);
        return;

    case 7: // command_query
        _OB_op_command_query(_ob_up);
        return;

    case 8: // get_attribute_config
        _OB_op_get_attribute_config(_ob_up);
        return;

    case 9: // info
        _OB_op_info(_ob_up);
        return;

    case 10: // ping
        _OB_op_ping(_ob_up);
        return;

    case 11: // read_attributes
        _OB_op_read_attributes(_ob_up);
        return;

    case 12: // set_attribute_config
        _OB_op_set_attribute_config(_ob_up);
        return;

    case 13: // write_attributes
        _OB_op_write_attributes(_ob_up);
        return;
    }

    _OB_dispatchBase(_ob_up);
}

//
// IDL:Tango/CallBack:1.0
//
OBDirectStubImpl_Tango::CallBack::CallBack(PortableServer::POA_ptr poa,
                                           const PortableServer::ObjectId& oid,
                                           PortableServer::ServantBase* servant)
#ifdef HAVE_VCPLUSPLUS_BUGS
{
    initialize(poa, oid, servant);
}
#else
    : OB::DirectStubImpl(poa, oid, servant)
{
}
#endif

//
// IDL:Tango/Device:1.0
//
OBDirectStubImpl_Tango::Device::Device(PortableServer::POA_ptr poa,
                                       const PortableServer::ObjectId& oid,
                                       PortableServer::ServantBase* servant)
#ifdef HAVE_VCPLUSPLUS_BUGS
{
    initialize(poa, oid, servant);
}
#else
    : OB::DirectStubImpl(poa, oid, servant)
{
}
#endif

//
// IDL:Tango/Device/name:1.0
//
char*
OBDirectStubImpl_Tango::Device::name()
{
    OB::InvocationHandler _ob_handler(this, "_get_name");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> name();
}

//
// IDL:Tango/Device/description:1.0
//
char*
OBDirectStubImpl_Tango::Device::description()
{
    OB::InvocationHandler _ob_handler(this, "_get_description");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> description();
}

//
// IDL:Tango/Device/state:1.0
//
Tango::DevState
OBDirectStubImpl_Tango::Device::state()
{
    OB::InvocationHandler _ob_handler(this, "_get_state");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> state();
}

//
// IDL:Tango/Device/status:1.0
//
char*
OBDirectStubImpl_Tango::Device::status()
{
    OB::InvocationHandler _ob_handler(this, "_get_status");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> status();
}

//
// IDL:Tango/Device/command_inout:1.0
//
CORBA::Any*
OBDirectStubImpl_Tango::Device::command_inout(const char* _ob_a0,
                                              const CORBA::Any& _ob_a1)
{
    OB::InvocationHandler _ob_handler(this, "command_inout");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> command_inout(_ob_a0, _ob_a1);
}

//
// IDL:Tango/Device/get_attribute_config:1.0
//
Tango::AttributeConfigList*
OBDirectStubImpl_Tango::Device::get_attribute_config(const Tango::DevVarStringArray& _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "get_attribute_config");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> get_attribute_config(_ob_a0);
}

//
// IDL:Tango/Device/set_attribute_config:1.0
//
void
OBDirectStubImpl_Tango::Device::set_attribute_config(const Tango::AttributeConfigList& _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "set_attribute_config");
    dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> set_attribute_config(_ob_a0);
}

//
// IDL:Tango/Device/read_attributes:1.0
//
Tango::AttributeValueList*
OBDirectStubImpl_Tango::Device::read_attributes(const Tango::DevVarStringArray& _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "read_attributes");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> read_attributes(_ob_a0);
}

//
// IDL:Tango/Device/write_attributes:1.0
//
void
OBDirectStubImpl_Tango::Device::write_attributes(const Tango::AttributeValueList& _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "write_attributes");
    dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> write_attributes(_ob_a0);
}

//
// IDL:Tango/Device/ping:1.0
//
void
OBDirectStubImpl_Tango::Device::ping()
{
    OB::InvocationHandler _ob_handler(this, "ping");
    dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> ping();
}

//
// IDL:Tango/Device/black_box:1.0
//
Tango::DevVarStringArray*
OBDirectStubImpl_Tango::Device::black_box(CORBA::Long _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "black_box");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> black_box(_ob_a0);
}

//
// IDL:Tango/Device/info:1.0
//
Tango::DevInfo*
OBDirectStubImpl_Tango::Device::info()
{
    OB::InvocationHandler _ob_handler(this, "info");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> info();
}

//
// IDL:Tango/Device/command_list_query:1.0
//
Tango::DevCmdInfoList*
OBDirectStubImpl_Tango::Device::command_list_query()
{
    OB::InvocationHandler _ob_handler(this, "command_list_query");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> command_list_query();
}

//
// IDL:Tango/Device/command_query:1.0
//
Tango::DevCmdInfo*
OBDirectStubImpl_Tango::Device::command_query(const char* _ob_a0)
{
    OB::InvocationHandler _ob_handler(this, "command_query");
    return dynamic_cast<POA_Tango::Device*>(_ob_servant_) -> command_query(_ob_a0);
}
