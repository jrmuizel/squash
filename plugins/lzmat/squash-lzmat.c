/* Copyright (c) 2013 The Squash Authors
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Evan Nemerson <evan@coeus-group.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <squash/squash.h>

#include "lzmat/lzmat.h"

SquashStatus squash_plugin_init_codec (SquashCodec* codec, SquashCodecFuncs* funcs);

static size_t
squash_lzmat_get_max_compressed_size (SquashCodec* codec, size_t uncompressed_length) {
  return MAX_LZMAT_ENCODED_SIZE (uncompressed_length);
}

static SquashStatus
squash_lzmat_status (int status) {
  SquashStatus res;

  switch (status) {
    case LZMAT_STATUS_OK:
      res = SQUASH_OK;
      break;
    case LZMAT_STATUS_BUFFER_TOO_SMALL:
      res = SQUASH_BUFFER_FULL;
      break;
    default:
      res = SQUASH_FAILED;
      break;
  }

  return res;
}

static SquashStatus
squash_lzmat_compress_buffer (SquashCodec* codec,
                              uint8_t* compressed, size_t* compressed_length,
                              const uint8_t* uncompressed, size_t uncompressed_length,
                              SquashOptions* options) {
  int lzmat_e;
  MP_U32 compressed_size = (MP_U32) *compressed_length;

  lzmat_e = lzmat_encode ((MP_U8*) compressed, &compressed_size,
                          (MP_U8*) uncompressed, (MP_U32) uncompressed_length);
  if (lzmat_e != LZMAT_STATUS_OK) {
    return squash_lzmat_status (lzmat_e);
  }
  *compressed_length = (size_t) compressed_size;

  return SQUASH_OK;
}

static SquashStatus
squash_lzmat_decompress_buffer (SquashCodec* codec,
                                uint8_t* decompressed, size_t* decompressed_length,
                                const uint8_t* compressed, size_t compressed_length,
                                SquashOptions* options) {
  int lzmat_e;
  MP_U32 decompressed_size = (MP_U32) *decompressed_length;

  lzmat_e = lzmat_decode ((MP_U8*) decompressed, &decompressed_size,
                          (MP_U8*) compressed, (MP_U32) compressed_length);
  if (lzmat_e != LZMAT_STATUS_OK) {
    return squash_lzmat_status (lzmat_e);
  }
  *decompressed_length = (size_t) decompressed_size;

  return SQUASH_OK;
}

SquashStatus
squash_plugin_init_codec (SquashCodec* codec, SquashCodecFuncs* funcs) {
  const char* name = squash_codec_get_name (codec);

  if (strcmp ("lzmat", name) == 0) {
    funcs->get_max_compressed_size = squash_lzmat_get_max_compressed_size;
    funcs->compress_buffer = squash_lzmat_compress_buffer;
    funcs->decompress_buffer = squash_lzmat_decompress_buffer;
  } else {
    return SQUASH_UNABLE_TO_LOAD;
  }

  return SQUASH_OK;
}