# Copyright 2018 inkeun.kim
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


cmake_minimum_required(VERSION 3.0.2)

project(AsioApp C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost 1.58.0 REQUIRED COMPONENTS system)

set(HEADER
  ${CMAKE_SOURCE_DIR}/include/app/common/common.h
  ${CMAKE_SOURCE_DIR}/include/app/common/uuid.h
  ${CMAKE_SOURCE_DIR}/include/app/network/internal/asio_server.h
)

set(CC
  ${CMAKE_SOURCE_DIR}/sources/common/uuid.cc
  ${CMAKE_SOURCE_DIR}/sources/network/asio_server.cc
)

set(SOURCES ${HEADER} ${CC})
add_executable(app ${HEADER} ${CMAKE_SOURCE_DIR}/sources/main.cc ${SOURCES})

target_link_libraries(app PUBLIC
 ${Boost_LIBRARIES}
)
