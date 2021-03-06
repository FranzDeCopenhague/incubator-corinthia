#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.



###
## global definitions
###
cmake_minimum_required(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



###
## group source objects
###
set(SOURCES
    main.c)



###
# Common include for all platform files
###
include_directories()
include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(SYSTEM ../../../DocFormats/api/headers)
include_directories(SYSTEM ../../../DocFormats/headers)
include_directories(.)
link_directories(${LIB_DIRS})



###
# executable (release artifact)
###
add_executable(dfconvert ${SOURCES})
target_link_libraries(dfconvert DocFormats ${LIBS})
source_group(src FILES ${SOURCES})
set_property(TARGET dfconvert PROPERTY FOLDER consumers)
