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

include(${PROJECT_SOURCE_DIR}/cmake/common/check_configuration.cmake)

cmake_host_system_information(RESULT HOSTNAME_SUFFIX QUERY HOSTNAME)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../common/EntitiesInfo.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/EntitiesInfo.hpp
    @ONLY
    )

set(SRCS
    Client.cpp
    Gateway.cpp
    BigHelloWorld.c
    Discovery.cpp
    )

list(APPEND libs interaction_client)
find_library(CLIENT microxrcedds_client REQUIRED 
    HINTS ${PROJECT_BINARY_DIR}/../../../temp_install/lib/libmicroxrcedds_client.a)

if(UXRCE_TEST_THREAD)
    list(APPEND libs interaction_thread_client)
    find_library(CLIENT_MULTITHREAD thread_microxrcedds_client REQUIRED 
    HINTS ${PROJECT_BINARY_DIR}/../../../temp_install/lib/libthread_microxrcedds_client.a)
endif()

foreach(lib ${libs})
    add_library(${lib} STATIC ${SRCS})
    set_common_compile_options(${lib})

    if(MSVC OR MSVC_IDE)
        target_compile_options(${lib}
            PRIVATE
                /wd4996
            PUBLIC
                -D_CRT_SECURE_NO_WARNINGS
            )
    endif()

    if(${lib} STREQUAL interaction_client)
        target_include_directories(${lib}
        PUBLIC
            ${PROJECT_BINARY_DIR}/../../../temp_install/include
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE
            ${GTEST_INCLUDE_DIRS}
        )

        target_link_libraries(${lib}
        PUBLIC
            ${CLIENT}
            custom_transports
        PRIVATE
            ${GTEST_BOTH_LIBRARIES}
        )
    elseif(${lib} STREQUAL interaction_thread_client)
        target_include_directories(${lib}
            PUBLIC
                ${PROJECT_BINARY_DIR}/../../../temp_install/include_thread
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}
            PRIVATE
                ${GTEST_INCLUDE_DIRS}
            )

        target_link_libraries(${lib}
        PUBLIC
            ${CLIENT_MULTITHREAD}
            custom_transports
        PRIVATE
            ${GTEST_BOTH_LIBRARIES}
        )
    endif()

    set_target_properties(${lib} PROPERTIES
        CXX_STANDARD
            11
        CXX_STANDARD_REQUIRED
            YES
        )
endforeach()