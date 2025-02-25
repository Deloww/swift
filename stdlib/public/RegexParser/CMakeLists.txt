#===--- CMakeLists.txt - Pattern matching engine support library -----------===#
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2021 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://swift.org/LICENSE.txt for license information
# See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
#
#===------------------------------------------------------------------------===#

set(swift_matching_engine_link_libraries
  swiftCore)

file(GLOB_RECURSE _MATCHING_ENGINE_SOURCES
  "${EXPERIMENTAL_STRING_PROCESSING_SOURCE_DIR}/Sources/_RegexParser/*.swift")
set(MATCHING_ENGINE_SOURCES)
foreach(source ${_MATCHING_ENGINE_SOURCES})
  file(TO_CMAKE_PATH "${source}" source)
  list(APPEND MATCHING_ENGINE_SOURCES ${source})
endforeach()
message(STATUS "Using Experimental String Processing library for _RegexParser (${EXPERIMENTAL_STRING_PROCESSING_SOURCE_DIR}).")

add_swift_target_library(swift_RegexParser ${SWIFT_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  "${MATCHING_ENGINE_SOURCES}"

  SWIFT_MODULE_DEPENDS_LINUX Glibc
  SWIFT_MODULE_DEPENDS_FREEBSD Glibc
  SWIFT_MODULE_DEPENDS_OPENBSD Glibc
  SWIFT_MODULE_DEPENDS_CYGWIN Glibc
  SWIFT_MODULE_DEPENDS_HAIKU Glibc
  SWIFT_MODULE_DEPENDS_WINDOWS CRT

  LINK_LIBRARIES ${swift_matching_engine_link_libraries}

  C_COMPILE_FLAGS
    -Dswift_RegexParser_EXPORTS
  SWIFT_COMPILE_FLAGS
    ${SWIFT_STANDARD_LIBRARY_SWIFT_FLAGS}
    # Workaround until `_RegexParser` is imported as implementation-only
    # by `_StringProcessing`.
    -Xfrontend -disable-implicit-string-processing-module-import
  LINK_FLAGS "${SWIFT_RUNTIME_SWIFT_LINK_FLAGS}"

  INSTALL_IN_COMPONENT stdlib
)
