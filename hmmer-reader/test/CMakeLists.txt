function(download_files dst)
  set(url "https://uk1s3.embassy.ebi.ac.uk/deciphon")

  file(DOWNLOAD ${url}/three-profs.hmm ${dst}/three-profs.hmm
       EXPECTED_HASH MD5=326b9aa04c6f6213e475d5174cc3746c)

  file(DOWNLOAD ${url}/empty.hmm ${dst}/empty.hmm
       EXPECTED_HASH MD5=d41d8cd98f00b204e9800998ecf8427e)

  file(DOWNLOAD ${url}/corrupted1.hmm ${dst}/corrupted1.hmm
       EXPECTED_HASH MD5=df1ad566e561f92385c2d60946035eb9)

  file(DOWNLOAD ${url}/corrupted2.hmm ${dst}/corrupted2.hmm
       EXPECTED_HASH MD5=c4d5cbac2aa65a76ba3dcd9bf57b7ed2)

  file(DOWNLOAD ${url}/corrupted3.hmm ${dst}/corrupted3.hmm
       EXPECTED_HASH MD5=0a2b5db84ef6a5d40493226a914cde15)

  file(DOWNLOAD ${url}/corrupted4.hmm ${dst}/corrupted4.hmm
       EXPECTED_HASH MD5=79730bd582b6f3ce6e1ac10946c05768)

  file(DOWNLOAD ${url}/corrupted5.hmm ${dst}/corrupted5.hmm
       EXPECTED_HASH MD5=214e9f50e207150a85079c0fa2facd56)

  file(DOWNLOAD ${url}/corrupted6.hmm ${dst}/corrupted6.hmm
       EXPECTED_HASH MD5=333a6b78b54c856f0ac32845901fa617)

  file(DOWNLOAD ${url}/corrupted7.hmm ${dst}/corrupted7.hmm
       EXPECTED_HASH MD5=94eb1d8922400b15e2c751b370011f2c)

  file(DOWNLOAD ${url}/corrupted8.hmm ${dst}/corrupted8.hmm
       EXPECTED_HASH MD5=1a17560d9c4808e0d84f95dfa41e671b)
endfunction()

set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${ASSETS})

download_files("${ASSETS}")

function(hmr_add_test name srcs)
  add_executable(${name} ${srcs})
  target_link_libraries(${name} PRIVATE HMMER_READER::hmmer_reader)
  target_compile_options(${name} PRIVATE ${WARNING_FLAGS})
  add_test(NAME ${name} COMMAND ${name})

  set(TMPDIR "${CMAKE_CURRENT_BINARY_DIR}/${name}.tmp")
  add_custom_command(
    TARGET ${name}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMPDIR})
  target_compile_definitions(${name} PUBLIC "ASSETS=\"${ASSETS}\"")
  target_compile_definitions(${name} PUBLIC "TMPDIR=\"${TMPDIR}\"")
endfunction()

hmr_add_test(read_hmm read_hmm.c)
