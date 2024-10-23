# Copyright 2018 Hippo Seven
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

cmake_minimum_required(VERSION 3.4.1)
project(image-webp C)

set(WEBPLIB_SOURCES
    image_webp.c
    giflib/lib/dgif_lib.c
    giflib/lib/gif_hash.c
    giflib/lib/gifalloc.c
    giflib/lib/openbsd-reallocarray.c
)

set(WEBPLIB_INCLUDES
    ${IMAGE_EXPORT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/webplib/lib
)

set(WEBPLIB_DEFINITIONS
)

set(WEBPLIB_LIBRARIES
)

if(IMAGE_SINGLE_SHARED_LIB)
    # Compile a static library for image-singlelib
    set(WEBPLIB_LIBRARY_NAME image-webp-static)
    set(WEBPLIB_LIBRARY_TYPE STATIC)
    set(WEBPLIB_INCLUDES_VISIBILITY PUBLIC) # Share include dir with image project
else()
    # Compile a shared library to be imported by image-core
    set(WEBPLIB_LIBRARY_NAME image-webp)
    set(WEBPLIB_LIBRARY_TYPE SHARED)
    set(WEBPLIB_INCLUDES_VISIBILITY PRIVATE)
    
    # Link against image shared lib to get helper methods
    set(WEBPLIB_LIBRARIES
        ${WEBPLIB_LIBRARIES}
        image
        log
    )
endif()

add_library(${WEBPLIB_LIBRARY_NAME} ${WEBPLIB_LIBRARY_TYPE} ${WEBPLIB_SOURCES})
target_include_directories(${WEBPLIB_LIBRARY_NAME} ${WEBPLIB_INCLUDES_VISIBILITY} ${WEBPLIB_INCLUDES})
target_compile_definitions(${WEBPLIB_LIBRARY_NAME} PRIVATE ${WEBPLIB_DEFINITIONS})
target_link_libraries(${WEBPLIB_LIBRARY_NAME} PRIVATE ${WEBPLIB_LIBRARIES})