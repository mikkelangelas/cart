cmake_minimum_required(VERSION 3.10)

project(cart)

add_executable(cart
    src/main.c
    src/cart.c
    src/gb.c
    src/cpu.c
    src/mmu.c
    src/ppu.c
    src/cartridge.c
    src/joypad.c
    src/util.c
)

set_target_properties(cart PROPERTIES C_STANDARD 99)

find_package(SDL3 REQUIRED CONFIG)

if (NOT ${SDL3_FOUND})
    add_subdirectory(third-party/SDL3)
    target_include_directories(third-party/SDL3/include)
endif()

target_link_libraries(cart PRIVATE SDL3::SDL3)
