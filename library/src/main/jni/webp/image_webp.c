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

#include <stdlib.h>

#include "image.h"
#include "decode.h"
#include "encode.h"
#include "types.h"
#include "../log.h"

#define IMAGE_WEBP_PREPARE_NONE 0x00
#define IMAGE_WEBP_PREPARE_BACKGROUND 0x01
#define IMAGE_WEBP_PREPARE_USE_BACKUP 0x02

typedef struct
{
  int tran;
  int disposal;
  int delay;
  int prepare;
} WebPFrame;

typedef struct
{
  // WebPInfo *webp_file;
  WebPFrame *frames;
  Stream *stream;
} WebPData;

typedef struct
{
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} RGBA;

static int error_code = 0;

LIBRARY_EXPORT
bool webp_init(ImageLibrary *library)
{
  library->loaded = true;
  library->is_magic = webp_is_magic;
  library->decode = webp_decode;
  library->decode_info = webp_decode_info;
  library->decode_buffer = NULL;
  library->create = NULL;
  library->get_description = webp_get_description;

  return true;
}

bool webp_is_magic(Stream *stream)
{
  uint8_t magic[2];

  size_t read = stream->peek(stream, magic, sizeof(magic));
  if (read != sizeof(magic))
  {
    LOGE(MSG("Could not read %zu bytes from stream, only read %zu"), sizeof(magic), read);
    return false;
  }
  bool r = magic[0] == IMAGE_WEBP_MAGIC_NUMBER_0 && magic[1] == IMAGE_WEBP_MAGIC_NUMBER_1;
  if (r)
  {
    return r;
  }

  return magic[0] == IMAGE_WEBP_MAGIC_NUMBER_2 && magic[1] == IMAGE_WEBP_MAGIC_NUMBER_3;
}

const char *webp_get_description()
{
  return IMAGE_WEBP_DECODER_DESCRIPTION;
}

AnimatedImage *webp_decode(Stream *stream, bool partially, bool *animated)
{
  *animated = true;

  AnimatedImage *animated_image = NULL;
  WebPData *webp_data = NULL;
  WebPFrame *frames = NULL;
  WebPInfo *webp_file = NULL;
  int i;

  animated_image = malloc(sizeof(AnimatedImage));
  webp_data = malloc(sizeof(WebPData));
  if (animated_image == NULL || webp_data == NULL)
  {
    WTF_OOM;
    free(animated_image);
    free(webp_data);
    return NULL;
  }

  // Open
  webp_file = DWebPOpen(stream, &custom_read_fun, &error_code);
  if (webp_file == NULL)
  {
    WTF_OOM;
    free(animated_image);
    free(webp_data);
    return NULL;
  }

  if (partially)
  {
    // Glance
    if (DWebPGlance(webp_file) != GIF_OK)
    {
      LOGE(MSG("GIF error code %d"), error_code);
      DWebPCloseFile(webp_file, &error_code);
      free(animated_image);
      free(webp_data);
      return NULL;
    }

    // Frame info
    frames = malloc(sizeof(WebPFrame));
    if (frames == NULL)
    {
      WTF_OOM;
      DWebPCloseFile(webp_file, &error_code);
      free(animated_image);
      free(webp_data);
      return NULL;
    }

    // Read gcb
    read_gcb(webp_file, 0, frames, NULL);
  }
  else
  {
    // Slurp
    if (DWebPSlurp(webp_file) == GIF_ERROR)
    {
      fix_webp_file(webp_file);
    }
    if (webp_file->ImageCount <= 0)
    {
      LOGE(MSG("No frame"));
      DWebPCloseFile(webp_file, &error_code);
      free(animated_image);
      free(webp_data);
      return NULL;
    }

    // Frame info
    frames = malloc(webp_file->ImageCount * sizeof(WebPFrame));
    if (frames == NULL)
    {
      WTF_OOM;
      DWebPCloseFile(webp_file, &error_code);
      free(animated_image);
      free(webp_data);
      return NULL;
    }

    // Read gcb
    for (i = 0; i < webp_file->ImageCount; i++)
    {
      read_gcb(webp_file, i, frames + i, i == 0 ? NULL : frames + (i - 1));
    }
  }

  webp_data->webp_file = webp_file;
  webp_data->frames = frames;
  webp_data->stream = partially ? stream : NULL;

  animated_image->width = (uint32_t)webp_file->SWidth;
  animated_image->height = (uint32_t)webp_file->SHeight;
  animated_image->format = IMAGE_FORMAT_GIF;
  animated_image->opaque = frames->tran < 0;
  animated_image->completed = !partially;
  animated_image->data = webp_data;

  animated_image->get_stream = &get_stream;
  animated_image->complete = &complete;
  animated_image->get_frame_count = &get_frame_count;
  animated_image->get_delay = &get_delay;
  animated_image->get_byte_count = &get_byte_count;
  animated_image->advance = &advance;
  animated_image->recycle = &recycle;

  return animated_image;
}

bool webp_decode_info(Stream *stream, ImageInfo *info)
{
  WebPInfo *webp_file = NULL;

  // Open
  webp_file = DWebPOpen(stream, &custom_read_fun, &error_code);
  if (webp_file == NULL)
  {
    WTF_OOM;
    return NULL;
  }

  info->width = (uint32_t)webp_file->SWidth;
  info->height = (uint32_t)webp_file->SHeight;
  info->format = IMAGE_FORMAT_GIF;
  info->opaque = false;   // Can't get opaque state, set false
  info->frame_count = -1; // For webp, must decode all frame to get frame count.

  DWebPCloseFile(webp_file, &error_code);
  return true;
}