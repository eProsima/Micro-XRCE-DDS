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

project(microxrcedds)

###############################################################################
# Config
###############################################################################
# Install path
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

option(UXRCE_BUILD_EXAMPLES "Build examples." OFF)

###############################################################################
# Projects
###############################################################################
include(ExternalProject)

ExternalProject_Add(client
    DOWNLOAD_COMMAND
        cd ${PROJECT_SOURCE_DIR} && git submodule update --init client
    PREFIX
        ${PROJECT_BINARY_DIR}/uclient
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}/client
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/temp_install
    CMAKE_ARGS
        -DUCLIENT_SUPERBUILD:BOOL=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}
        -DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}
        -DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}
        -DDATA_INSTALL_DIR:PATH=${DATA_INSTALL_DIR}
        -DUCLIENT_BUILD_EXAMPLES:BOOL=${UXRCE_BUILD_EXAMPLES}
    )

ExternalProject_Add(agent
    DOWNLOAD_COMMAND
        cd ${PROJECT_SOURCE_DIR} && git submodule update --init agent
    PREFIX
        ${PROJECT_BINARY_DIR}/uagent
    SOURCE_DIR
        ${PROJECT_SOURCE_DIR}/agent
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/temp_install
    CMAKE_ARGS
        -DUCLIENT_SUPERBUILD:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBIN_INSTALL_DIR:PATH=${BIN_INSTALL_DIR}
        -DINCLUDE_INSTALL_DIR:PATH=${INCLUDE_INSTALL_DIR}
        -DLIB_INSTALL_DIR:PATH=${LIB_INSTALL_DIR}
        -DDATA_INSTALL_DIR:PATH=${DATA_INSTALL_DIR}
        -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
    DEPENDS
        client
    )

# Install dependencies.
if(EXISTS ${CMAKE_BINARY_DIR}/temp_install/)
    install(
        DIRECTORY
            ${CMAKE_BINARY_DIR}/temp_install/
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
        )
endif()
