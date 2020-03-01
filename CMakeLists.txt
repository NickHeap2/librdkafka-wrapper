# CMakeList.txt : CMake project for librdkafka-wrapper, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (librdkafka-wrapper VERSION 0.1)

set(SOURCE_FILES librdkafka-wrapper.c librdkafka-wrapper.h librdkafka-wrapper.def)
add_library (librdkafka-wrapper-static STATIC ${SOURCE_FILES})
add_library (librdkafka-wrapper SHARED ${SOURCE_FILES})

set_property(TARGET librdkafka-wrapper
    PROPERTY C_STANDARD 11)
set_property(TARGET librdkafka-wrapper
    PROPERTY VS_PACKAGE_REFERENCES "librdkafka.redist_1.3.0")

set_property(TARGET librdkafka-wrapper-static
    PROPERTY C_STANDARD 11)
set_property(TARGET librdkafka-wrapper-static
    PROPERTY VS_PACKAGE_REFERENCES "librdkafka.redist_1.3.0")


set(SOURCE_FILES wrapper-test.c librdkafka-wrapper.h)
add_executable (wrapper-test ${SOURCE_FILES})
target_link_libraries(wrapper-test librdkafka-wrapper-static)

set_property(TARGET wrapper-test
    PROPERTY C_STANDARD 11)

target_include_directories(librdkafka-wrapper PUBLIC
                          "${CMAKE_CURRENT_BINARY_DIR}"
                          "${CMAKE_CURRENT_SOURCE_DIR}"
                          )

if (UNIX)
    find_library(LIBRDKAFKA librdkafka.so)
else()
    find_library(LIBRDKAFKA librdkafka.lib PATHS ${CMAKE_CURRENT_BINARY_DIR}/packages/librdkafka.redist.1.3.0/build/native/lib/win/x64/win-x64-Release/v120/)
endif()

if(NOT LIBRDKAFKA)
    message(FATAL_ERROR "LIBRDKAFKA library not found!")
else()
    message(STATUS "Found LIBRDKAFKA: " ${LIBRDKAFKA})
    target_link_libraries(librdkafka-wrapper-static ${LIBRDKAFKA})
    target_link_libraries(librdkafka-wrapper ${LIBRDKAFKA})
endif()

find_program(NUGET nuget)
if(NOT NUGET)
    message(FATAL_ERROR "NUGET command not found!")
else()
    add_custom_target(nuget-restore
      COMMAND ${NUGET} restore ${CMAKE_CURRENT_SOURCE_DIR}/packages.config -PackagesDirectory ${CMAKE_CURRENT_SOURCE_DIR}/packages
      COMMAND ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/packages.config -PackagesDirectory ${CMAKE_CURRENT_BINARY_DIR}/packages
    )
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config
${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)

add_dependencies(${PROJECT_NAME} nuget-restore)

install(TARGETS librdkafka-wrapper
  DESTINATION bin/
)

install(TARGETS librdkafka-wrapper-static
  DESTINATION bin/
)

install(TARGETS wrapper-test
  DESTINATION bin/
)

if (WIN32)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/win-x64/native/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()


# list all vars
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
