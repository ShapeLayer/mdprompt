cmake_minimum_required(VERSION 3.16)
project(mdprompt VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if(POLICY CMP0135)
 cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
include(GNUInstallDirs)
if(APPLE)
 set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR};@loader_path")
elseif(UNIX)
 set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR};$ORIGIN")
endif()
include(FetchContent)
set(CMARK_TESTS OFF CACHE BOOL "" FORCE)
set(CMARK_SHARED ON CACHE BOOL "" FORCE)
set(CMARK_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(cmark URL https://codeload.github.com/github/cmark-gfm/tar.gz/refs/tags/0.29.0.gfm.13
 URL_HASH SHA256=5abc61798ebd9de5660bc076443c07abad2b8d15dbc11094a3a79644b8ad243a)
FetchContent_MakeAvailable(cmark)
if(WIN32)
 # This pinned release reuses the core DLL export macro in the extensions
 # header. Build its declarations undecorated and export the definitions
 # through CMake instead of incorrectly marking them dllimport (MSVC C4273).
 target_compile_definitions(libcmark-gfm-extensions PRIVATE CMARK_GFM_STATIC_DEFINE)
 set_target_properties(libcmark-gfm-extensions PROPERTIES
   DEFINE_SYMBOL libcmark_gfm_extensions_EXPORTS
   WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
add_library(mdprompt SHARED native/mdprompt.c)
target_include_directories(mdprompt PUBLIC native ${cmark_SOURCE_DIR}/src ${cmark_BINARY_DIR}/src PRIVATE ${cmark_SOURCE_DIR}/extensions ${cmark_BINARY_DIR}/extensions)
target_link_libraries(mdprompt PUBLIC libcmark-gfm)
add_executable(mdprompt-cli native/main.c)
target_include_directories(mdprompt-cli PRIVATE ${cmark_SOURCE_DIR}/extensions ${cmark_BINARY_DIR}/extensions)
target_link_libraries(mdprompt-cli PRIVATE mdprompt libcmark-gfm-extensions)
if(WIN32)
 # Windows does not search dependency build directories for runtime DLLs.
 add_custom_command(TARGET mdprompt-cli POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
     "$<TARGET_FILE:libcmark-gfm>"
     "$<TARGET_FILE:libcmark-gfm-extensions>"
     "$<TARGET_FILE_DIR:mdprompt-cli>"
   VERBATIM)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
 target_compile_options(mdprompt PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
include(CTest)
option(MDPROMPT_PYTHON_TESTS "Enable optional Python conformance tests" ON)
if(BUILD_TESTING AND MDPROMPT_PYTHON_TESTS)
 find_package(Python3 3.10 COMPONENTS Interpreter QUIET)
 if(Python3_FOUND)
  add_test(NAME conformance COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_mdprompt.py $<TARGET_FILE:mdprompt-cli>)
 else()
  message(STATUS "Python 3.10+ not found: optional conformance tests are unavailable")
 endif()
endif()
install(TARGETS mdprompt mdprompt-cli)
install(FILES native/mdprompt.h DESTINATION include)
