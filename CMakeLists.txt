#
# Breakdown <https://github.com/rodlie/breakdown>
#
# Copyright (c) 2020, Ole-André Rodlie <ole.andre.rodlie@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

cmake_minimum_required(VERSION 3.0)
project(Breakdown VERSION 1.0.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(BREAKPAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/breakpad)
set(BREAKPAD_BIN ${CMAKE_BINARY_DIR}/breakpad)
set(BREAKDOWN ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
else()
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

file(MAKE_DIRECTORY ${BREAKPAD_BIN})

find_package(PkgConfig REQUIRED)
find_package(Git QUIET)

pkg_search_module(ZIP REQUIRED libzip)

configure_file(${BREAKDOWN}/breakdown.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/breakdown.pc
               @ONLY
)

if(NOT EXISTS ${BREAKPAD_SRC}/configure)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

option(MXE32_STATIC "Build using MXE i686 static" OFF)
option(MXE64_STATIC "Build using MXE x86_64 static" OFF)
option(MXE32_SHARED "Build using MXE i686 shared" OFF)
option(MXE64_SHARED "Build using MXE x86_64 shared" OFF)

if(NOT EXISTS ${BREAKPAD_BIN}/src/libbreakpad.a)
    message("-- MISSING BREAKPAD (will try to build)")
    if(MXE32_STATIC)
        set(BREAKPAD_OPTIONS "--host=i686-w64-mingw32.static")
    elseif(MXE64_STATIC)
        set(BREAKPAD_OPTIONS "--host=x86_64-w64-mingw32.static")
    elseif(MXE32_SHARED)
        set(BREAKPAD_OPTIONS "--host=i686-w64-mingw32.shared")
    elseif(MXE64_SHARED)
        set(BREAKPAD_OPTIONS "--host=x86_64-w64-mingw32.shared")
    endif()
    execute_process(
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/.breakpad.sh ${BREAKPAD_SRC} ${BREAKPAD_OPTIONS}
        WORKING_DIRECTORY ${BREAKPAD_BIN}
    )
endif()

add_compile_options(-Wall -Wextra)
add_definitions(-D__STDC_FORMAT_MACROS=1 -Wno-format)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${BREAKDOWN}
    ${BREAKPAD_SRC}/src
    ${ZIP_INCLUDE_DIRS}
)

add_executable(stackwalker ${BREAKDOWN}/main.cpp)
target_link_libraries(stackwalker ${PROJECT_NAME})

add_library(${PROJECT_NAME}
            SHARED
            ${BREAKDOWN}/breakdown.h
            ${BREAKDOWN}/breakdown.cpp
)

set_target_properties(${PROJECT_NAME}
                      PROPERTIES
                      VERSION
                      ${PROJECT_VERSION}
)

set_target_properties(${PROJECT_NAME}
                      PROPERTIES
                      SOVERSION
                      ${PROJECT_VERSION_MAJOR}
)

target_link_libraries(${PROJECT_NAME}
                      ${BREAKPAD_BIN}/src/libbreakpad.a
                      ${BREAKPAD_BIN}/src/third_party/libdisasm/libdisasm.a
                      ${ZIP_LIBRARIES}
)

if(NOT APPLE)
    install(FILES
            LICENSE
            LICENSE.breakpad
            README.md
            DESTINATION
            ${CMAKE_INSTALL_DOCDIR}-${PROJECT_VERSION}
    )
    install(TARGETS
            stackwalker
            RUNTIME
            DESTINATION bin
    )
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/breakdown.pc
            DESTINATION
            ${LIB_INSTALL_DIR}/pkgconfig
    )
    install(TARGETS
            ${PROJECT_NAME}
            DESTINATION
            ${LIB_INSTALL_DIR}
    )
    install(FILES
            ${BREAKDOWN}/breakdown.h
            DESTINATION include/${PROJECT_NAME}
    )
endif()
