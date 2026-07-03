cmake_minimum_required(VERSION 3.19)
project(BotCraft-Worker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOTCRAFT_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BOTCRAFT_BUILD_TESTS OFF CACHE INTERNAL "")
set(BOTCRAFT_BUILD_DOC OFF CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(
    Botcraft
    GIT_REPOSITORY https://github.com/adepierre/Botcraft.git
    GIT_TAG        3eafaf57a57660f8699943abf41d3c7bb08ecabf
)

FetchContent_MakeAvailable(Botcraft)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE botcraft)
