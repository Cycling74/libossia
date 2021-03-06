project(ossia-max C CXX)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INCLUDE_CURRENT_DIR 1)

option(OSSIA_MAX_TEST "Run automatic test for C74 Max" OFF)
set(MAXSDK_PATH "${OSSIA_3RDPARTY_FOLDER}/max-sdk" CACHE PATH "Path to Max SDK folder")

include(GenerateExportHeader)
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../../../../support;@loader_path")
elseif(WIN32)
    set(CMAKE_INSTALL_RPATH  "\$ORIGIN/../support;\$ORIGIN")
endif()

if(${MAXSDK_PATH} STREQUAL "${OSSIA_3RDPARTY_FOLDER}/max-sdk")
  execute_process(COMMAND git submodule update --init -- ${OSSIA_3RDPARTY_FOLDER}/max-sdk
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(MAXSDK_MAX_INCLUDE_DIR "${MAXSDK_PATH}/source/c74support/max-includes")
if(APPLE)
  find_library(MAXSDK_API_LIBRARY NAMES MaxAPI HINTS "${MAXSDK_MAX_INCLUDE_DIR}")
elseif(WIN32)
  if("${CMAKE_SIZEOF_VOID_P}" MATCHES "4")
    find_library(MAXSDK_API_LIBRARY NAMES MaxAPI.lib HINTS "${MAXSDK_MAX_INCLUDE_DIR}")
  else()
    find_library(MAXSDK_API_LIBRARY NAMES MaxAPI.lib HINTS "${MAXSDK_MAX_INCLUDE_DIR}/x64")
  endif()
endif()

if(APPLE)
    FIND_PATH(FILE_H_DIR Files.h
        HINTS
        /Library/Developer/CommandLineTools//SDKs/MacOSX.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
        /System/Library/Frameworks/
        /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers/
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers)
    message( STATUS " Files.h have been found here : ${FILE_H_DIR}")
    if( ${FILE_H_DIR} EQUAL "FILE_H_DIR-NOT_FOUND")
        message( STATUS "  Files.h NOT FOUND !!" )
    else()
        include_directories("${FILE_H_DIR}")
    endif()
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SRCS
    src/ossia-max.cpp
    src/ossia-max.hpp
    src/object_base.cpp
    src/object_base.hpp
    src/ossia_object.cpp
    src/ossia_object.hpp
    src/attribute.cpp
    src/attribute.hpp
    src/client.hpp
    src/client.cpp
    src/device.hpp
    src/device.cpp
    src/device_base.hpp
    src/device_base.cpp
    src/explorer.hpp
    src/explorer.cpp
    src/fuzzysearch.hpp
    src/fuzzysearch.cpp
    src/logger.hpp
    src/logger.cpp
    src/matcher.cpp
    src/matcher.hpp
    src/model.hpp
    src/model.cpp
    src/monitor.cpp
    src/monitor.hpp
    src/node_base.cpp
    src/node_base.hpp
    src/parameter.hpp
    src/parameter.cpp
    src/parameter_base.hpp
    src/parameter_base.cpp
    src/remote.hpp
    src/remote.cpp
    src/router.cpp
    src/router.hpp
    src/search.hpp
    src/search.cpp
    src/search_filter.hpp
    src/search_filter.cpp
    src/utils.hpp
    src/utils.cpp
    src/view.hpp
    src/view.cpp
    src/ZeroconfMinuitListener.cpp
    src/ZeroconfMinuitListener.hpp
    src/ZeroconfOscqueryListener.cpp
    src/ZeroconfOscqueryListener.hpp

    ${MAXSDK_MAX_INCLUDE_DIR}/common/commonsyms.c
    )

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS})
target_compile_options(${PROJECT_NAME} PRIVATE ${OSSIA_COMPILE_OPTIONS})

# add_custom_target just to display sources files in qtcreator
add_custom_target(source SOURCES ${PROJECT_SRCS})
use_gold(${PROJECT_NAME})
add_linker_warnings_external(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE ossia ${MAXSDK_API_LIBRARY})

generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MAXSDK_MAX_INCLUDE_DIR}

    ${OSSIA_3RDPARTY_FOLDER}/concurrentqueue
    ${OSSIA_3RDPARTY_FOLDER}/Servus)

if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC_VERSION)
    target_include_directories(${PROJECT_NAME} PRIVATE ${FILE_H_DIR})
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z>)
    target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,-undefined,dynamic_lookup)
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN_VERSION _CRT_SECURE_NO_WARNINGS)
    if("${CMAKE_SIZEOF_VOID_P}" MATCHES "8")
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll64")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll")
    endif()
endif()

function(setup_max_external theTarget)
    if(APPLE)
        set_property(TARGET ${theTarget} PROPERTY BUNDLE True)
        set_property(TARGET ${theTarget} PROPERTY BUNDLE_EXTENSION "mxo")
        target_link_libraries(${theTarget} PRIVATE -Wl,-undefined,dynamic_lookup)
        file(COPY resources/PkgInfo DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${theTarget}.mxo/Contents/)
    elseif(WIN32)
        if("${CMAKE_SIZEOF_VOID_P}" MATCHES "8")
            set_target_properties(${theTarget} PROPERTIES SUFFIX ".mxe64")
        else()
            set_target_properties(${theTarget} PROPERTIES SUFFIX ".mxe")
        endif()
    endif()
endfunction()

# Shim externals to allow objects to be found by Max search box
set(OSSIA_EXT_LIST
    ossia.attribute
    ossia.parameter
    ossia.model
    ossia.view
    ossia.remote
    ossia.device
    ossia.client
    ossia.logger
    ossia.ossia
    ossia.explorer
    ossia.router
    ossia.fuzzysearch
    ossia.search
    ossia.monitor
)
set(OSSIA_MAX_SYMBOLS "")
foreach(ossiaExt ${OSSIA_EXT_LIST})
    string(REPLACE "." "_" ossia_lib_name ${ossiaExt})
    configure_file(src/empty_external.c.in ${CMAKE_CURRENT_BINARY_DIR}/${ossiaExt}.c)
    add_library(${ossiaExt} MODULE ${CMAKE_CURRENT_BINARY_DIR}/${ossiaExt}.c)
    target_link_libraries(${ossiaExt} PRIVATE ${PROJECT_NAME})
    target_include_directories(${ossiaExt} PRIVATE ${CMAKE_BINARY_DIR}/ossia)
    setup_max_external(${ossiaExt})

    string(APPEND OSSIA_MAX_SYMBOLS "_${ossia_lib_name}_setup\n")
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/package-info.json.in" "${CMAKE_CURRENT_BINARY_DIR}/package-info.json" @ONLY)

# We only export the Max required symbols.
# This is of utmost importance for support in Max4Live : ossia and Ableton Live both use 
# boost.asio, which leads to conflicts at runtime
if(APPLE AND CMAKE_BUILD_TYPE MATCHES Release)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/symbols.txt" "${OSSIA_MAX_SYMBOLS}\n")
  get_property(FLODSF TARGET ossia-max PROPERTY LINK_FLAGS)
  set_property(TARGET ossia-max APPEND PROPERTY
    LINK_FLAGS 
      "-s -exported_symbols_list '${CMAKE_CURRENT_BINARY_DIR}/symbols.txt'")
  get_property(FLODSF TARGET ossia-max PROPERTY LINK_FLAGS)
endif()

set_target_properties(ossia.ossia PROPERTIES OUTPUT_NAME "ossia")

set(OSSIA_MAX_INSTALL_FOLDER "ossia-max-package/ossia" CACHE STRING "Max Package Install folder")

install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/help"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples"
        "${CMAKE_CURRENT_SOURCE_DIR}/init"
        "${CMAKE_CURRENT_SOURCE_DIR}/interfaces"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        "${CMAKE_CURRENT_SOURCE_DIR}/patchers"
        "${CMAKE_CURRENT_SOURCE_DIR}/extras"
        "${CMAKE_CURRENT_SOURCE_DIR}/media"
    DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}"
    COMPONENT
        ${PROJECT_NAME}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/package-info.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/icon.png"
    DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}"
    COMPONENT
        ${PROJECT_NAME}
)

install(
    TARGETS
        ${PROJECT_NAME}
    BUNDLE DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/support"
    RUNTIME DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/support"
    LIBRARY DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/support"
    COMPONENT
        ${PROJECT_NAME}
)

install(
    TARGETS
        ${OSSIA_EXT_LIST}
    BUNDLE DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/externals"
    RUNTIME DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/externals"
    LIBRARY DESTINATION
        "${OSSIA_MAX_INSTALL_FOLDER}/externals"
    COMPONENT
        ${PROJECT_NAME}
)

if(OSSIA_MAX_TEST)
    set(OSSIA_MAX_TEST_PATCHERS
        ${CMAKE_CURRENT_LIST_DIR}/max-test/179-priority_when_binding_view.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/312-default_broken.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/355-repetition_filter_issue.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/366-priorities_init.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/367-set_message_with_empty_list.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/368-set_broken.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/369-register_object_only_once.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/377-noloadbanged_initialization.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/379-list_test.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/383-remote_outputs_zero_on_node_creation.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/384-address_output_persistence.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/387-attribute-propagation.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/415-poly_reset.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/441-multi_load.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/447-delete_node_during_registration.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/499-min_max_range.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/503-patcher_registration.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/506-poly-registering.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/507-dynamic_registering_under_device_object.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/508-bogus_parameter.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/508-bogus_parameter_on_bpatcher_deletion.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/513-impulse_test.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/549-string_range.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/554-set_parameter_recurse.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/559-ghost_doublon.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/545-bind_remote_without_args.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/542-mute.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/543-double_namespace.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/565-range_values_order.maxtest.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/573-set_unit_from_ossia.attribute.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/575-send_list.maxpat

        # test dependencies
        ${CMAKE_CURRENT_LIST_DIR}/max-test/559-master_subpatcher.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/default-sometimes_broken.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/subpatcher_for_503.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/model-test-for-369.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/param-test-for-369.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/submodel-test-for-369.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/default_in_poly.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/507_sub_bpatch.maxpat
        ${CMAKE_CURRENT_LIST_DIR}/max-test/508-bpatcher.maxpat
        )
    install(
        FILES
          ${OSSIA_MAX_TEST_PATCHERS}
        DESTINATION
          "${OSSIA_MAX_INSTALL_FOLDER}/patchers/tests"
        COMPONENT
          ${PROJECT_NAME}
          )
endif(OSSIA_MAX_TEST)
