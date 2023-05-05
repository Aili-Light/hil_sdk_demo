cmake_minimum_required(VERSION 3.5)

project(hil_sdk_demo)

message(STATUS "Processor name is ${CMAKE_SYSTEM_PROCESSOR}")
include_directories(
    include
    ${CMAKE_SOURCE_DIR}/hil_sdk/include 
    ${CMAKE_SOURCE_DIR}/src/c
)
set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/hil_sdk/lib)
aux_source_directory(src/c/utils UTILS_SRC_LIST)

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
    )

    add_executable(${PROJECT_NAME}_push_1_ch 
        ${UTILS_SRC_LIST}
        src/c/demo/demo_push_1_ch.cpp
    )
    target_link_libraries(${PROJECT_NAME}_push_1_ch
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_push_multi_ch
        ${UTILS_SRC_LIST}
        src/c/demo/demo_push_multi_ch.cpp
    )
    target_link_libraries(${PROJECT_NAME}_push_multi_ch
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_push_v4l2
        ${UTILS_SRC_LIST}
        src/c/demo/demo_push_v4l2.cpp
        src/c/alg_camera/v4l2_camera.cpp
    )
    target_link_libraries(${PROJECT_NAME}_push_v4l2
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
        pthread
        anl
    )

    include_directories(${CMAKE_SOURCE_DIR}/qcap_lib
    )

    link_directories(
        ${CMAKE_SOURCE_DIR}/qcap_lib/lib/
    )

    if (WITH_QCAP)
        add_executable(${PROJECT_NAME}_push_qcap
            ${UTILS_SRC_LIST}
            src/c/demo/demo_push_qcap.cpp
            src/c/qcap/qcapdev.cpp
        )
        target_link_libraries(${PROJECT_NAME}_push_qcap
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk.so
            pthread
            anl
            -lqcap.x64
        )

        install (TARGETS ${PROJECT_NAME}_push_qcap RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()

    FILE(GLOB HIL_SDK_LIB "${CMAKE_SOURCE_DIR}/hil_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libhil_sdk*")
    install (FILES ${HIL_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
endif()

install (TARGETS ${PROJECT_NAME}_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_push_1_ch RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_push_multi_ch RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_push_v4l2 RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
