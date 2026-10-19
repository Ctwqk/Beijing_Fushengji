find_package(GTest QUIET)
if(NOT GTest_FOUND)
  if(NOT FSJ_FETCH_DEPENDENCIES)
    message(FATAL_ERROR "GoogleTest not found (install libgtest-dev or enable FSJ_FETCH_DEPENDENCIES)")
  endif()
  message(STATUS "GoogleTest not found on the system, fetching it")
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz)
  FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

add_executable(fsj_tests
  test_main.cpp
  test_item_market.cpp
  test_player.cpp
  test_game.cpp
  test_random_event.cpp
  test_llm.cpp
  test_language.cpp)
target_link_libraries(fsj_tests PRIVATE fsj_core GTest::gtest)
fsj_enable_warnings(fsj_tests)
gtest_discover_tests(fsj_tests DISCOVERY_MODE PRE_TEST)

# End-to-end check of the libcurl client against a local mock OpenAI server.
if(FSJ_HAVE_CURL)
  find_package(Python3 COMPONENTS Interpreter)
  if(Python3_Interpreter_FOUND)
    add_executable(fsj_llm_http_test test_llm_http.cpp test_main.cpp)
    target_link_libraries(fsj_llm_http_test PRIVATE fsj_ai fsj_core GTest::gtest)
    fsj_enable_warnings(fsj_llm_http_test)
    add_test(NAME llm_http_mock_server
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_with_mock_server.py $<TARGET_FILE:fsj_llm_http_test>)
  endif()
endif()
