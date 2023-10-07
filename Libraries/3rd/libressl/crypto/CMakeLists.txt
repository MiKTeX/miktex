## CMakeLists.txt
##
## Copyright (C) 2016-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/libressl/crypto")

set(crypto_source_dir ${libressl_source_dir}/crypto)

add_definitions(
    -DOPENSSL_NO_ASM
)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${crypto_source_dir}
    ${crypto_source_dir}/asn1
    ${crypto_source_dir}/bio
    ${crypto_source_dir}/bn
    ${crypto_source_dir}/bytestring
    ${crypto_source_dir}/curve25519
    ${crypto_source_dir}/dh
    ${crypto_source_dir}/dsa
    ${crypto_source_dir}/ec
    ${crypto_source_dir}/ecdh
    ${crypto_source_dir}/ecdsa
    ${crypto_source_dir}/evp
    ${crypto_source_dir}/hidden
    ${crypto_source_dir}/hmac
    ${crypto_source_dir}/modes
    ${crypto_source_dir}/ocsp
    ${crypto_source_dir}/pkcs12
    ${crypto_source_dir}/rsa
    ${crypto_source_dir}/sha
    ${crypto_source_dir}/x509
)

if(HOST_AARCH64)
    include_directories(${crypto_source_dir}/bn/arch/aarch64/)
elseif(HOST_ARM)
    include_directories(${crypto_source_dir}/bn/arch/arm/)
elseif(HOST_I386)
    include_directories(${crypto_source_dir}/bn/arch/i386/)
elseif(HOST_MIPS64)
    include_directories(${crypto_source_dir}/bn/arch/mips64)
elseif(HOST_POWERPC)
    include_directories(${crypto_source_dir}/bn/arch/powerpc)
elseif(HOST_POWERPC64)
    include_directories(${crypto_source_dir}/bn/arch/powerpc64)
elseif(HOST_RISCV64)
    include_directories(${crypto_source_dir}/bn/arch/riscv64)
elseif(HOST_SPARC64)
    include_directories(${crypto_source_dir}/bn/arch/sparc64)
elseif(HOST_X86_64)
    include_directories(${crypto_source_dir}/bn/arch/amd64)
endif()

list(APPEND crypto_sources
    ${crypto_source_dir}/aes/aes_core.c
)

list(APPEND crypto_sources
    ${crypto_source_dir}/aes/aes_cbc.c
    ${crypto_source_dir}/camellia/camellia.c
    ${crypto_source_dir}/camellia/cmll_cbc.c
    ${crypto_source_dir}/rc4/rc4_enc.c
    ${crypto_source_dir}/rc4/rc4_skey.c
    ${crypto_source_dir}/whrlpool/wp_block.c
)  
  
list(APPEND crypto_sources
    ${crypto_source_dir}/cpt_err.c
    ${crypto_source_dir}/cryptlib.c
    ${crypto_source_dir}/crypto_init.c
    ${crypto_source_dir}/cversion.c
    ${crypto_source_dir}/ex_data.c
    ${crypto_source_dir}/malloc-wrapper.c
    ${crypto_source_dir}/mem_clr.c
    ${crypto_source_dir}/mem_dbg.c
    ${crypto_source_dir}/o_fips.c
    ${crypto_source_dir}/o_init.c
    ${crypto_source_dir}/o_str.c
    ${crypto_source_dir}/aes/aes_cfb.c
    ${crypto_source_dir}/aes/aes_ctr.c
    ${crypto_source_dir}/aes/aes_ecb.c
    ${crypto_source_dir}/aes/aes_ige.c
    ${crypto_source_dir}/aes/aes_ofb.c
    ${crypto_source_dir}/aes/aes_wrap.c
    ${crypto_source_dir}/asn1/a_bitstr.c
    ${crypto_source_dir}/asn1/a_enum.c
    ${crypto_source_dir}/asn1/a_int.c
    ${crypto_source_dir}/asn1/a_mbstr.c
    ${crypto_source_dir}/asn1/a_object.c
    ${crypto_source_dir}/asn1/a_octet.c
    ${crypto_source_dir}/asn1/a_pkey.c
    ${crypto_source_dir}/asn1/a_print.c
    ${crypto_source_dir}/asn1/a_pubkey.c
    ${crypto_source_dir}/asn1/a_strex.c
    ${crypto_source_dir}/asn1/a_string.c
    ${crypto_source_dir}/asn1/a_strnid.c
    ${crypto_source_dir}/asn1/a_time.c
    ${crypto_source_dir}/asn1/a_time_posix.c
    ${crypto_source_dir}/asn1/a_time_tm.c
    ${crypto_source_dir}/asn1/a_type.c
    ${crypto_source_dir}/asn1/a_utf8.c
    ${crypto_source_dir}/asn1/ameth_lib.c
    ${crypto_source_dir}/asn1/asn1_err.c
    ${crypto_source_dir}/asn1/asn1_gen.c
    ${crypto_source_dir}/asn1/asn1_item.c
    ${crypto_source_dir}/asn1/asn1_lib.c
    ${crypto_source_dir}/asn1/asn1_old.c
    ${crypto_source_dir}/asn1/asn1_old_lib.c
    ${crypto_source_dir}/asn1/asn1_par.c
    ${crypto_source_dir}/asn1/asn1_types.c
    ${crypto_source_dir}/asn1/asn_mime.c
    ${crypto_source_dir}/asn1/asn_moid.c
    ${crypto_source_dir}/asn1/bio_asn1.c
    ${crypto_source_dir}/asn1/bio_ndef.c
    ${crypto_source_dir}/asn1/p5_pbe.c
    ${crypto_source_dir}/asn1/p5_pbev2.c
    ${crypto_source_dir}/asn1/p8_pkey.c
    ${crypto_source_dir}/asn1/t_crl.c
    ${crypto_source_dir}/asn1/t_req.c
    ${crypto_source_dir}/asn1/t_spki.c
    ${crypto_source_dir}/asn1/t_x509.c
    ${crypto_source_dir}/asn1/t_x509a.c
    ${crypto_source_dir}/asn1/tasn_dec.c
    ${crypto_source_dir}/asn1/tasn_enc.c
    ${crypto_source_dir}/asn1/tasn_fre.c
    ${crypto_source_dir}/asn1/tasn_new.c
    ${crypto_source_dir}/asn1/tasn_prn.c
    ${crypto_source_dir}/asn1/tasn_typ.c
    ${crypto_source_dir}/asn1/tasn_utl.c
    ${crypto_source_dir}/asn1/x_algor.c
    ${crypto_source_dir}/asn1/x_attrib.c
    ${crypto_source_dir}/asn1/x_bignum.c
    ${crypto_source_dir}/asn1/x_crl.c
    ${crypto_source_dir}/asn1/x_exten.c
    ${crypto_source_dir}/asn1/x_info.c
    ${crypto_source_dir}/asn1/x_long.c
    ${crypto_source_dir}/asn1/x_name.c
    ${crypto_source_dir}/asn1/x_pkey.c
    ${crypto_source_dir}/asn1/x_pubkey.c
    ${crypto_source_dir}/asn1/x_req.c
    ${crypto_source_dir}/asn1/x_sig.c
    ${crypto_source_dir}/asn1/x_spki.c
    ${crypto_source_dir}/asn1/x_val.c
    ${crypto_source_dir}/asn1/x_x509.c
    ${crypto_source_dir}/asn1/x_x509a.c
    ${crypto_source_dir}/bf/bf_cfb64.c
    ${crypto_source_dir}/bf/bf_ecb.c
    ${crypto_source_dir}/bf/bf_enc.c
    ${crypto_source_dir}/bf/bf_ofb64.c
    ${crypto_source_dir}/bf/bf_skey.c
    ${crypto_source_dir}/bio/b_dump.c
    ${crypto_source_dir}/bio/b_print.c
    ${crypto_source_dir}/bio/b_sock.c
    ${crypto_source_dir}/bio/bf_buff.c
    ${crypto_source_dir}/bio/bf_nbio.c
    ${crypto_source_dir}/bio/bf_null.c
    ${crypto_source_dir}/bio/bio_cb.c
    ${crypto_source_dir}/bio/bio_err.c
    ${crypto_source_dir}/bio/bio_lib.c
    ${crypto_source_dir}/bio/bio_meth.c
    ${crypto_source_dir}/bio/bss_acpt.c
    ${crypto_source_dir}/bio/bss_bio.c
    ${crypto_source_dir}/bio/bss_conn.c
    ${crypto_source_dir}/bio/bss_dgram.c
    ${crypto_source_dir}/bio/bss_fd.c
    ${crypto_source_dir}/bio/bss_file.c
    ${crypto_source_dir}/bio/bss_mem.c
    ${crypto_source_dir}/bio/bss_null.c
    ${crypto_source_dir}/bio/bss_sock.c
    ${crypto_source_dir}/bn/bn_add.c
    ${crypto_source_dir}/bn/bn_bpsw.c
    ${crypto_source_dir}/bn/bn_const.c
    ${crypto_source_dir}/bn/bn_convert.c
    ${crypto_source_dir}/bn/bn_ctx.c
    ${crypto_source_dir}/bn/bn_div.c
    ${crypto_source_dir}/bn/bn_err.c
    ${crypto_source_dir}/bn/bn_exp.c
    ${crypto_source_dir}/bn/bn_gcd.c
    ${crypto_source_dir}/bn/bn_isqrt.c
    ${crypto_source_dir}/bn/bn_kron.c
    ${crypto_source_dir}/bn/bn_lib.c
    ${crypto_source_dir}/bn/bn_mod.c
    ${crypto_source_dir}/bn/bn_mod_sqrt.c
    ${crypto_source_dir}/bn/bn_mont.c
    ${crypto_source_dir}/bn/bn_mul.c
    ${crypto_source_dir}/bn/bn_prime.c
    ${crypto_source_dir}/bn/bn_primitives.c
    ${crypto_source_dir}/bn/bn_print.c
    ${crypto_source_dir}/bn/bn_rand.c
    ${crypto_source_dir}/bn/bn_recp.c
    ${crypto_source_dir}/bn/bn_shift.c
    ${crypto_source_dir}/bn/bn_small_primes.c
    ${crypto_source_dir}/bn/bn_sqr.c
    ${crypto_source_dir}/bn/bn_word.c
    ${crypto_source_dir}/buffer/buf_err.c
    ${crypto_source_dir}/buffer/buffer.c
    ${crypto_source_dir}/bytestring/bs_ber.c
    ${crypto_source_dir}/bytestring/bs_cbb.c
    ${crypto_source_dir}/bytestring/bs_cbs.c
    ${crypto_source_dir}/camellia/cmll_cfb.c
    ${crypto_source_dir}/camellia/cmll_ctr.c
    ${crypto_source_dir}/camellia/cmll_ecb.c
    ${crypto_source_dir}/camellia/cmll_misc.c
    ${crypto_source_dir}/camellia/cmll_ofb.c
    ${crypto_source_dir}/cast/c_cfb64.c
    ${crypto_source_dir}/cast/c_ecb.c
    ${crypto_source_dir}/cast/c_enc.c
    ${crypto_source_dir}/cast/c_ofb64.c
    ${crypto_source_dir}/cast/c_skey.c
    ${crypto_source_dir}/chacha/chacha.c
    ${crypto_source_dir}/cmac/cm_ameth.c
    ${crypto_source_dir}/cmac/cm_pmeth.c
    ${crypto_source_dir}/cmac/cmac.c
    ${crypto_source_dir}/cms/cms_asn1.c
    ${crypto_source_dir}/cms/cms_att.c
    ${crypto_source_dir}/cms/cms_dd.c
    ${crypto_source_dir}/cms/cms_enc.c
    ${crypto_source_dir}/cms/cms_env.c
    ${crypto_source_dir}/cms/cms_err.c
    ${crypto_source_dir}/cms/cms_ess.c
    ${crypto_source_dir}/cms/cms_io.c
    ${crypto_source_dir}/cms/cms_kari.c
    ${crypto_source_dir}/cms/cms_lib.c
    ${crypto_source_dir}/cms/cms_pwri.c
    ${crypto_source_dir}/cms/cms_sd.c
    ${crypto_source_dir}/cms/cms_smime.c
    ${crypto_source_dir}/conf/conf_api.c
    ${crypto_source_dir}/conf/conf_def.c
    ${crypto_source_dir}/conf/conf_err.c
    ${crypto_source_dir}/conf/conf_lib.c
    ${crypto_source_dir}/conf/conf_mall.c
    ${crypto_source_dir}/conf/conf_mod.c
    ${crypto_source_dir}/conf/conf_sap.c
    ${crypto_source_dir}/ct/ct_b64.c
    ${crypto_source_dir}/ct/ct_err.c
    ${crypto_source_dir}/ct/ct_log.c
    ${crypto_source_dir}/ct/ct_oct.c
    ${crypto_source_dir}/ct/ct_policy.c
    ${crypto_source_dir}/ct/ct_prn.c
    ${crypto_source_dir}/ct/ct_sct.c
    ${crypto_source_dir}/ct/ct_sct_ctx.c
    ${crypto_source_dir}/ct/ct_vfy.c
    ${crypto_source_dir}/ct/ct_x509v3.c
    ${crypto_source_dir}/curve25519/curve25519-generic.c
    ${crypto_source_dir}/curve25519/curve25519.c
    ${crypto_source_dir}/des/cbc_cksm.c
    ${crypto_source_dir}/des/cbc_enc.c
    ${crypto_source_dir}/des/cfb64ede.c
    ${crypto_source_dir}/des/cfb64enc.c
    ${crypto_source_dir}/des/cfb_enc.c
    ${crypto_source_dir}/des/des_enc.c
    ${crypto_source_dir}/des/ecb3_enc.c
    ${crypto_source_dir}/des/ecb_enc.c
    ${crypto_source_dir}/des/ede_cbcm_enc.c
    ${crypto_source_dir}/des/enc_read.c
    ${crypto_source_dir}/des/enc_writ.c
    ${crypto_source_dir}/des/fcrypt.c
    ${crypto_source_dir}/des/fcrypt_b.c
    ${crypto_source_dir}/des/ofb64ede.c
    ${crypto_source_dir}/des/ofb64enc.c
    ${crypto_source_dir}/des/ofb_enc.c
    ${crypto_source_dir}/des/pcbc_enc.c
    ${crypto_source_dir}/des/qud_cksm.c
    ${crypto_source_dir}/des/rand_key.c
    ${crypto_source_dir}/des/set_key.c
    ${crypto_source_dir}/des/str2key.c
    ${crypto_source_dir}/des/xcbc_enc.c
    ${crypto_source_dir}/dh/dh_ameth.c
    ${crypto_source_dir}/dh/dh_asn1.c
    ${crypto_source_dir}/dh/dh_check.c
    ${crypto_source_dir}/dh/dh_err.c
    ${crypto_source_dir}/dh/dh_gen.c
    ${crypto_source_dir}/dh/dh_key.c
    ${crypto_source_dir}/dh/dh_lib.c
    ${crypto_source_dir}/dh/dh_pmeth.c
    ${crypto_source_dir}/dsa/dsa_ameth.c
    ${crypto_source_dir}/dsa/dsa_asn1.c
    ${crypto_source_dir}/dsa/dsa_err.c
    ${crypto_source_dir}/dsa/dsa_gen.c
    ${crypto_source_dir}/dsa/dsa_key.c
    ${crypto_source_dir}/dsa/dsa_lib.c
    ${crypto_source_dir}/dsa/dsa_meth.c
    ${crypto_source_dir}/dsa/dsa_ossl.c
    ${crypto_source_dir}/dsa/dsa_pmeth.c
    ${crypto_source_dir}/dsa/dsa_prn.c
    ${crypto_source_dir}/ec/ec_ameth.c
    ${crypto_source_dir}/ec/ec_asn1.c
    ${crypto_source_dir}/ec/ec_check.c
    ${crypto_source_dir}/ec/ec_curve.c
    ${crypto_source_dir}/ec/ec_cvt.c
    ${crypto_source_dir}/ec/ec_err.c
    ${crypto_source_dir}/ec/ec_key.c
    ${crypto_source_dir}/ec/ec_kmeth.c
    ${crypto_source_dir}/ec/ec_lib.c
    ${crypto_source_dir}/ec/ec_mult.c
    ${crypto_source_dir}/ec/ec_oct.c
    ${crypto_source_dir}/ec/ec_pmeth.c
    ${crypto_source_dir}/ec/ec_print.c
    ${crypto_source_dir}/ec/eck_prn.c
    ${crypto_source_dir}/ec/ecp_mont.c
    ${crypto_source_dir}/ec/ecp_oct.c
    ${crypto_source_dir}/ec/ecp_smpl.c
    ${crypto_source_dir}/ec/ecx_methods.c
    ${crypto_source_dir}/ecdh/ecdh.c
    ${crypto_source_dir}/ecdsa/ecdsa.c
    ${crypto_source_dir}/engine/engine_stubs.c
    ${crypto_source_dir}/err/err.c
    ${crypto_source_dir}/err/err_all.c
    ${crypto_source_dir}/err/err_prn.c
    ${crypto_source_dir}/evp/bio_b64.c
    ${crypto_source_dir}/evp/bio_enc.c
    ${crypto_source_dir}/evp/bio_md.c
    ${crypto_source_dir}/evp/c_all.c
    ${crypto_source_dir}/evp/cipher_method_lib.c
    ${crypto_source_dir}/evp/digest.c
    ${crypto_source_dir}/evp/e_aes.c
    ${crypto_source_dir}/evp/e_aes_cbc_hmac_sha1.c
    ${crypto_source_dir}/evp/e_bf.c
    ${crypto_source_dir}/evp/e_camellia.c
    ${crypto_source_dir}/evp/e_cast.c
    ${crypto_source_dir}/evp/e_chacha.c
    ${crypto_source_dir}/evp/e_chacha20poly1305.c
    ${crypto_source_dir}/evp/e_des.c
    ${crypto_source_dir}/evp/e_des3.c
    ${crypto_source_dir}/evp/e_gost2814789.c
    ${crypto_source_dir}/evp/e_idea.c
    ${crypto_source_dir}/evp/e_null.c
    ${crypto_source_dir}/evp/e_rc2.c
    ${crypto_source_dir}/evp/e_rc4.c
    ${crypto_source_dir}/evp/e_rc4_hmac_md5.c
    ${crypto_source_dir}/evp/e_sm4.c
    ${crypto_source_dir}/evp/e_xcbc_d.c
    ${crypto_source_dir}/evp/encode.c
    ${crypto_source_dir}/evp/evp_aead.c
    ${crypto_source_dir}/evp/evp_enc.c
    ${crypto_source_dir}/evp/evp_err.c
    ${crypto_source_dir}/evp/evp_key.c
    ${crypto_source_dir}/evp/evp_lib.c
    ${crypto_source_dir}/evp/evp_pbe.c
    ${crypto_source_dir}/evp/evp_pkey.c
    ${crypto_source_dir}/evp/m_gost2814789.c
    ${crypto_source_dir}/evp/m_gostr341194.c
    ${crypto_source_dir}/evp/m_md4.c
    ${crypto_source_dir}/evp/m_md5.c
    ${crypto_source_dir}/evp/m_md5_sha1.c
    ${crypto_source_dir}/evp/m_null.c
    ${crypto_source_dir}/evp/m_ripemd.c
    ${crypto_source_dir}/evp/m_sha1.c
    ${crypto_source_dir}/evp/m_sha3.c
    ${crypto_source_dir}/evp/m_sigver.c
    ${crypto_source_dir}/evp/m_streebog.c
    ${crypto_source_dir}/evp/m_sm3.c
    ${crypto_source_dir}/evp/m_wp.c
    ${crypto_source_dir}/evp/names.c
    ${crypto_source_dir}/evp/p5_crpt.c
    ${crypto_source_dir}/evp/p5_crpt2.c
    ${crypto_source_dir}/evp/p_dec.c
    ${crypto_source_dir}/evp/p_enc.c
    ${crypto_source_dir}/evp/p_lib.c
    ${crypto_source_dir}/evp/p_open.c
    ${crypto_source_dir}/evp/p_seal.c
    ${crypto_source_dir}/evp/p_sign.c
    ${crypto_source_dir}/evp/p_verify.c
    ${crypto_source_dir}/evp/pmeth_fn.c
    ${crypto_source_dir}/evp/pmeth_gn.c
    ${crypto_source_dir}/evp/pmeth_lib.c
    ${crypto_source_dir}/gost/gost2814789.c
    ${crypto_source_dir}/gost/gost89_keywrap.c
    ${crypto_source_dir}/gost/gost89_params.c
    ${crypto_source_dir}/gost/gost89imit_ameth.c
    ${crypto_source_dir}/gost/gost89imit_pmeth.c
    ${crypto_source_dir}/gost/gost_asn1.c
    ${crypto_source_dir}/gost/gost_err.c
    ${crypto_source_dir}/gost/gostr341001.c
    ${crypto_source_dir}/gost/gostr341001_ameth.c
    ${crypto_source_dir}/gost/gostr341001_key.c
    ${crypto_source_dir}/gost/gostr341001_params.c
    ${crypto_source_dir}/gost/gostr341001_pmeth.c
    ${crypto_source_dir}/gost/gostr341194.c
    ${crypto_source_dir}/gost/streebog.c
    ${crypto_source_dir}/hkdf/hkdf.c
    ${crypto_source_dir}/hmac/hm_ameth.c
    ${crypto_source_dir}/hmac/hm_pmeth.c
    ${crypto_source_dir}/hmac/hmac.c
    ${crypto_source_dir}/idea/i_cbc.c
    ${crypto_source_dir}/idea/i_cfb64.c
    ${crypto_source_dir}/idea/i_ecb.c
    ${crypto_source_dir}/idea/i_ofb64.c
    ${crypto_source_dir}/idea/i_skey.c
    ${crypto_source_dir}/kdf/hkdf_evp.c
    ${crypto_source_dir}/kdf/kdf_err.c
    ${crypto_source_dir}/lhash/lh_stats.c
    ${crypto_source_dir}/lhash/lhash.c
    ${crypto_source_dir}/md4/md4.c
    ${crypto_source_dir}/md5/md5.c
    ${crypto_source_dir}/modes/cbc128.c
    ${crypto_source_dir}/modes/ccm128.c
    ${crypto_source_dir}/modes/cfb128.c
    ${crypto_source_dir}/modes/ctr128.c
    ${crypto_source_dir}/modes/gcm128.c
    ${crypto_source_dir}/modes/ofb128.c
    ${crypto_source_dir}/modes/xts128.c
    ${crypto_source_dir}/objects/o_names.c
    ${crypto_source_dir}/objects/obj_dat.c
    ${crypto_source_dir}/objects/obj_err.c
    ${crypto_source_dir}/objects/obj_lib.c
    ${crypto_source_dir}/objects/obj_xref.c
    ${crypto_source_dir}/ocsp/ocsp_asn.c
    ${crypto_source_dir}/ocsp/ocsp_cl.c
    ${crypto_source_dir}/ocsp/ocsp_err.c
    ${crypto_source_dir}/ocsp/ocsp_ext.c
    ${crypto_source_dir}/ocsp/ocsp_ht.c
    ${crypto_source_dir}/ocsp/ocsp_lib.c
    ${crypto_source_dir}/ocsp/ocsp_prn.c
    ${crypto_source_dir}/ocsp/ocsp_srv.c
    ${crypto_source_dir}/ocsp/ocsp_vfy.c
    ${crypto_source_dir}/pem/pem_all.c
    ${crypto_source_dir}/pem/pem_err.c
    ${crypto_source_dir}/pem/pem_info.c
    ${crypto_source_dir}/pem/pem_lib.c
    ${crypto_source_dir}/pem/pem_oth.c
    ${crypto_source_dir}/pem/pem_pk8.c
    ${crypto_source_dir}/pem/pem_pkey.c
    ${crypto_source_dir}/pem/pem_sign.c
    ${crypto_source_dir}/pem/pem_x509.c
    ${crypto_source_dir}/pem/pem_xaux.c
    ${crypto_source_dir}/pem/pvkfmt.c
    ${crypto_source_dir}/pkcs12/p12_add.c
    ${crypto_source_dir}/pkcs12/p12_asn.c
    ${crypto_source_dir}/pkcs12/p12_attr.c
    ${crypto_source_dir}/pkcs12/p12_crpt.c
    ${crypto_source_dir}/pkcs12/p12_crt.c
    ${crypto_source_dir}/pkcs12/p12_decr.c
    ${crypto_source_dir}/pkcs12/p12_init.c
    ${crypto_source_dir}/pkcs12/p12_key.c
    ${crypto_source_dir}/pkcs12/p12_kiss.c
    ${crypto_source_dir}/pkcs12/p12_mutl.c
    ${crypto_source_dir}/pkcs12/p12_npas.c
    ${crypto_source_dir}/pkcs12/p12_p8d.c
    ${crypto_source_dir}/pkcs12/p12_p8e.c
    ${crypto_source_dir}/pkcs12/p12_sbag.c
    ${crypto_source_dir}/pkcs12/p12_utl.c
    ${crypto_source_dir}/pkcs12/pk12err.c
    ${crypto_source_dir}/pkcs7/pk7_asn1.c
    ${crypto_source_dir}/pkcs7/pk7_attr.c
    ${crypto_source_dir}/pkcs7/pk7_doit.c
    ${crypto_source_dir}/pkcs7/pk7_lib.c
    ${crypto_source_dir}/pkcs7/pk7_mime.c
    ${crypto_source_dir}/pkcs7/pk7_smime.c
    ${crypto_source_dir}/pkcs7/pkcs7err.c
    ${crypto_source_dir}/poly1305/poly1305.c
    ${crypto_source_dir}/rand/rand_err.c
    ${crypto_source_dir}/rand/rand_lib.c
    ${crypto_source_dir}/rand/randfile.c
    ${crypto_source_dir}/rc2/rc2_cbc.c
    ${crypto_source_dir}/rc2/rc2_ecb.c
    ${crypto_source_dir}/rc2/rc2_skey.c
    ${crypto_source_dir}/rc2/rc2cfb64.c
    ${crypto_source_dir}/rc2/rc2ofb64.c
    ${crypto_source_dir}/ripemd/ripemd.c
    ${crypto_source_dir}/rsa/rsa_ameth.c
    ${crypto_source_dir}/rsa/rsa_asn1.c
    ${crypto_source_dir}/rsa/rsa_blinding.c
    ${crypto_source_dir}/rsa/rsa_chk.c
    ${crypto_source_dir}/rsa/rsa_eay.c
    ${crypto_source_dir}/rsa/rsa_err.c
    ${crypto_source_dir}/rsa/rsa_gen.c
    ${crypto_source_dir}/rsa/rsa_lib.c
    ${crypto_source_dir}/rsa/rsa_meth.c
    ${crypto_source_dir}/rsa/rsa_none.c
    ${crypto_source_dir}/rsa/rsa_oaep.c
    ${crypto_source_dir}/rsa/rsa_pk1.c
    ${crypto_source_dir}/rsa/rsa_pmeth.c
    ${crypto_source_dir}/rsa/rsa_prn.c
    ${crypto_source_dir}/rsa/rsa_pss.c
    ${crypto_source_dir}/rsa/rsa_saos.c
    ${crypto_source_dir}/rsa/rsa_sign.c
    ${crypto_source_dir}/rsa/rsa_x931.c
    ${crypto_source_dir}/sha/sha1.c
    ${crypto_source_dir}/sha/sha256.c
    ${crypto_source_dir}/sha/sha3.c
    ${crypto_source_dir}/sha/sha512.c
    ${crypto_source_dir}/sm3/sm3.c
    ${crypto_source_dir}/sm4/sm4.c
    ${crypto_source_dir}/stack/stack.c
    ${crypto_source_dir}/ts/ts_asn1.c
    ${crypto_source_dir}/ts/ts_conf.c
    ${crypto_source_dir}/ts/ts_err.c
    ${crypto_source_dir}/ts/ts_lib.c
    ${crypto_source_dir}/ts/ts_req_print.c
    ${crypto_source_dir}/ts/ts_req_utils.c
    ${crypto_source_dir}/ts/ts_rsp_print.c
    ${crypto_source_dir}/ts/ts_rsp_sign.c
    ${crypto_source_dir}/ts/ts_rsp_utils.c
    ${crypto_source_dir}/ts/ts_rsp_verify.c
    ${crypto_source_dir}/ts/ts_verify_ctx.c
    ${crypto_source_dir}/txt_db/txt_db.c
    ${crypto_source_dir}/ui/ui_err.c
    ${crypto_source_dir}/ui/ui_lib.c
    ${crypto_source_dir}/ui/ui_null.c
    ${crypto_source_dir}/ui/ui_util.c
    ${crypto_source_dir}/whrlpool/wp_dgst.c
    ${crypto_source_dir}/x509/by_dir.c
    ${crypto_source_dir}/x509/by_file.c
    ${crypto_source_dir}/x509/by_mem.c
    ${crypto_source_dir}/x509/x509_addr.c
    ${crypto_source_dir}/x509/x509_akey.c
    ${crypto_source_dir}/x509/x509_akeya.c
    ${crypto_source_dir}/x509/x509_alt.c
    ${crypto_source_dir}/x509/x509_asid.c
    ${crypto_source_dir}/x509/x509_att.c
    ${crypto_source_dir}/x509/x509_bcons.c
    ${crypto_source_dir}/x509/x509_bitst.c
    ${crypto_source_dir}/x509/x509_cmp.c
    ${crypto_source_dir}/x509/x509_conf.c
    ${crypto_source_dir}/x509/x509_constraints.c
    ${crypto_source_dir}/x509/x509_cpols.c
    ${crypto_source_dir}/x509/x509_crld.c
    ${crypto_source_dir}/x509/x509_d2.c
    ${crypto_source_dir}/x509/x509_def.c
    ${crypto_source_dir}/x509/x509_err.c
    ${crypto_source_dir}/x509/x509_ext.c
    ${crypto_source_dir}/x509/x509_extku.c
    ${crypto_source_dir}/x509/x509_genn.c
    ${crypto_source_dir}/x509/x509_ia5.c
    ${crypto_source_dir}/x509/x509_info.c
    ${crypto_source_dir}/x509/x509_int.c
    ${crypto_source_dir}/x509/x509_issuer_cache.c
    ${crypto_source_dir}/x509/x509_lib.c
    ${crypto_source_dir}/x509/x509_lu.c
    ${crypto_source_dir}/x509/x509_ncons.c
    ${crypto_source_dir}/x509/x509_obj.c
    ${crypto_source_dir}/x509/x509_ocsp.c
    ${crypto_source_dir}/x509/x509_pcons.c
    ${crypto_source_dir}/x509/x509_pku.c
    ${crypto_source_dir}/x509/x509_pmaps.c
    ${crypto_source_dir}/x509/x509_policy.c
    ${crypto_source_dir}/x509/x509_prn.c
    ${crypto_source_dir}/x509/x509_purp.c
    ${crypto_source_dir}/x509/x509_r2x.c
    ${crypto_source_dir}/x509/x509_req.c
    ${crypto_source_dir}/x509/x509_set.c
    ${crypto_source_dir}/x509/x509_skey.c
    ${crypto_source_dir}/x509/x509_trs.c
    ${crypto_source_dir}/x509/x509_txt.c
    ${crypto_source_dir}/x509/x509_utl.c
    ${crypto_source_dir}/x509/x509_v3.c
    ${crypto_source_dir}/x509/x509_verify.c
    ${crypto_source_dir}/x509/x509_vfy.c
    ${crypto_source_dir}/x509/x509_vpm.c
    ${crypto_source_dir}/x509/x509cset.c
    ${crypto_source_dir}/x509/x509name.c
    ${crypto_source_dir}/x509/x509rset.c
    ${crypto_source_dir}/x509/x509spki.c
    ${crypto_source_dir}/x509/x509type.c
    ${crypto_source_dir}/x509/x_all.c
)

if(WIN32)
    list(APPEND crypto_sources
        ${crypto_source_dir}/bio/b_win.c
        ${crypto_source_dir}/compat/crypto_lock_win.c
        ${crypto_source_dir}/ui/ui_openssl_win.c
    )
endif()

if(WIN32)
    list(APPEND crypto_sources
        ${crypto_source_dir}/compat/posix_win.c
    )
endif()

if(NOT HAVE_ASPRINTF)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/bsd-asprintf.c)
endif()

if(NOT HAVE_FREEZERO)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/freezero.c)
endif()

if(NOT HAVE_GETPAGESIZE)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/getpagesize.c)
endif()

if(NOT HAVE_INET_PTON)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/inet_pton.c)
endif()

if(NOT HAVE_REALLOCARRAY)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/reallocarray.c)
endif()

if(NOT HAVE_RECALLOCARRAY)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/recallocarray.c)
endif()

if(NOT HAVE_STRCASECMP)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strcasecmp.c)
endif()

if(NOT HAVE_STRLCAT)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strlcat.c)
endif()

if(NOT HAVE_STRLCPY)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strlcpy.c)
endif()

if(NOT HAVE_STRNDUP)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strndup.c)
    if(NOT HAVE_STRNLEN)
        list(APPEND crypto_sources ${crypto_source_dir}/compat/strnlen.c)
    endif()
endif()

if(NOT HAVE_STRSEP)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strsep.c)
endif()

if(NOT HAVE_STRTONUM)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/strtonum.c)
endif()

if(NOT HAVE_SYSLOG_R)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/syslog_r.c)
endif()

if(NOT HAVE_TIMEGM)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/timegm.c)
endif()

if(NOT HAVE_EXPLICIT_BZERO)
    if(WIN32)
        list(APPEND crypto_sources ${crypto_source_dir}/compat/explicit_bzero_win.c)
    endif()
endif()

if(NOT HAVE_ARC4RANDOM_BUF)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/arc4random.c)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/arc4random_uniform.c)
    if(NOT HAVE_GETENTROPY)
        if(WIN32)
            list(APPEND crypto_sources ${crypto_source_dir}/compat/getentropy_win.c)
        endif()
    endif()
endif()

if(NOT HAVE_TIMINGSAFE_BCMP)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/timingsafe_bcmp.c)
endif()

if(NOT HAVE_TIMINGSAFE_MEMCMP)
    list(APPEND crypto_sources ${crypto_source_dir}/compat/timingsafe_memcmp.c)
endif()

list(APPEND crypto_sources ${configured_headers})

add_subdirectory(shared)

if(WITH_STANDALONE_SETUP)
    add_subdirectory(static)
endif()
