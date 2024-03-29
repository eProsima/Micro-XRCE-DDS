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

add_executable(discovery-test DiscoveryIntegration.cpp)

gtest_add_tests(
    TARGET
        discovery-test
    SOURCES
        DiscoveryIntegration.cpp
    TEST_LIST
        allDiscoveryTest
    )

if(WIN32)
    set(PATH_ENV "$ENV{PATH}")
    string(REPLACE ";" "\\;" PATH_ENV "${PATH_ENV}")

    set_tests_properties(${allDiscoveryTest} PROPERTIES ENVIRONMENT
        "PATH=${CMAKE_PREFIX_PATH}/bin\\;${PATH_ENV}")
else()
    set_tests_properties(${allDiscoveryTest} PROPERTIES ENVIRONMENT
        "LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib")
endif()

target_include_directories(discovery-test
    PRIVATE
        ${GTEST_INCLUDE_DIR}
    )

target_link_libraries(discovery-test
    PRIVATE
        interaction_client
        microxrcedds_agent
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

set_target_properties(discovery-test PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

