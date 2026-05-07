set(BOOST_MINOR_MINIMAL 86)
set(BOOST_MINOR_LATEST 90)
set(BOOST_MINOR_CONAN 86)

if(OSSIA_USE_CONAN)
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
      "${CMAKE_BINARY_DIR}/conan.cmake")
  endif()
  include(${CMAKE_BINARY_DIR}/conan.cmake)
  conan_check(VERSION 1.29.0 REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

  set(CONAN_PROFILE "default" CACHE STRING "The profile to use for building conan deps, useful for cross compiling")
  conan_cmake_configure(
    REQUIRES boost/1.${BOOST_MINOR_CONAN}.0
    GENERATORS cmake_find_package
    OPTIONS
      boost:shared=False
      boost:without_stacktrace=True
      boost:without_context=True
      boost:without_coroutine=True
      boost:without_fiber=True
      boost:without_locale=True
      boost:without_log=True
  )
  conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS_HOST build_type=${CMAKE_BUILD_TYPE}
    SETTINGS_BUILD build_type=${CMAKE_BUILD_TYPE}
    PROFILE_HOST ${CONAN_PROFILE}
    PROFILE_BUILD default
  )
  find_package(Boost 1.${BOOST_MINOR_CONAN} REQUIRED GLOBAL)
  if(BOOST_ROOT)
    set(Boost_INCLUDE_DIR "${BOOST_ROOT}" CACHE INTERNAL "")
  endif()
  add_library(boost INTERFACE IMPORTED GLOBAL)
  set_property(TARGET boost PROPERTY
               INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIR}")
  return()
endif()

unset(BOOST_VERSIONS_LIST)
set(current_val ${BOOST_MINOR_LATEST})
while("${current_val}" GREATER_EQUAL "${BOOST_MINOR_MINIMAL}")
    list(APPEND BOOST_VERSIONS_LIST ${current_val})
    math(EXPR current_val "${current_val} - 1")
endwhile()

foreach(boost_version ${BOOST_VERSIONS_LIST})
  find_package(Boost 1.${boost_version} EXACT GLOBAL QUIET)
  if(Boost_FOUND)
    break()
  endif()
endforeach()

if (NOT Boost_FOUND)
  set(OSSIA_MUST_INSTALL_BOOST 1 CACHE INTERNAL "")
  set(BOOST_VERSION "boost_1_${BOOST_MINOR_LATEST}_0" CACHE INTERNAL "")

  if(NOT EXISTS "${OSSIA_3RDPARTY_FOLDER}/${BOOST_VERSION}/")
    message(STATUS "Downloading boost to ${OSSIA_3RDPARTY_FOLDER}/${BOOST_VERSION}.tar.gz")
    set(BOOST_URL https://github.com/ossia/sdk/releases/download/sdk31/${BOOST_VERSION}.tar.gz)
    set(BOOST_ARCHIVE ${BOOST_VERSION}.tar.gz)

    file(DOWNLOAD "${BOOST_URL}" "${OSSIA_3RDPARTY_FOLDER}/${BOOST_ARCHIVE}")

    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E tar xzf "${BOOST_ARCHIVE}"
      WORKING_DIRECTORY "${OSSIA_3RDPARTY_FOLDER}"
      COMMAND_ERROR_IS_FATAL ANY
    )
  endif()
  set(BOOST_ROOT "${OSSIA_3RDPARTY_FOLDER}/${BOOST_VERSION}")
  set(BOOST_ROOT "${OSSIA_3RDPARTY_FOLDER}/${BOOST_VERSION}" CACHE INTERNAL "")
  set(Boost_INCLUDE_DIR "${BOOST_ROOT}")
  list(PREPEND CMAKE_FIND_ROOT_PATH "${BOOST_ROOT}")

  find_package(Boost 1.${BOOST_MINOR_LATEST} REQUIRED GLOBAL)
endif()
if(BOOST_ROOT)
  set(Boost_INCLUDE_DIR "${BOOST_ROOT}" CACHE INTERNAL "")
endif()
add_library(boost INTERFACE IMPORTED GLOBAL)
set_property(TARGET boost PROPERTY
             INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIR}")
