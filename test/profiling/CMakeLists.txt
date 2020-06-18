# Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.5)

project(microxrcedds-profiling)

ExternalProject_Add(clientlib
    GIT_REPOSITORY
        https://github.com/eProsima/Micro-XRCE-DDS-Client.git
    GIT_TAG
        ${_client_tag}
    PREFIX
        ${PROJECT_BINARY_DIR}/client
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    )

ExternalProject_Add(agentlib
    GIT_REPOSITORY
        https://github.com/eProsima/Micro-XRCE-DDS-Agent.git
    GIT_TAG
        ${_agent_tag}
    PREFIX
        ${PROJECT_BINARY_DIR}/agent
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/install
    CMAKE_CACHE_ARGS
        -DBUILD_SHARED_LIBS:BOOL=FALSE
        -DCMAKE_BUILD_TYPE:STRING=MinSizeRel
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DUAGENT_FAST_PROFILE:BOOL=FALSE
        -DUAGENT_DISCOVERY_PROFILE:BOOL=FALSE
        -DUAGENT_P2P_PROFILE:BOOL=FALSE
        -DUAGENT_LOGGER_PROFILE:BOOL=FALSE
        -DUAGENT_SERVER_BUFFER_SIZE:STRING=1024
    )

ExternalProject_Add(publisher
    SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/publisher
    PREFIX
        ${PROJECT_BINARY_DIR}/publisher
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH:PATH=${PROJECT_BINARY_DIR}/install
    DEPENDS
        clientlib
    )

ExternalProject_Add(subscriber
    SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/subscriber
    PREFIX
        ${PROJECT_BINARY_DIR}/subscriber
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH:PATH=${PROJECT_BINARY_DIR}/install
    DEPENDS
        clientlib
    )

ExternalProject_Add(agent
    SOURCE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/agent
    PREFIX
        ${PROJECT_BINARY_DIR}/agent
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/install
    CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH:PATH=<INSTALL_DIR>
    DEPENDS
        agentlib
    )


file(COPY ${CMAKE_CURRENT_LIST_DIR}/profiling.py DESTINATION ${PROJECT_BINARY_DIR})
