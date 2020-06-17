
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

project(publisher-profiling)

find_package(microxrcedds_client)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        microxrcedds_client
    )

install(
    TARGETS
        ${PROJECT_NAME}
    RUNTIME DESTINATION
        bin
    )