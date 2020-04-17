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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DEFAULT_FASTRTPS_PROFILES.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/DEFAULT_FASTRTPS_PROFILES.xml
    @ONLY
    )

add_executable(publisher-subscriber-interaction-test PublisherSubscriberInteraction.cpp)

add_gtest(publisher-subscriber-interaction-test
    SOURCES
        PublisherSubscriberInteraction.cpp
    ENVIRONMENTS
        $<$<PLATFORM_ID:Linux>:LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib>
        $<$<PLATFORM_ID:Windows>:PATH=${CMAKE_PREFIX_PATH}/bin>
    )

target_include_directories(publisher-subscriber-interaction-test
    PRIVATE
        ${GTEST_INCLUDE_DIR}
    )

target_link_libraries(publisher-subscriber-interaction-test
    PRIVATE
        interaction_client
        microxrcedds_agent
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(publisher-subscriber-interaction-test PROPERTIES
        CXX_STANDARD
            11
        CXX_STANDARD_REQUIRED
            YES
    )

