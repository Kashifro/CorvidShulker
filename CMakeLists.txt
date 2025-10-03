cmake_minimum_required(VERSION 3.5)

project(shulke)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(libhat
        GIT_REPOSITORY https://github.com/BasedInc/libhat.git
        GIT_TAG 69149a1cee5a0228492826bd71af1a32d73ed87e
        EXCLUDE_FROM_ALL)

FetchContent_Declare(properties-parser
        GIT_REPOSITORY https://github.com/minecraft-linux/properties-parser.git
        GIT_TAG 79f5360f889100bd4a1e069f4d9b86e94c902633
        EXCLUDE_FROM_ALL)


FetchContent_MakeAvailable(libhat properties-parser)


add_library(shulke SHARED src/main.cpp 
)
target_include_directories(shulke PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ 
)


target_link_libraries(shulke PUBLIC libhat::libhat properties-parser)

install(TARGETS shulke LIBRARY DESTINATION mods)
