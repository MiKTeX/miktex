## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/libressl/ssl")

set(ssl_source_dir ${libressl_source_dir}/ssl)

include_directories(BEFORE
  ${ssl_source_dir}
)

list(APPEND ssl_sources
  ${ssl_source_dir}/bio_ssl.c
  ${ssl_source_dir}/bs_ber.c
  ${ssl_source_dir}/bs_cbb.c
  ${ssl_source_dir}/bs_cbs.c
  ${ssl_source_dir}/d1_both.c
  ${ssl_source_dir}/d1_clnt.c
  ${ssl_source_dir}/d1_enc.c
  ${ssl_source_dir}/d1_lib.c
  ${ssl_source_dir}/d1_meth.c
  ${ssl_source_dir}/d1_pkt.c
  ${ssl_source_dir}/d1_srtp.c
  ${ssl_source_dir}/d1_srvr.c
  ${ssl_source_dir}/pqueue.c
  ${ssl_source_dir}/s23_clnt.c
  ${ssl_source_dir}/s23_lib.c
  ${ssl_source_dir}/s23_pkt.c
  ${ssl_source_dir}/s23_srvr.c
  ${ssl_source_dir}/s3_both.c
  ${ssl_source_dir}/s3_cbc.c
  ${ssl_source_dir}/s3_clnt.c
  ${ssl_source_dir}/s3_lib.c
  ${ssl_source_dir}/s3_pkt.c
  ${ssl_source_dir}/s3_srvr.c
  ${ssl_source_dir}/ssl_algs.c
  ${ssl_source_dir}/ssl_asn1.c
  ${ssl_source_dir}/ssl_cert.c
  ${ssl_source_dir}/ssl_ciph.c
  ${ssl_source_dir}/ssl_err.c
  ${ssl_source_dir}/ssl_err2.c
  ${ssl_source_dir}/ssl_lib.c
  ${ssl_source_dir}/ssl_rsa.c
  ${ssl_source_dir}/ssl_sess.c
  ${ssl_source_dir}/ssl_stat.c
  ${ssl_source_dir}/ssl_txt.c
  ${ssl_source_dir}/t1_clnt.c
  ${ssl_source_dir}/t1_enc.c
  ${ssl_source_dir}/t1_lib.c
  ${ssl_source_dir}/t1_meth.c
  ${ssl_source_dir}/t1_reneg.c
  ${ssl_source_dir}/t1_srvr.c
)  
  
if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
