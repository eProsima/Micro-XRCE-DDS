###############################################################################
#
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
#
###############################################################################

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM_NAME_LINUX ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(PLATFORM_NAME_WINDOWS ON)
endif()

set(SRCS ShapesDemo.cpp)

add_executable(shapes_demo_test ${SRCS})
add_gtest(shapes_demo_test
    SOURCES
        ${SRCS}
    ENVIRONMENTS
        $<$<PLATFORM_ID:Linux>:LD_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib>
        $<$<PLATFORM_ID:Windows>:PATH=${CMAKE_PREFIX_PATH}/bin>
    )

target_include_directories(shapes_demo_test
    PRIVATE
        ${PROJECT_BINARY_DIR}/include
        ${GTEST_INCLUDE_DIRS}
    )

target_link_libraries(shapes_demo_test
    PRIVATE
        microxrcedds_agent
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

target_compile_definitions(shapes_demo_test
    PRIVATE
        $<$<BOOL:${PLATFORM_NAME_LINUX}>:PLATFORM_NAME_LINUX>
        $<$<BOOL:${PLATFORM_NAME_WINDOWS}>:PLATFORM_NAME_WINDOWS>
    )

set_target_properties(shapes_demo_test PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )

string(REPLACE " " "' '" UTEST_SHAPESDEMO_DIR "${CLIENT_INSTALL_DIR}/examples/uxr/client/ShapeDemoClient/bin/ShapeDemoClient")
file(TO_CMAKE_PATH "${UAGENT_SOURCE_DIR}/agent.refs" UTEST_SHAPESDEMO_REFS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/shapesdemo/config.hpp
    )
