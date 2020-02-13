# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

macro(eprosima_clone_package package)

    if(NOT ${package}_FOUND)

        # Parse arguments.
        set(options REQUIRED)
        set(uniValueArgs BRANCH REPO_URL OPTIONS)
        cmake_parse_arguments(FIND "${options}" "${uniValueArgs}" "" ${ARGN})

        if(EXISTS "${PROJECT_BINARY_DIR}/${package}_clone" AND ${PACKAGE_UPDATE})
            file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/${package}_clone")
        endif()

        if(NOT EXISTS "${PROJECT_BINARY_DIR}/${package}_clone")
            message(STATUS "${package} is being cloning (${FIND_REPO_URL} --- ${FIND_BRANCH}) ...")
            execute_process(
                COMMAND git clone "${FIND_REPO_URL}" "${package}_clone" --branch ${FIND_BRANCH}
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                RESULT_VARIABLE EXECUTE_RESULT
                )
            if(EXECUTE_RESULT EQUAL 0)
                foreach(opt_ ${FIND_OPTIONS})
                    set(${opt_} ON)
                endforeach()
                add_subdirectory(${PROJECT_BINARY_DIR}/${package}_clone)
                set(${package}_FOUND TRUE)
                if(NOT IS_TOP_LEVEL)
                    set(${package}_FOUND TRUE PARENT_SCOPE)
                endif()
            else()
                message(WARNING "Cannot clone ${package}")
            endif()
        else()
            add_subdirectory(${PROJECT_BINARY_DIR}/${package}_clone)
            set(${package}_FOUND TRUE)
            if(NOT IS_TOP_LEVEL)
                set(${package}_FOUND TRUE PARENT_SCOPE)
            endif()
        endif()

        if(${package}_FOUND)
            message(STATUS "${package} library found...")
        elseif(${FIND_REQUIRED})
            message(FATAL_ERROR "${package} library not found...")
        else()
            message(STATUS "${package} library not found...")
        endif()
    endif()
endmacro()
