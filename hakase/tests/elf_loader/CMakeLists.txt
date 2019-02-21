set(CMAKE_CXX_FLAGS "${FRIEND_ELF_CXX_FLAGS}")

add_executable_with_deps(elf_loader_test.elf friend.cc)
