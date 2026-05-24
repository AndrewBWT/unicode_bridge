# Guard against rebuilding by accident.
if(NOT DEFINED UNICODE_BRIDGE_TESTS_INCLUDED)
  set(UNICODE_BRIDGE_TESTS_INCLUDED TRUE)
  # Set library name so that, if we change it, we only have to change this line
  # Set the name of the example executable once
  set(UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME UNICODE_BRIDGE_tests)
  
  set(UNICODE_BRIDGE_TEST_RUN_ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/data/AUNICODE_BRIDGE_test_config"
    CACHE STRING "Optional runtime args for UNICODE_BRIDGE_test executable")

  # The set of all test files. The idea behind these is they mirror the files in the library.
  # However we may change this as we move forward.

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1  # pin to a specific release
  )
  FetchContent_MakeAvailable(Catch2)
  # These are the libs required for the testing suite.
  set(UNICODE_BRIDGE_TESTS_REQUIRED_LIBS
    Catch2::Catch2
    unicode_bridge
  )

  #There are some additional entries at the end which are not mirrored by files in the library.
  set(UNICODE_BRIDGE_TESTS_FILES
    src/main.cpp
  )
  
  # Sets how the IDE organises the files.
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. FILES
    ${UNICODE_BRIDGE_TESTS_FILES}
  )

  # Add the set of files to executable.
  add_executable(${UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME} ${UNICODE_BRIDGE_TESTS_FILES})
  
  set_common_compile_options(${UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME})
  
  # Add the include directory to the target.
  target_include_directories(${UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME} PRIVATE include)
  # Link the REQUIRED_LIBS to the LIBRARY.
  target_link_libraries(${UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME} PUBLIC ${UNICODE_BRIDGE_TESTS_REQUIRED_LIBS})
  # Ensure that the IDE sees the files in the file structure they are organised in.
  
  # Adds the test as a command.
  add_test(NAME "Run abc_print test suite executable"
           COMMAND ${UNICODE_BRIDGE_TESTS_EXECUTABLE_NAME} "--config" ${UNICODE_BRIDGE_TEST_RUN_ARGS})
endif()