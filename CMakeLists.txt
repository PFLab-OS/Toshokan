CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

set(HAKASE_ELFLOADER_BIN_CFLAGS ${FRIEND_CFLAGS} -T friend/friend.ld)
set(CONTAINER_TAG 3a5eabf0fab92b8099129a3185f5fc98808ec8f3)
set(BUILD_CONTAINER livadk/toshokan_build:${CONTAINER_TAG})
set(KERNEL_SRC_CONTAINER livadk/toshokan_qemu_kernel:${CONTAINER_TAG})
set(COMMON_TEST_CFLAGS --std=c++14 --coverage -iquote common/tests/mock -I common/tests/mock -iquote . -iquote hakase/ -I/cpputest/include -L/cpputest/lib -lCppUTest -lCppUTestExt -pthread)
set(COMMON_TEST_CONTAINER livadk/cpputest:aac118d572d3c41bc9e3bed32c7ae8c19249784c)

set(SSH_COMMAND ${DOCKER_CMD} -it --network toshokan_net livadk/toshokan_ssh:${CONTAINER_TAG} ssh -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -p 2222 hakase@toshokan_qemu)
set(SFTP_COMMAND ${DOCKER_CMD} -i --network toshokan_net livadk/toshokan_ssh:${CONTAINER_TAG} sftp -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -P 2222 hakase@toshokan_qemu)
set(REMOTE_USER hakase)
# WIP: REMOTE_USER:=$(if $(ENV_FILE),$(shell . $(ROOT_DIR)/$(ENV_FILE); echo $$SSH_USER),hakase)
#set(SSH_COMMAND ssh nuc)
#set(SFTP_COMMAND sftp nuc)
#set(REMOTE_USER liva)

set(CMAKE_ASM_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/bin/g++)
set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/bin/g++)
set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/bin/g++)

add_custom_target(generate_wrappers
  COMMAND script/generate_wrappers.sh ${BUILD_CONTAINER})

function(add_executable_with_deps bin)
  add_executable(${bin} ${ARGN})
  add_dependencies(${bin} generate_wrappers)
endfunction()

set(HAKASE_CXX_FLAGS "-g -O0 -MMD -MP -Wall --std=c++14 -static -isystem ${CMAKE_CURRENT_SOURCE_DIR}/hakase -iquote ${CMAKE_CURRENT_SOURCE_DIR} -D __HAKASE__")
set(FRIEND_CXX_FLAGS "-O0 -Wall --std=c++14 -nostdinc -nostdlib -isystem ${CMAKE_CURRENT_SOURCE_DIR}/friend -iquote ${CMAKE_CURRENT_SOURCE_DIR}/hakase -iquote ${CMAKE_CURRENT_SOURCE_DIR} -D__FRIEND__")
set(FRIEND_ELF_CXX_FLAGS "${FRIEND_CXX_FLAGS} -T ${CMAKE_CURRENT_SOURCE_DIR}/friend/friend.ld")
add_subdirectory(hakase/tests/)


