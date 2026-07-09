# Guard against rebuilding by accident.
if(NOT DEFINED UNICODE_BRIDGE_EXAMPLES_INCLUDED)
  set(UNICODE_BRIDGE_EXAMPLES_INCLUDED TRUE)
  # Set library name so that, if we change it, we only have to change this line
  # Set the name of the example executable once
  set(UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME unicode_bridge_examples)
  # These are the libs required for the examples executable.
  set(UNICODE_BRIDGE_EXAMPLES_REQUIRED_LIBS
    unicode_bridge
  )

  #There are some additional entries at the end which are not mirrored by files in the library.
  set(UNICODE_BRIDGE_EXAMPLES_FILES
    src/main.cpp
  )
  
  # Sets how the IDE organises the files.
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. FILES
    ${UNICODE_BRIDGE_EXAMPLES_FILES}
  )

  # Add the set of files to executable.
  add_executable(${UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME} ${UNICODE_BRIDGE_EXAMPLES_FILES})
  
  set_common_compile_options(${UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME})
  
  # Add the include directory to the target.
  target_include_directories(${UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME} PRIVATE include)
  # Link the REQUIRED_LIBS to the LIBRARY.
  target_link_libraries(${UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME} PUBLIC ${UNICODE_BRIDGE_EXAMPLES_REQUIRED_LIBS})
  # Ensure that the IDE sees the files in the file structure they are organised in.
  
  # Adds the test as a command.
  add_test(NAME "Run examples test suite executable"
           COMMAND ${UNICODE_BRIDGE_EXAMPLES_EXECUTABLE_NAME})
endif()