set(CMAKE_CXX_FLAGS "${HAKASE_CXX_FLAGS}")

add_executable_with_deps(print/print.bin test.cc ../print.cc print/hakase.cc)
add_executable_with_deps(memrw/reading_signature.bin test.cc memrw/reading_signature.cc)
add_executable_with_deps(memrw/rw_small.bin test.cc memrw/rw_small.cc)
add_executable_with_deps(memrw/rw_large.bin test.cc memrw/rw_large.cc)
add_executable_with_deps(simple_loader/simple_loader.bin test.cc ../simple_loader.cc simple_loader/hakase.cc)
add_executable_with_deps(interrupt/interrupt.bin test.cc ../elf_loader.cc ../interrupt.cc interrupt/hakase.cc)
add_executable_with_deps(elf_loader/elf_loader.bin test.cc ../elf_loader.cc ../interrupt.cc elf_loader/hakase.cc)

add_subdirectory(simple_loader)
add_subdirectory(elf_loader)
add_subdirectory(interrupt)

