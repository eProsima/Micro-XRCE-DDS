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

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(microxrcedds_ci LANGUAGES C CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "Unsupported compiler.")
endif()

include(ExternalProject)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

set(_c_flags "-fwrapv -fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
set(_cxx_flags "-fwrapv -fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
set(_exe_linker_flags "-fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-inline-small-functions -fno-default-inline")
set(_shared_linker_flags "-fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-inline-small-functions -fno-default-inline")

check_cxx_compiler_flag("-fprofile-abs-path" _have_fprofile_abs_path)
if(_have_fprofile_abs_path)
    set(_cxx_flags "${_cxx_flags} -fprofile-abs-path")
endif()

ExternalProject_Add(microxrcedds
    SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../
    BINARY_DIR
        ${PROJECT_BINARY_DIR}/microxrcedds-build
    INSTALL_DIR
        ${PROJECT_BINARY_DIR}/temp_install
    CMAKE_CACHE_ARGS
        -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_FLAGS:STRING=${_c_flags}
        -DCMAKE_CXX_FLAGS:STRING=${_cxx_flags}
        -DCMAKE_EXE_LINKER_FLAGS:STRING=${_exe_linker_flags}
        -DCMAKE_SHARED_LINKER_FLAGS:STRING=${_shared_linker_flags}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
        -DUXRCE_BUILD_CI_TESTS:BOOL=ON
        -DUXRCE_ENABLE_GEN:BOOL=OFF # TODO (juliber): move test away from Gradle.
        -DGTEST_INDIVIDUAL:BOOL=ON
    )
