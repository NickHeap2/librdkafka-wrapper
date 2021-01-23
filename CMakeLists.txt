# CMakeList.txt : CMake project for librdkafka-wrapper, include source and define
# project specific logic here.
#
#list all vars
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

cmake_minimum_required (VERSION 3.8)

project (oekafka-wrapper VERSION 1.0 LANGUAGES C)

# shared source files
set(SOURCE_FILES oekafka-wrapper.c oekafka-wrapper.h oekafka-wrapper.def)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX -g")

#add_library (oekafka-wrapper-static STATIC ${SOURCE_FILES})
add_library (oekafka-wrapper SHARED ${SOURCE_FILES})
target_include_directories(oekafka-wrapper PRIVATE "." "avro" "rdkafka" "serdes" "/usr/include/librdkafka/" "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/")

set_property(TARGET oekafka-wrapper
    PROPERTY SOVERSION 1)

#set_property(TARGET oekafka-wrapper
#    PROPERTY C_STANDARD 11)
#set_property(TARGET oekafka-wrapper
#    PROPERTY VS_PACKAGE_REFERENCES "librdkafka.redist_1.3.0")

#set_property(TARGET oekafka-wrapper-static
#    PROPERTY C_STANDARD 11)
#set_property(TARGET oekafka-wrapper-static
#    PROPERTY VS_PACKAGE_REFERENCES "librdkafka.redist_1.3.0")

# add consumer test executable
set(CONSUMER_SOURCE_FILES consumer-test.c oekafka-wrapper.h)
add_executable (consumer-test ${CONSUMER_SOURCE_FILES})
target_link_libraries(consumer-test oekafka-wrapper)
target_include_directories(consumer-test PRIVATE "." "avro" "rdkafka" "serdes" "/usr/include/librdkafka/" "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/")

# add producer test executable
set(PRODUCER_SOURCE_FILES producer-test.c oekafka-wrapper.h)
add_executable (producer-test ${PRODUCER_SOURCE_FILES})
target_link_libraries(producer-test oekafka-wrapper)
target_include_directories(producer-test PRIVATE "." "avro" "rdkafka" "serdes" "/usr/include/librdkafka/" "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/")

# add combined test executable
set(COMBINED_SOURCE_FILES combined-test.c oekafka-wrapper.h)
add_executable (combined-test ${COMBINED_SOURCE_FILES})
target_link_libraries(combined-test oekafka-wrapper)
target_include_directories(combined-test PRIVATE "." "avro" "rdkafka" "serdes" "/usr/include/librdkafka/" "packages/librdkafka.redist.1.3.0/build/native/include/librdkafka/")


#set_property(TARGET wrapper-test
#    PROPERTY C_STANDARD 11)

#target_include_directories(librdkafka-wrapper PUBLIC
#                          "${CMAKE_CURRENT_BINARY_DIR}"
#                          "${CMAKE_CURRENT_SOURCE_DIR}"
#                          )

#libkafka
if (UNIX)
    message(STATUS "Finding Linux libkafka library")
    #find_library(LIBRDKAFKA librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    #find_library(LIBRDKAFKA centos7-librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    find_library(LIBRDKAFKA librdkafka.so.1 PATHS /usr/lib64/)
else()
    message(STATUS "IS NOT UNIX")
    find_library(LIBRDKAFKA librdkafka.lib PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/build/native/lib/win/x64/win-x64-Release/v120/)
endif()

if(NOT LIBRDKAFKA)
    message(FATAL_ERROR "LIBRDKAFKA library not found!")
else()
    message(STATUS "Found LIBRDKAFKA: " ${LIBRDKAFKA})
#    target_link_libraries(wrapper-test ${LIBRDKAFKA})
    #target_link_libraries(oekafka-wrapper-static ${LIBRDKAFKA})
    target_link_libraries(oekafka-wrapper ${LIBRDKAFKA})
endif()

#libserdes
if (UNIX)
    message(STATUS "Finding Linux libserdes library")
    #find_library(LIBRDKAFKA librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    #find_library(LIBRDKAFKA centos7-librdkafka.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/)
    find_library(LIBSERDES libserdes.so.1 PATHS /usr/lib64/)
endif()

if(NOT LIBSERDES)
    message(FATAL_ERROR "LIBSERDES library not found!")
else()
    message(STATUS "Found LIBSERDES: " ${LIBSERDES})
#    target_link_libraries(wrapper-test ${LIBRDKAFKA})
    #target_link_libraries(oekafka-wrapper-static ${LIBRDKAFKA})
    target_link_libraries(oekafka-wrapper ${LIBSERDES})
endif()

#libavro
if (UNIX)
    message(STATUS "Finding Linux libavro library")
    find_library(LIBAVRO libavro.so.23.0.0 PATHS /usr/lib64/)
endif()

if(NOT LIBAVRO)
    message(FATAL_ERROR "LIBAVRO library not found!")
else()
    message(STATUS "Found LIBAVRO: " ${LIBAVRO})
endif()

#libavrocpp
if (UNIX)
    message(STATUS "Finding Linux libavrocpp library")
    find_library(LIBAVROCPP libavrocpp.so.1.8.0.0 PATHS /usr/lib64/)
endif()

if(NOT LIBAVROCPP)
    message(FATAL_ERROR "LIBAVROCPP library not found!")
else()
    message(STATUS "Found LIBAVROCPP: " ${LIBAVROCPP})
endif()


if (NOT UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.config
    ${CMAKE_CURRENT_BINARY_DIR}/packages.config COPYONLY)
endif()

#find_program(NUGET nuget)
#if(NOT NUGET)
#    message(FATAL_ERROR "NUGET command not found!")
#else()
#    add_custom_target(nuget-restore
#      #COMMAND ${NUGET} restore ${CMAKE_CURRENT_SOURCE_DIR}/packages.config -PackagesDirectory ${CMAKE_CURRENT_SOURCE_DIR}/packages
#      #COMMAND ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/packages.config -PackagesDirectory ${CMAKE_CURRENT_BINARY_DIR}/packages
#    )
#endif()

#add_dependencies(${PROJECT_NAME} nuget-restore)

install(TARGETS oekafka-wrapper
  DESTINATION bin/
)

#install(TARGETS librdkafka-wrapper-static
#  DESTINATION bin/
#)

install(TARGETS consumer-test
  DESTINATION bin/
)

install(TARGETS producer-test
  DESTINATION bin/
)

install(TARGETS combined-test
  DESTINATION bin/
)

if (UNIX)
    #install(DIRECTORY "${PROJECT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/linux-x64/native/"
    install(FILES ${LIBRDKAFKA} ${LIBSERDES} ${LIBAVRO} ${LIBAVROCPP}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
else()
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/packages/librdkafka.redist.1.3.0/runtimes/win-x64/native/"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()
