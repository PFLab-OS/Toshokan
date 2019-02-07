set(CMAKE_CXX_FLAGS "${FRIEND_CXX_FLAGS}")

add_executable_with_deps(simple_loader_test raw.cc)
