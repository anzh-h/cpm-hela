cmake_minimum_required (VERSION 3.22)
project (cpm_hela)

include(FetchContent)
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        3.1.0
)
set(SFML_BUILD_AUDIO OFF CACHE INTERNAL "")
set(SFML_BUILD_NETWORK OFF CACHE INTERNAL "")
set(SFML_BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(SFML)

find_package(Threads REQUIRED)

add_executable(cpm-hela
        src/CellType.cpp
        src/ColourScheme.cpp
        src/DivisionHandler.cpp
        src/Main.cpp
        src/RandomNumberGenerators.cpp
        src/SquareCellGrid.cpp
        src/SuperCell.cpp
        src/SuperCellTemplate.cpp
)

set_target_properties(cpm-hela PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(cpm-hela PRIVATE include)

target_link_libraries(cpm-hela PRIVATE
        Threads::Threads
        SFML::Graphics
)

add_custom_command(
        TARGET cpm-hela POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/default.cfg"
        "${CMAKE_CURRENT_SOURCE_DIR}/default.pgm"
        "$<TARGET_FILE_DIR:cpm-hela>"
        COMMENT "Copying config and layout files to output directory"
)