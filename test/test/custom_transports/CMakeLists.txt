# Copyright 2021-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

# include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)

# cmake_host_system_information(RESULT HOSTNAME_SUFFIX QUERY HOSTNAME)

# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../common/EntitiesInfo.hpp.in
#     ${CMAKE_CURRENT_BINARY_DIR}/EntitiesInfo.hpp
#     @ONLY
#     )

set(SRCS
    Custom_transports.cpp
    )

add_library(custom_transports STATIC ${SRCS})

set_common_compile_options(custom_transports)

if(MSVC OR MSVC_IDE)
    target_compile_options(custom_transports
        PRIVATE
            /wd4996
        PUBLIC
            -D_CRT_SECURE_NO_WARNINGS
        )
endif()

target_include_directories(custom_transports
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE
        ${GTEST_INCLUDE_DIRS}
    )

target_link_libraries(custom_transports
    PUBLIC
        microxrcedds_client
        microxrcedds_agent
    PRIVATE
        ${GTEST_BOTH_LIBRARIES}
    )

set_target_properties(custom_transports PROPERTIES
    CXX_STANDARD
        11
    CXX_STANDARD_REQUIRED
        YES
    )
