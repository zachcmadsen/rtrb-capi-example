include(FetchContent)

FetchContent_Declare(
    rtrb
    GIT_REPOSITORY  https://github.com/zachcmadsen/rtrb-capi
    GIT_TAG main
)
message(STATUS "Fetching rtrb-capi...")
FetchContent_MakeAvailable(rtrb)

# Turn these off to reduce the number of files to build.
set(SDL_STATIC OFF CACHE BOOL "Build a static version of SDL2")
set(SDL_TEST OFF CACHE BOOL "Build the SDL2_test library")

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL
    GIT_TAG         release-2.30.5
)
message(STATUS "Fetching SDL2...")
FetchContent_MakeAvailable(SDL2)

add_executable(rtrb-example main.cpp)
target_link_libraries(rtrb-example PRIVATE rtrb::rtrb-static SDL2::SDL2main SDL2::SDL2)

# Output the rtrb-example executable to the bin subdirectory.
set_target_properties(rtrb-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Copy SDL2.dll to rtrb-example's output directory.
if (WIN32)
    add_custom_command(
        TARGET rtrb-example
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:rtrb-example>"
        VERBATIM
    )
endif()