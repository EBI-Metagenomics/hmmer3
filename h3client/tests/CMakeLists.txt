set(BINDIR ${CMAKE_CURRENT_BINARY_DIR})
set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})

set(ASSETS ${BINDIR}/assets)
set(TMPDIR ${BINDIR}/tmp)

file(MAKE_DIRECTORY ${ASSETS})
file(MAKE_DIRECTORY ${TMPDIR})

add_compile_definitions(ASSETS=\"${ASSETS}\")
add_compile_definitions(TMPDIR=\"${TMPDIR}\")

file(COPY assets/ross.fasta DESTINATION ${ASSETS})
file(COPY assets/ross.poor.fasta DESTINATION ${ASSETS})
file(COPY assets/ross.5.hmm DESTINATION ${ASSETS})

add_test(NAME start_daemon COMMAND ${SRCDIR}/start_daemon.sh ${ASSETS}/ross.5.hmm)
add_test(NAME stop_daemon COMMAND ${SRCDIR}/stop_daemon.sh ${ASSETS}/ross.5.hmm)

add_executable(single single.c file_hash.c helper.c fs.c)
target_link_libraries(single PRIVATE H3CLIENT::h3client)
add_test(NAME single COMMAND single)

add_executable(multi multi.c file_hash.c helper.c fs.c)
target_link_libraries(multi PRIVATE H3CLIENT::h3client)
add_test(NAME multi COMMAND multi)

add_executable(corrupt corrupt.c file_hash.c helper.c fs.c)
target_link_libraries(corrupt PRIVATE H3CLIENT::h3client)
add_test(NAME corrupt COMMAND corrupt)

set_tests_properties(single PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(multi PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(corrupt PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(start_daemon PROPERTIES FIXTURES_SETUP daemon)
set_tests_properties(stop_daemon PROPERTIES FIXTURES_CLEANUP daemon)
