set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
file(MAKE_DIRECTORY ${ASSETS})
file(COPY three-profs.hmm DESTINATION ${ASSETS})
file(COPY empty.hmm DESTINATION ${ASSETS})
file(COPY corrupted1.hmm DESTINATION ${ASSETS})
file(COPY corrupted2.hmm DESTINATION ${ASSETS})
file(COPY corrupted3.hmm DESTINATION ${ASSETS})
file(COPY corrupted4.hmm DESTINATION ${ASSETS})
file(COPY corrupted5.hmm DESTINATION ${ASSETS})
file(COPY corrupted6.hmm DESTINATION ${ASSETS})
file(COPY corrupted7.hmm DESTINATION ${ASSETS})
file(COPY corrupted8.hmm DESTINATION ${ASSETS})
file(COPY noacc.hmm DESTINATION ${ASSETS})

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
