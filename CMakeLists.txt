# Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################
# CMake build rules for micro XRCE-DDS
###############################################################################

cmake_minimum_required(VERSION 3.5)

# Set CMAKE_BUILD_TYPE to Release by default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(microxrcedds VERSION "1.2.0")

set(_client_tag feature/pic)
set(_agent_tag feature/spdlog)
set(_gen_tag master)
set(_tests_tag feature/move)

###############################################################################
# Build options.
###############################################################################
option(UXRCE_BUILD_EXAMPLES "Build examples." OFF)
option(UXRCE_ENABLE_GEN "Enable the building and installation of Micro XRCE-DDS Gen." OFF)
option(UXRCE_BUILD_TESTS "Built tests." OFF)

set(_client_test_command "")
set(_agent_test_command "")
if(UXRCE_BUILD_TESTS)
    set(_client_test_command COMMAND ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_BUILD_TYPE} --no-compress-output --output-on-failure -T Test)
    set(_agent_test_command COMMAND ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_BUILD_TYPE} --no-compress-output --output-on-failure -T Test)
endif()

include(GNUInstallDirs)
set(BIN_INSTALL_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for C headers")
set(LIB_INSTALL_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR    ${CMAKE_INSTALL_DATADIR}    CACHE PATH "Installation directory for data")
if(WIN32)
    set(LICENSE_INSTALL_DIR . CACHE PATH "Installation directory for licenses")
else()
    set(LICENSE_INSTALL_DIR ${DATA_INSTALL_DIR}/${PROJECT_NAME} CACHE PATH "Installation directory for licenses")
endif()

###############################################################################
# Dependencies
###############################################################################
include(ExternalProject)

ExternalProject_Add(client
    GIT_REPOSITORY
        https://github.com/eProsima/Micro-XRCE-DDS-Client.git
    GIT_TAG
        ${_client_tag}
    PREFIX
        ${PROJECT_BINARY_DIR}/client
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/temp_install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
        -DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}
        -DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}
        -DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}
        -DDATA_INSTALL_DIR:PATH=${DATA_INSTALL_DIR}
        -DUCLIENT_BUILD_EXAMPLES:BOOL=${UXRCE_BUILD_EXAMPLES}
        -DUCLIENT_PIC:BOOL=ON
        -DUCLIENT_BUILD_TESTS:BOOL=${UXRCE_BUILD_TESTS}
        -DGTEST_INDIVIDUAL:BOOL=ON
    TEST_COMMAND
        ${_client_test_command}
    )
ExternalProject_Get_Property(client SOURCE_DIR)
set(_client_source_dir ${SOURCE_DIR})

ExternalProject_Add(agent
    GIT_REPOSITORY
        https://github.com/eProsima/Micro-XRCE-DDS-Agent.git
    GIT_TAG
        ${_agent_tag}
    PREFIX
        ${PROJECT_BINARY_DIR}/agent
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/temp_install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
        -DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}
        -DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}
        -DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}
        -DDATA_INSTALL_DIR:PATH=${DATA_INSTALL_DIR}
        -DUAGENT_BUILD_TESTS:BOOL=${UXRCE_BUILD_TESTS}
        -DGTEST_INDIVIDUAL:BOOL=ON
    TEST_COMMAND
        ${_agent_test_command}
    DEPENDS
        client
    )
ExternalProject_Get_Property(agent SOURCE_DIR)
set(_agent_source_dir ${SOURCE_DIR})

if(UXRCE_ENABLE_GEN)
    ExternalProject_Add(gen
        GIT_REPOSITORY
            https://github.com/eProsima/Micro-XRCE-DDS-Gen.git
        GIT_TAG
            ${_gen_tag}
        PREFIX
            ${PROJECT_BINARY_DIR}/gen
        INSTALL_DIR
            ${PROJECT_BINARY_DIR}/temp_install
        CONFIGURE_COMMAND
            ""
        BUILD_COMMAND
            cd <SOURCE_DIR> && gradle build
        INSTALL_COMMAND
            COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/${BIN_INSTALL_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/scripts/microxrceddsgen <INSTALL_DIR>/${BIN_INSTALL_DIR}/
            COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/scripts/microxrceddsgen.bat <INSTALL_DIR>/${BIN_INSTALL_DIR}/
            COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/share/microxrcedds
            COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/share/microxrcedds/microxrceddsgen.jar <INSTALL_DIR>/share/microxrcedds/
        )
endif()

if(UXRCE_BUILD_TESTS)
    ExternalProject_Add(itests
        GIT_REPOSITORY
            https://github.com/eProsima/Micro-XRCE-DDS-Integration-Tests.git
        GIT_TAG
            ${_tests_tag}
        PREFIX
            ${PROJECT_BINARY_DIR}/itests
        INSTALL_COMMAND
            ""
        TEST_COMMAND
            COMMAND ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_BUILD_TYPE} --no-compress-output --output-on-failure -T Test
        CMAKE_CACHE_ARGS
            -DCMAKE_PREFIX_PATH:PATH=${PROJECT_BINARY_DIR}/temp_install
            -DGTEST_INDIVIDUAL:BOOL=ON
            -DUCLIENT_SOURCE_DIR:PATH=${_client_source_dir}
            -DUAGENT_SOURCE_DIR:PATH=${_agent_source_dir}
        DEPENDS
            client
            agent
        )
endif()

###############################################################################
# Install
###############################################################################
if(EXISTS ${CMAKE_BINARY_DIR}/temp_install/)
    install(
        DIRECTORY
            ${CMAKE_BINARY_DIR}/temp_install/
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
        USE_SOURCE_PERMISSIONS
        )
endif()
