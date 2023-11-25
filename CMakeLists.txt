cmake_minimum_required(VERSION 3.5)

project(hil_sdk_demo)

message(STATUS "Processor name is ${CMAKE_SYSTEM_PROCESSOR}")
include_directories(
    ${CMAKE_SOURCE_DIR}/hil_sdk/include 
    ${CMAKE_SOURCE_DIR}/src/c
)
set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/hil_sdk/lib)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/release/linux)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    add_executable(${PROJECT_NAME}_init src/c/demo/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_init
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        pthread
        anl
        rt
    )

    add_executable(${PROJECT_NAME}_device_fromFile
        src/c/demo/demo_device_fromFile.cpp
    )
    target_link_libraries(${PROJECT_NAME}_device_fromFile
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_cvtColor.so  
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk_device.so  
        pthread
    )
    set_target_properties(${PROJECT_NAME}_device_fromFile PROPERTIES LINKER_LANGUAGE CXX)

    add_executable(${PROJECT_NAME}_device_fromDir
        src/c/demo/demo_device_fromDir.cpp
    )
    target_link_libraries(${PROJECT_NAME}_device_fromDir
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_cvtColor.so
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk_device.so  
        pthread
    )
    set_target_properties(${PROJECT_NAME}_device_fromDir PROPERTIES LINKER_LANGUAGE CXX)

    FILE(GLOB HIL_SDK_LIB "${CMAKE_SOURCE_DIR}/hil_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/lib*")
    install (FILES ${HIL_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
    install (TARGETS ${PROJECT_NAME}_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    install (TARGETS ${PROJECT_NAME}_device_fromFile RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    install (TARGETS ${PROJECT_NAME}_device_fromDir RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
endif()

