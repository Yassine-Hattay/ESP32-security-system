# Check ESP-IDF version and error out if it is not in the supported range.
#
# Note for arduino-esp32 developers: to bypass the version check locally,
# set ARDUINO_SKIP_IDF_VERSION_CHECK environment variable to 1. For example:
#   export ARDUINO_SKIP_IDF_VERSION_CHECK=1
#   idf.py build

set(min_supported_idf_version "5.3.0")
set(max_supported_idf_version "5.3.99")
set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")

if ("${idf_version}" AND NOT "$ENV{ARDUINO_SKIP_IDF_VERSION_CHECK}")
  if (idf_version VERSION_LESS min_supported_idf_version)
    message(FATAL_ERROR "Arduino-esp32 can be used with ESP-IDF versions "
                        "between ${min_supported_idf_version} and ${max_supported_idf_version}, "
                        "but an older version is detected: ${idf_version}.")
  endif()
  if (idf_version VERSION_GREATER max_supported_idf_version)
    message(FATAL_ERROR "Arduino-esp32 can be used with ESP-IDF versions "
                        "between ${min_supported_idf_version} and ${max_supported_idf_version}, "
                        "but a newer version is detected: ${idf_version}.")
  endif()
endif()

set(CORE_SRCS
  cores/esp32/base64.cpp
  cores/esp32/cbuf.cpp
  cores/esp32/ColorFormat.c
  cores/esp32/chip-debug-report.cpp
  cores/esp32/esp32-hal-adc.c
  cores/esp32/esp32-hal-bt.c
  cores/esp32/esp32-hal-cpu.c
  cores/esp32/esp32-hal-dac.c
  cores/esp32/esp32-hal-gpio.c
  cores/esp32/esp32-hal-i2c.c
  cores/esp32/esp32-hal-i2c-slave.c
  cores/esp32/esp32-hal-ledc.c
  cores/esp32/esp32-hal-matrix.c
  cores/esp32/esp32-hal-misc.c
  cores/esp32/esp32-hal-periman.c
  cores/esp32/esp32-hal-psram.c
  cores/esp32/esp32-hal-rgb-led.c
  cores/esp32/esp32-hal-sigmadelta.c
  cores/esp32/esp32-hal-spi.c
  cores/esp32/esp32-hal-time.c
  cores/esp32/esp32-hal-timer.c
  cores/esp32/esp32-hal-tinyusb.c
  cores/esp32/esp32-hal-touch.c
  cores/esp32/esp32-hal-touch-ng.c
  cores/esp32/esp32-hal-uart.c
  cores/esp32/esp32-hal-rmt.c
  cores/esp32/Esp.cpp
  cores/esp32/freertos_stats.cpp
  cores/esp32/FunctionalInterrupt.cpp
  cores/esp32/HardwareSerial.cpp
  cores/esp32/HEXBuilder.cpp
  cores/esp32/IPAddress.cpp
  cores/esp32/libb64/cdecode.c
  cores/esp32/libb64/cencode.c
  cores/esp32/MacAddress.cpp
  cores/esp32/main.cpp
  cores/esp32/MD5Builder.cpp
  cores/esp32/Print.cpp
  cores/esp32/SHA1Builder.cpp
  cores/esp32/stdlib_noniso.c
  cores/esp32/Stream.cpp
  cores/esp32/StreamString.cpp
  cores/esp32/Tone.cpp
  cores/esp32/HWCDC.cpp
  cores/esp32/USB.cpp
  cores/esp32/USBCDC.cpp
  cores/esp32/USBMSC.cpp
  cores/esp32/FirmwareMSC.cpp
  cores/esp32/firmware_msc_fat.c
  cores/esp32/wiring_pulse.c
  cores/esp32/wiring_shift.c
  cores/esp32/WMath.cpp
  cores/esp32/WString.cpp
  )

  set(LIBRARY_SRCS
  libraries/Time/DateStrings.cpp
  libraries/Time/Time.cpp
  libraries/NTPClient/NTPClient.cpp
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_dec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_big_enc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_common.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_dec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct64_enc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_dec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_ct_enc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_pwr8.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_pwr8_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_pwr8_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_pwr8_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_pwr8_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_dec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_small_enc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_x86ni.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_x86ni_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_x86ni_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_x86ni_ctr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/aes_x86ni_ctrcbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/asn1enc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ccm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ccopy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/chacha20_ct.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/chacha20_sse2.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec16be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec16le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec32be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec32le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec64be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dec64le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_ct.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_ct_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_ct_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_support.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_tab.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_tab_cbcdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/des_tab_cbcenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dig_oid.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/dig_size.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/eax.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_atr.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_default_sign_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_default_sign_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_default_vrfy_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_default_vrfy_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i15_bits.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i15_sign_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i15_sign_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i15_vrfy_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i15_vrfy_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i31_bits.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i31_sign_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i31_sign_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i31_vrfy_asn1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_i31_vrfy_raw.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ecdsa_rta.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_all_m15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_all_m31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_i15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_i31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_m15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_m31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_m62.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_c25519_m64.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_curve25519.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_default.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_keygen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_p256_m15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_p256_m31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_p256_m62.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_p256_m64.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_prime_i15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_prime_i31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_pubkey.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_secp256r1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_secp384r1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ec_secp521r1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc16be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc16le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc32be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc32le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc64be.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/enc64le.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/encode_ec_pk8der.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/encode_ec_rawder.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/encode_rsa_pk8der.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/encode_rsa_rawder.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/gcm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ghash_ctmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ghash_ctmul32.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ghash_ctmul64.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ghash_pclmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ghash_pwr8.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/hkdf.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/hmac.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/hmac_ct.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/hmac_drbg.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_add.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_bitlen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_decmod.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_decode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_decred.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_encode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_fmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_iszero.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_moddiv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_modpow.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_modpow2.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_montmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_mulacc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_muladd.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_ninv15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_reduce.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_rshift.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_sub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i15_tmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_add.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_bitlen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_decmod.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_decode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_decred.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_encode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_fmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_iszero.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_moddiv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_modpow.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_modpow2.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_montmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_mulacc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_muladd.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_ninv31.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_reduce.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_rshift.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_sub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i31_tmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_add.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_bitlen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_decmod.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_decode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_decred.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_div32.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_encode.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_fmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_iszero.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_modpow.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_montmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_mulacc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_muladd.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_ninv32.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_reduce.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_sub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i32_tmont.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/i62_modpow2.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/md5.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/md5sha1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/mgf1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/multihash.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/pemdec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/pemenc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/pkey_decoder.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/poly1305_ctmul.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/poly1305_ctmul32.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/poly1305_ctmulq.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/poly1305_i15.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/prf.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/prf_md5sha1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/prf_sha256.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/prf_sha384.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_keygen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_modulus.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_oaep_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_oaep_encrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pkcs1_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pkcs1_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_priv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_privexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pss_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pss_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_default_pubexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_keygen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_modulus.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_oaep_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_oaep_encrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pkcs1_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pkcs1_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_priv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_privexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pss_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pss_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i15_pubexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_keygen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_keygen_inner.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_modulus.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_oaep_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_oaep_encrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pkcs1_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pkcs1_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_priv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_privexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pss_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pss_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i31_pubexp.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_oaep_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_oaep_encrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_pkcs1_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_pkcs1_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_priv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_pss_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_pss_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i32_pub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_keygen.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_oaep_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_oaep_encrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_pkcs1_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_pkcs1_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_priv.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_pss_sign.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_pss_vrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_i62_pub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_oaep_pad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_oaep_unpad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_pkcs1_sig_pad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_pkcs1_sig_unpad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_pss_sig_pad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_pss_sig_unpad.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/rsa_ssl_decrypt.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/settings.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/sha1.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/sha2big.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/sha2small.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/shake.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/skey_decoder.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_ccert_single_ec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_ccert_single_rsa.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_client.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_client_default_rsapub.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_client_full.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_aescbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_aesccm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_aesgcm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_chapol.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_descbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_ec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_ecdsa.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_engine_default_rsavrfy.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_hashes.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_hs_client.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_hs_server.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_io.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_keyexport.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_lru.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_rec_cbc.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_rec_ccm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_rec_chapol.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_rec_gcm.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_scert_single_ec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_scert_single_rsa.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_full_ec.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_full_rsa.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_mine2c.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_mine2g.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_minf2c.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_minf2g.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_minr2g.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_minu2g.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/ssl_server_minv2g.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/sysrng.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/x509_decoder.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/x509_knownkey.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/x509_minimal.c
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/x509_minimal_full.c
  libraries/ESP-Mail-Client/src/client/SSLClient/client/BSSL_CertStore.cpp
  libraries/ESP-Mail-Client/src/client/SSLClient/client/BSSL_Helper.cpp
  libraries/ESP-Mail-Client/src/client/SSLClient/client/BSSL_SSL_Client.cpp
  libraries/ESP-Mail-Client/src/client/SSLClient/client/BSSL_TCP_Client.cpp
  libraries/ESP-Mail-Client/src/client/SSLClient/client/stm32_time_fix.c
  libraries/ESP-Mail-Client/src/ESP_Mail_Client.cpp
  libraries/ESP-Mail-Client/src/extras/RFC2047.cpp
  )

set(ARDUINO_ALL_LIBRARIES
  ArduinoOTA
  AsyncUDP
  BLE
  BluetoothSerial
  DNSServer
  EEPROM
  ESP_I2S
  ESP_NOW
  ESP_SR
  ESPmDNS
  Ethernet
  FFat
  FS
  HTTPClient
  HTTPUpdate
  Insights
  LittleFS
  Matter
  NetBIOS
  Network
  OpenThread
  PPP
  Preferences
  RainMaker
  SD_MMC
  SD
  SimpleBLE
  SPIFFS
  SPI
  Ticker
  Update
  USB
  WebServer
  NetworkClientSecure
  WiFi
  WiFiProv
  Wire
  Zigbee
  )

set(ARDUINO_LIBRARY_ArduinoOTA_SRCS libraries/ArduinoOTA/src/ArduinoOTA.cpp)

set(ARDUINO_LIBRARY_AsyncUDP_SRCS libraries/AsyncUDP/src/AsyncUDP.cpp)

set(ARDUINO_LIBRARY_BluetoothSerial_SRCS
  libraries/BluetoothSerial/src/BluetoothSerial.cpp
  libraries/BluetoothSerial/src/BTAddress.cpp
  libraries/BluetoothSerial/src/BTAdvertisedDeviceSet.cpp
  libraries/BluetoothSerial/src/BTScanResultsSet.cpp)

set(ARDUINO_LIBRARY_DNSServer_SRCS libraries/DNSServer/src/DNSServer.cpp)

set(ARDUINO_LIBRARY_EEPROM_SRCS libraries/EEPROM/src/EEPROM.cpp)

set(ARDUINO_LIBRARY_ESP_I2S_SRCS libraries/ESP_I2S/src/ESP_I2S.cpp)

set(ARDUINO_LIBRARY_ESP_NOW_SRCS
  libraries/ESP_NOW/src/ESP32_NOW.cpp
  libraries/ESP_NOW/src/ESP32_NOW_Serial.cpp)

set(ARDUINO_LIBRARY_ESP_SR_SRCS
  libraries/ESP_SR/src/ESP_SR.cpp
  libraries/ESP_SR/src/esp32-hal-sr.c)

set(ARDUINO_LIBRARY_ESPmDNS_SRCS libraries/ESPmDNS/src/ESPmDNS.cpp)

set(ARDUINO_LIBRARY_Ethernet_SRCS libraries/Ethernet/src/ETH.cpp)

set(ARDUINO_LIBRARY_FFat_SRCS libraries/FFat/src/FFat.cpp)

set(ARDUINO_LIBRARY_FS_SRCS
  libraries/FS/src/FS.cpp
  libraries/FS/src/vfs_api.cpp)

set(ARDUINO_LIBRARY_HTTPClient_SRCS libraries/HTTPClient/src/HTTPClient.cpp)

set(ARDUINO_LIBRARY_HTTPUpdate_SRCS libraries/HTTPUpdate/src/HTTPUpdate.cpp)

set(ARDUINO_LIBRARY_Insights_SRCS libraries/Insights/src/Insights.cpp)

set(ARDUINO_LIBRARY_LittleFS_SRCS libraries/LittleFS/src/LittleFS.cpp)

set(ARDUINO_LIBRARY_NetBIOS_SRCS libraries/NetBIOS/src/NetBIOS.cpp)

set(ARDUINO_LIBRARY_OpenThread_SRCS
  libraries/OpenThread/src/OThreadCLI.cpp
  libraries/OpenThread/src/OThreadCLI_Util.cpp)

set(ARDUINO_LIBRARY_Matter_SRCS
  libraries/Matter/src/MatterEndpoints/MatterGenericSwitch.cpp
  libraries/Matter/src/MatterEndpoints/MatterOnOffLight.cpp
  libraries/Matter/src/MatterEndpoints/MatterDimmableLight.cpp
  libraries/Matter/src/MatterEndpoints/MatterColorTemperatureLight.cpp
  libraries/Matter/src/MatterEndpoints/MatterColorLight.cpp
  libraries/Matter/src/MatterEndpoints/MatterEnhancedColorLight.cpp
  libraries/Matter/src/MatterEndpoints/MatterFan.cpp
  libraries/Matter/src/MatterEndpoints/MatterTemperatureSensor.cpp
  libraries/Matter/src/MatterEndpoints/MatterHumiditySensor.cpp
  libraries/Matter/src/MatterEndpoints/MatterContactSensor.cpp
  libraries/Matter/src/MatterEndpoints/MatterPressureSensor.cpp
  libraries/Matter/src/MatterEndpoints/MatterOccupancySensor.cpp
  libraries/Matter/src/MatterEndpoints/MatterOnOffPlugin.cpp
  libraries/Matter/src/MatterEndpoints/MatterThermostat.cpp
  libraries/Matter/src/Matter.cpp)

set(ARDUINO_LIBRARY_PPP_SRCS
  libraries/PPP/src/PPP.cpp
  libraries/PPP/src/ppp.c)

set(ARDUINO_LIBRARY_Preferences_SRCS libraries/Preferences/src/Preferences.cpp)

set(ARDUINO_LIBRARY_RainMaker_SRCS
  libraries/RainMaker/src/RMaker.cpp
  libraries/RainMaker/src/RMakerNode.cpp
  libraries/RainMaker/src/RMakerParam.cpp
  libraries/RainMaker/src/RMakerDevice.cpp
  libraries/RainMaker/src/RMakerType.cpp
  libraries/RainMaker/src/RMakerQR.cpp
  libraries/RainMaker/src/RMakerUtils.cpp
  libraries/RainMaker/src/AppInsights.cpp)

set(ARDUINO_LIBRARY_SD_MMC_SRCS libraries/SD_MMC/src/SD_MMC.cpp)

set(ARDUINO_LIBRARY_SD_SRCS
  libraries/SD/src/SD.cpp
  libraries/SD/src/sd_diskio.cpp
  libraries/SD/src/sd_diskio_crc.c)

set(ARDUINO_LIBRARY_SimpleBLE_SRCS libraries/SimpleBLE/src/SimpleBLE.cpp)

set(ARDUINO_LIBRARY_SPIFFS_SRCS libraries/SPIFFS/src/SPIFFS.cpp)

set(ARDUINO_LIBRARY_SPI_SRCS libraries/SPI/src/SPI.cpp)

set(ARDUINO_LIBRARY_Ticker_SRCS libraries/Ticker/src/Ticker.cpp)

set(ARDUINO_LIBRARY_Update_SRCS
  libraries/Update/src/Updater.cpp
  libraries/Update/src/HttpsOTAUpdate.cpp)

set(ARDUINO_LIBRARY_USB_SRCS
  libraries/USB/src/USBHID.cpp
  libraries/USB/src/USBMIDI.cpp
  libraries/USB/src/USBHIDMouse.cpp
  libraries/USB/src/USBHIDKeyboard.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_da_DK.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_de_DE.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_en_US.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_es_ES.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_fr_FR.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_hu_HU.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_it_IT.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_pt_BR.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_pt_PT.cpp
  libraries/USB/src/keyboardLayout/KeyboardLayout_sv_SE.cpp
  libraries/USB/src/USBHIDGamepad.cpp
  libraries/USB/src/USBHIDConsumerControl.cpp
  libraries/USB/src/USBHIDSystemControl.cpp
  libraries/USB/src/USBHIDVendor.cpp
  libraries/USB/src/USBVendor.cpp)

set(ARDUINO_LIBRARY_WebServer_SRCS
  libraries/WebServer/src/WebServer.cpp
  libraries/WebServer/src/Parsing.cpp
  libraries/WebServer/src/detail/mimetable.cpp
  libraries/WebServer/src/middleware/MiddlewareChain.cpp
  libraries/WebServer/src/middleware/AuthenticationMiddleware.cpp
  libraries/WebServer/src/middleware/CorsMiddleware.cpp
  libraries/WebServer/src/middleware/LoggingMiddleware.cpp)

set(ARDUINO_LIBRARY_NetworkClientSecure_SRCS
  libraries/NetworkClientSecure/src/ssl_client.cpp
  libraries/NetworkClientSecure/src/NetworkClientSecure.cpp)

set(ARDUINO_LIBRARY_Network_SRCS
  libraries/Network/src/NetworkInterface.cpp
  libraries/Network/src/NetworkEvents.cpp
  libraries/Network/src/NetworkManager.cpp
  libraries/Network/src/NetworkClient.cpp
  libraries/Network/src/NetworkServer.cpp
  libraries/Network/src/NetworkUdp.cpp)

set(ARDUINO_LIBRARY_WiFi_SRCS
  libraries/WiFi/src/WiFiAP.cpp
  libraries/WiFi/src/WiFi.cpp
  libraries/WiFi/src/WiFiGeneric.cpp
  libraries/WiFi/src/WiFiMulti.cpp
  libraries/WiFi/src/WiFiScan.cpp
  libraries/WiFi/src/WiFiSTA.cpp
  libraries/WiFi/src/STA.cpp
  libraries/WiFi/src/AP.cpp)

set(ARDUINO_LIBRARY_WiFiProv_SRCS libraries/WiFiProv/src/WiFiProv.cpp)

set(ARDUINO_LIBRARY_Wire_SRCS libraries/Wire/src/Wire.cpp)

set(ARDUINO_LIBRARY_Zigbee_SRCS
  libraries/Zigbee/src/ZigbeeCore.cpp
  libraries/Zigbee/src/ZigbeeEP.cpp
  libraries/Zigbee/src/ZigbeeHandlers.cpp
  libraries/Zigbee/src/ep/ZigbeeColorDimmableLight.cpp
  libraries/Zigbee/src/ep/ZigbeeColorDimmerSwitch.cpp
  libraries/Zigbee/src/ep/ZigbeeDimmableLight.cpp
  libraries/Zigbee/src/ep/ZigbeeLight.cpp
  libraries/Zigbee/src/ep/ZigbeeSwitch.cpp
  libraries/Zigbee/src/ep/ZigbeeTempSensor.cpp
  libraries/Zigbee/src/ep/ZigbeeThermostat.cpp
  libraries/Zigbee/src/ep/ZigbeeFlowSensor.cpp
  libraries/Zigbee/src/ep/ZigbeePressureSensor.cpp
  libraries/Zigbee/src/ep/ZigbeeOccupancySensor.cpp
  libraries/Zigbee/src/ep/ZigbeeCarbonDioxideSensor.cpp
  )

set(ARDUINO_LIBRARY_BLE_SRCS
  libraries/BLE/src/BLE2901.cpp
  libraries/BLE/src/BLE2902.cpp
  libraries/BLE/src/BLE2904.cpp
  libraries/BLE/src/BLEAddress.cpp
  libraries/BLE/src/BLEAdvertisedDevice.cpp
  libraries/BLE/src/BLEAdvertising.cpp
  libraries/BLE/src/BLEBeacon.cpp
  libraries/BLE/src/BLECharacteristic.cpp
  libraries/BLE/src/BLECharacteristicMap.cpp
  libraries/BLE/src/BLEClient.cpp
  libraries/BLE/src/BLEDescriptor.cpp
  libraries/BLE/src/BLEDescriptorMap.cpp
  libraries/BLE/src/BLEDevice.cpp
  libraries/BLE/src/BLEEddystoneTLM.cpp
  libraries/BLE/src/BLEEddystoneURL.cpp
  libraries/BLE/src/BLEExceptions.cpp
  libraries/BLE/src/BLEHIDDevice.cpp
  libraries/BLE/src/BLERemoteCharacteristic.cpp
  libraries/BLE/src/BLERemoteDescriptor.cpp
  libraries/BLE/src/BLERemoteService.cpp
  libraries/BLE/src/BLEScan.cpp
  libraries/BLE/src/BLESecurity.cpp
  libraries/BLE/src/BLEServer.cpp
  libraries/BLE/src/BLEService.cpp
  libraries/BLE/src/BLEServiceMap.cpp
  libraries/BLE/src/BLEUtils.cpp
  libraries/BLE/src/BLEUUID.cpp
  libraries/BLE/src/BLEValue.cpp
  libraries/BLE/src/FreeRTOS.cpp
  libraries/BLE/src/GeneralUtils.cpp
  )

set(ARDUINO_LIBRARIES_SRCS)
set(ARDUINO_LIBRARIES_REQUIRES)
set(ARDUINO_LIBRARIES_INCLUDEDIRS)
foreach(libname IN LISTS ARDUINO_ALL_LIBRARIES)
  if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_${libname})
    if(ARDUINO_LIBRARY_${libname}_SRCS)
      list(APPEND ARDUINO_LIBRARIES_SRCS ${ARDUINO_LIBRARY_${libname}_SRCS})
    endif()
    if(ARDUINO_LIBRARY_${libname}_REQUIRES)
      list(APPEND ARDUINO_LIBRARIES_REQUIRES ${ARDUINO_LIBRARY_${libname}_REQUIRES})
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libraries/${libname}/src)
      list(APPEND ARDUINO_LIBRARIES_INCLUDEDIRS libraries/${libname}/src)
    endif()
  endif()
endforeach()

set(includedirs variants/${CONFIG_ARDUINO_VARIANT}/ cores/esp32/ ${ARDUINO_LIBRARIES_INCLUDEDIRS}
  libraries/Time/
  libraries/NTPClient/
  libraries/ESP-Mail-Client/src/client/SSLClient/bssl/
  libraries/ESP-Mail-Client/src/client/SSLClient/client/
  libraries/ESP-Mail-Client/src/
  libraries/ESP-Mail-Client/src/extras/
  )

set(srcs ${CORE_SRCS} ${ARDUINO_LIBRARIES_SRCS})
set(priv_includes cores/esp32/libb64)
set(requires spi_flash esp_partition mbedtls wpa_supplicant esp_adc esp_eth http_parser esp_ringbuf esp_driver_gptimer esp_driver_usb_serial_jtag driver)
set(priv_requires fatfs nvs_flash app_update spiffs bootloader_support bt esp_hid usb esp_psram ${ARDUINO_LIBRARIES_REQUIRES})

if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_OpenThread)
  #if(CONFIG_SOC_IEEE802154_SUPPORTED) # Does not work!
  #if(CONFIG_OPENTHREAD_ENABLED) # Does not work!
  if(IDF_TARGET STREQUAL "esp32c6" OR IDF_TARGET STREQUAL "esp32h2") # Sadly only this works
    list(APPEND requires openthread)
  endif()
endif()

if(IDF_TARGET STREQUAL "esp32p4")
  list(APPEND requires esp_driver_touch_sens)
endif()

idf_component_register(INCLUDE_DIRS ${includedirs} PRIV_INCLUDE_DIRS ${priv_includes} SRCS ${srcs} ${LIBRARY_SRCS} REQUIRES ${requires} PRIV_REQUIRES ${priv_requires})

if(NOT CONFIG_FREERTOS_HZ EQUAL 1000 AND NOT "$ENV{ARDUINO_SKIP_TICK_CHECK}")
    # See delay() in cores/esp32/esp32-hal-misc.c.
    message(FATAL_ERROR "esp32-arduino requires CONFIG_FREERTOS_HZ=1000 "
                        "(currently ${CONFIG_FREERTOS_HZ})")
endif()

string(TOUPPER ${CONFIG_ARDUINO_VARIANT} idf_target_caps)
string(REPLACE "-" "_" idf_target_for_macro "${idf_target_caps}")
target_compile_options(${COMPONENT_TARGET} PUBLIC
    -DARDUINO=10812
    -DARDUINO_${idf_target_for_macro}_DEV
    -DARDUINO_ARCH_ESP32
    -DARDUINO_BOARD="${idf_target_caps}_DEV"
    -DARDUINO_VARIANT="${CONFIG_ARDUINO_VARIANT}"
    -DESP32=ESP32)

if(CONFIG_AUTOSTART_ARDUINO)
    # in autostart mode, arduino-esp32 contains app_main() function and needs to
    # reference setup() and loop() in the main component. If we add main
    # component to priv_requires then we create a large circular dependency
    # (arduino-esp32 -> main -> arduino-esp32) and can get linker errors, so
    # instead we add setup() and loop() to the undefined symbols list so the
    # linker will always include them.
    #
    # (As they are C++ symbol, we need to add the C++ mangled names.)
    target_link_libraries(${COMPONENT_LIB} INTERFACE "-u _Z5setupv -u _Z4loopv")
endif()

# This function adds a dependency on the given component if the component is included into the build.
function(maybe_add_component component_name)
    idf_build_get_property(components BUILD_COMPONENTS)
    if (${component_name} IN_LIST components)
        idf_component_get_property(lib_name ${component_name} COMPONENT_LIB)
        target_link_libraries(${COMPONENT_LIB} PUBLIC ${lib_name})
    endif()
endfunction()

if(IDF_TARGET MATCHES "esp32s2|esp32s3|esp32p4" AND CONFIG_TINYUSB_ENABLED)
    maybe_add_component(arduino_tinyusb)
endif()
if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_ArduinoOTA)
    maybe_add_component(esp_https_ota)
endif()
if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_ESP_SR)
    maybe_add_component(espressif__esp_sr)
endif()
if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_Matter)
    maybe_add_component(espressif__esp_matter)
endif()
if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_LittleFS)
    maybe_add_component(joltwallet__littlefs)
endif()
if(NOT CONFIG_ARDUINO_SELECTIVE_COMPILATION OR CONFIG_ARDUINO_SELECTIVE_WiFiProv)
    maybe_add_component(espressif__network_provisioning)
endif()
