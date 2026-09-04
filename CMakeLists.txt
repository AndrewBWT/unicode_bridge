function(set_common_compile_options TARGET)
  target_compile_features(${TARGET} PUBLIC cxx_std_23)
  
  target_compile_options(${TARGET} PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
  )
  if(MSVC)
#    target_compile_options(${TARGET} PRIVATE /W4 /permissive- /bigobj /utf-8)
  else()
#    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
  endif()
  set_target_properties(${TARGET} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

# Current CMake version. We have not tested with other versions.
cmake_minimum_required(VERSION 3.28)

# Use FetchContent to get libraries etc.
include(FetchContent)

# Set up the project information
project(
  unicode_bridge
  VERSION 			0.0.1
  DESCRIPTION 		"A simple library for transforming and validating unicode strings."
  LANGUAGES 		CXX
)

# Set up the variables TOP_LEVEL_PROJECT and RUNNING_TESTS

# UNICODE_BRIDGE_TOP_LEVEL_PROJECT informs the user if this is the top level project.
# If it is the top level project, we assume that it is being built on its own.
set(UNICODE_BRIDGE_TOP_LEVEL_PROJECT FALSE)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(UNICODE_BRIDGE_TOP_LEVEL_PROJECT TRUE)
endif()

# We include this here so that the variable BUILD_TESTING is now defined as TRUE (due to its inclusion by CTest).
# Remember, the user can set BUILD_TESTING themselves if they so wish, include(CTest) won't override that value.
if(UNICODE_BRIDGE_TOP_LEVEL_PROJECT)
  include(CTest)
endif()

# Ensures that BUILD_TESTING is defined as FALSE. If LSPS_TOP_LEVEL_PROJECT == FALSE, it would be an undefined value.
if(NOT DEFINED BUILD_TESTING)
  set(BUILD_TESTING OFF)
endif()

# Sets up the UNICODE_BRIDGE_RUNNING_TESTS variable.
set(UNICODE_BRIDGE_RUNNING_TESTS FALSE)
if(UNICODE_BRIDGE_TOP_LEVEL_PROJECT AND BUILD_TESTING)
  set(UNICODE_BRIDGE_RUNNING_TESTS TRUE)
endif()

# Set up the options for the user.
option(UNICODE_BRIDGE_ENABLE_DOCS "Build Unicode bridge documentation" ${UNICODE_BRIDGE_TOP_LEVEL_PROJECT})
option(UNICODE_BRIDGE_ENABLE_TESTS "Build Unicode bridge tests" ${UNICODE_BRIDGE_RUNNING_TESTS})
option(UNICODE_BRIDGE_ENABLE_EXAMPLES "Build Unicode bridge examples" ${UNICODE_BRIDGE_TOP_LEVEL_PROJECT})

# Show the user the status of the core variables.
message(STATUS "Top level project: ${UNICODE_BRIDGE_TOP_LEVEL_PROJECT}")
message(STATUS "Building tests: ${UNICODE_BRIDGE_ENABLE_TESTS}")
message(STATUS "Building docs: ${UNICODE_BRIDGE_ENABLE_DOCS}")
message(STATUS "Building examples: ${UNICODE_BRIDGE_ENABLE_EXAMPLES}")
  
# Add the library folder.
add_subdirectory(library)

# Enable documentation if required.
#if(UNICODE_BRIDGE_ENABLE_DOCS)
#  find_package(Doxygen)
#  if(Doxygen_FOUND)
#    add_subdirectory(docs)
#  else()
#    message(STATUS "Doxygen not found, skipping documentation creation.")
#  endif()
#endif()

# Enable tests if required
if(UNICODE_BRIDGE_ENABLE_TESTS)
  add_subdirectory(tests)
endif()

# Enable examples if required
if(UNICODE_BRIDGE_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()
