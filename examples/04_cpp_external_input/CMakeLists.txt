# Copyright 2021 Touca, Inc. Subject to Apache-2.0 License.

include(GNUInstallDirs)

touca_find_package("cxxopts")

add_executable(example_cpp_external_input "")

target_sources(
        example_cpp_external_input
    PRIVATE
        students.cpp
        students_test.cpp
)

target_link_libraries(
        example_cpp_external_input
    PRIVATE
        ${TOUCA_TARGET_MAIN}
        cxxopts::cxxopts
)

target_include_directories(
        example_cpp_external_input
    PRIVATE
        ${TOUCA_CLIENT_ROOT_DIR}
)

source_group(
    TREE ${CMAKE_CURRENT_LIST_DIR}
    FILES $<TARGET_PROPERTY:example_cpp_external_input,SOURCES>
)

install(
    TARGETS example_cpp_external_input
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
