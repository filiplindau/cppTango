set(SOURCES dbapi_class.cpp
            dbapi_server.cpp
            dbapi_datum.cpp
            dbapi_base.cpp
            dbapi_device.cpp
            dbapi_history.cpp
            dbapi_attribute.cpp
            dbapi_cache.cpp
            dbapi_serverdata.cpp
            devapi_attr.cpp
            devapi_base.cpp
            devapi_data.cpp
            devapi_datahist.cpp
            devapi_utils.cpp
            devapi_pipe.cpp
            api_util.cpp
            asynreq.cpp
            cbthread.cpp
            proxy_asyn.cpp
            proxy_asyn_cb.cpp
            attr_proxy.cpp
            group.cpp
            filedatabase.cpp
            apiexcept.cpp
            accessproxy.cpp
            lockthread.cpp
            event.cpp
            eventkeepalive.cpp
            eventqueue.cpp
            notifdeventconsumer.cpp
            zmqeventconsumer.cpp)

set(HEADERS accessproxy.h
            apiexcept.h
            cbthread.h
            dbapi.h
            devapi.h
            devasyn.h
            event.h
            eventconsumer.h
            filedatabase.h
            group.h
            lockthread.h
            Database.h
            DbDevice.h
            ApiUtil.h
            DeviceData.h
            DeviceAttribute.h
            DevicePipe.h
            Connection.h
            DeviceProxy.h
            AttributeProxy.h
            event.tpp
            devapi_attr.tpp
            devapi_utils.tpp
            api_util.tpp
            devapi_pipe.tpp)

add_subdirectory(helpers)
if(WIN32)
    add_library(client_objects_dyn OBJECT ${SOURCES})
    add_library(client_objects_sta OBJECT ${SOURCES})
    target_compile_definitions(client_objects_dyn PRIVATE "${dyn_defs}")
    target_compile_definitions(client_objects_sta PRIVATE "${static_defs}")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(client_objects_sta PRIVATE "/MTd")
    else()
        target_compile_options(client_objects_sta PRIVATE "/MT")
    endif()
else(WIN32)
    add_library(client_objects OBJECT ${SOURCES})
    target_compile_options(client_objects PRIVATE -fPIC)
endif(WIN32)
install(FILES ${HEADERS} DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
