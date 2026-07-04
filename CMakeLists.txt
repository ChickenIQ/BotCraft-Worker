cmake_minimum_required(VERSION 3.19)
project(BotCraft-Worker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Choose the type of build." FORCE)
endif ()

set(BOTCRAFT_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BOTCRAFT_BUILD_TESTS OFF CACHE INTERNAL "")
set(BOTCRAFT_BUILD_DOC OFF CACHE INTERNAL "")
if (NOT BOTCRAFT_CACHED_CREDENTIALS_PATH)
    set(BOTCRAFT_CACHED_CREDENTIALS_PATH "cached_credentials.json" CACHE INTERNAL "")
endif ()

include(FetchContent)
FetchContent_Declare(
        Botcraft
        GIT_REPOSITORY https://github.com/RegularRabbit05/Botcraft.git
        GIT_TAG 956396e584631d3040337ba6dcd199a165b072c0
)

FetchContent_MakeAvailable(Botcraft)

add_executable(${PROJECT_NAME}
        src/botcraft-worker/SocketPacket.hpp
        src/botcraft-worker/AtomicQueue.hpp
        src/botcraft-worker/ChatClient.hpp
        src/botcraft-worker/SocketConnection.hpp
        src/ChatClient.cpp
        src/SocketConnection.cpp
        src/SocketPacket.cpp
        src/WorkerEntrypoint.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE botcraft)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT output)
if (ipo_supported)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE
        -Os
        -ffunction-sections
        -fdata-sections
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Oz)
endif ()

target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,--gc-sections
        -Wl,--strip-all
)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-z,stack-size=1048576)