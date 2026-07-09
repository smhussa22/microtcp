add_executable(microtcp_tests
    TunDevice_test.cc
)
target_link_libraries(microtcp_tests
    PRIVATE microtcp_lib GTest::gtest_main
)

add_test(NAME microtcp_tests COMMAND microtcp_tests)
