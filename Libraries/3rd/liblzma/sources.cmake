## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2006-2018 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(liblzma_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/common/tuklib_physmem.c

  # COND_THREADS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/common/tuklib_cpucores.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_preset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_preset.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/hardware_physmem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/memcmplen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_flags_common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_flags_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/vli_size.c

  # COND_MAIN_ENCODER
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/alone_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_buffer_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_buffer_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_header_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_buffer_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_encoder_memusage.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_buffer_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_flags_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_buffer_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_flags_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/vli_encoder.c

  # COND_MAIN_ENCODER AND COND_THREADS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/hardware_cputhreads.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/outqueue.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/outqueue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_encoder_mt.c

  # COND_MAIN_DECODER
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/alone_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/alone_decoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/auto_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_buffer_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_decoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/block_header_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/easy_decoder_memusage.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_buffer_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_decoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/filter_flags_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/index_hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_buffer_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_decoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/stream_flags_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/common/vli_decoder.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/check.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/check.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc_macros.h

  # COND_CHECK_CRC32 AND COND_SMALL
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc32_fast.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc32_table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc32_table_be.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc32_table_le.h

  # COND_CHECK_CRC64 AND COND_SMALL
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc64_fast.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc64_table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc64_table_be.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/crc64_table_le.h

  # COND_CHECK_SHA256
  # COND_INTERNAL_SHA256
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/check/sha256.c
  
  # COND_FILTER_LZ AND COND_ENCODER_LZ
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_encoder_hash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_encoder_hash_table.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_encoder_mf.c
  
  # COND_FILTER_LZ AND COND_DECODER_LZ
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lz/lz_decoder.h

  # COND_FILTER_LZMA1 AND COND_ENCODER_LZMA1
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/fastpos.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder_optimum_fast.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder_optimum_normal.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder_presets.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_encoder_private.h

  # COND_FILTER_LZMA1 AND COND_SMALL
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/fastpos_table.c
        
  # COND_FILTER_LZMA1 AND COND_DECODER_LZMA1
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma_decoder.h

  # COND_FILTER_LZMA1 AND COND_ENCODER_LZMA2
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma2_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma2_encoder.h

  # COND_FILTER_LZMA1 AND COND_DECODER_LZMA2
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma2_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/lzma/lzma2_decoder.h

  # COND_FILTER_LZMA1
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder/range_common.h

  # COND_FILTER_LZMA1 AND COND_ENCODER_LZMA1
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder/price.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder/price_table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder/range_encoder.h

  # COND_FILTER_LZMA1 AND COND_DECODER_LZMA1
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/rangecoder/range_decoder.h

  # COND_FILTER_DELTA
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_common.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_private.h

  # COND_FILTER_DELTA AND COND_ENCODER_DELTA
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_encoder.h

  # COND_FILTER_DELTA AND COND_DECODER_DELTA
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/delta/delta_decoder.h

  # COND_FILTER_SIMPLE
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_coder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_coder.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_private.h

  # COND_FILTER_SIMPLE AND COND_ENCODER_SIMPLE
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_encoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_encoder.h

  # COND_FILTER_SIMPLE AND COND_DECODER_SIMPLE
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_decoder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/simple_decoder.h

  # COND_FILTER_SIMPLE AND COND_FILTER_X86
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/x86.c

  # COND_FILTER_SIMPLE AND COND_FILTER_POWERPC
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/powerpc.c

  # COND_FILTER_SIMPLE AND COND_FILTER_IA64
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/ia64.c

  # COND_FILTER_SIMPLE AND COND_FILTER_ARM
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/arm.c

  # COND_FILTER_SIMPLE AND COND_FILTER_ARMTHUMB
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/armthumb.c

  # COND_FILTER_SIMPLE AND COND_FILTER_SPARC
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/liblzma/simple/sparc.c
  
  ${CMAKE_CURRENT_SOURCE_DIR}/liblzma-version.h
)
