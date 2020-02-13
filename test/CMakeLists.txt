# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_host_system_information(RESULT HOSTNAME_SUFFIX QUERY HOSTNAME)

project(microxrcedds_itests LANGUAGES C CXX)

set(UCLIENT_SOURCE_DIR . CACHE PATH "Client source dir.")
set(UAGENT_SOURCE_DIR . CACHE PATH "Agent source dir.")

find_package(microcdr REQUIRED)
find_package(microxrcedds_client REQUIRED)

find_package(fastcdr REQUIRED)
find_package(microxrcedds_agent REQUIRED)

###############################################################################
# Testing
###############################################################################
include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

enable_testing()
include(CTest)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(test/cross_serialization)
endif()

add_subdirectory(test/interaction_client)
add_subdirectory(test/client_agent)
add_subdirectory(test/publisher_subscriber)
add_subdirectory(test/discovery)
#add_subdirectory(test/shapes_demo) TODO (julianbermudez): fix client and agent paths.
