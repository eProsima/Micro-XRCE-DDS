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

cmake_host_system_information(RESULT HOSTNAME_SUFFIX QUERY HOSTNAME)

add_executable(client-thread-test ClientAgentInteractionThread.cpp)

add_gtest(client-thread-test
    SOURCES
        ClientAgentInteractionThread.cpp
    ENVIRONMENTS
        $<$<PLATFORM_ID:Linux>:LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib>
        $<$<PLATFORM_ID:Windows>:PATH=${CMAKE_PREFIX_PATH}/bin>
    )

target_include_directories(client-thread-test
    PRIVATE
        ${PROJECT_BINARY_DIR}/microxrcedds-build/itests
        ${GTEST_INCLUDE_DIR}
    )

target_link_libraries(client-thread-test
    PRIVATE
        interaction_thread_client
        microxrcedds_agent
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(client-thread-test PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

