/*
 * Copyright 2015 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IMAGE_IMAGE_WEBP_H
#define IMAGE_IMAGE_WEBP_H

#include <stdbool.h>

#include "gif_lib.h"
#include "animated_image.h"
#include "image_library.h"
#include "stream.h"


#define IMAGE_WEBP_DECODER_DESCRIPTION ("libweb rfc")

#define IMAGE_WEBP_MAGIC_NUMBER_0 0x52
#define IMAGE_WEBP_MAGIC_NUMBER_1 0x49
#define IMAGE_WEBP_MAGIC_NUMBER_2 0x57
#define IMAGE_WEBP_MAGIC_NUMBER_3 0x45

bool gif_init(ImageLibrary* library);

bool gif_is_magic(Stream* stream);

const char* gif_get_description();

AnimatedImage* gif_decode(Stream* stream, bool partially, bool* animated);

bool gif_decode_info(Stream* stream, ImageInfo* info);


#endif // IMAGE_IMAGE_WEBP_H
