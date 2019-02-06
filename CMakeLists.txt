CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

set(HAKASE_CFLAGS -g -O0 -MMD -MP -Wall --std=c++14 -static -iquote hakase -I hakase -iquote . -D __HAKASE__)
set(FRIEND_CFLAGS -O0 -Wall --std=c++14 -nostdinc -nostdlib -iquote friend -I friend -iquote hakase -iquote . -D__FRIEND__)
set(HAKASE_SIMPLELOADER_BIN_CFLAGS ${FRIEND_CFLAGS})
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

set(CMAKE_CXX_COMPILER bin/g++)
set(CMAKE_CXX_FLAGS "-g -O0 -MMD -MP -Wall --std=c++14 -static -iquote hakase -I hakase -iquote . -D __HAKASE__")

add_custom_target(generate_wrappers
  COMMAND script/generate_wrappers.sh ${BUILD_CONTAINER})

add_executable(hakase/interrupt/test/interrupt.bin hakase/tests/test.o hakase/elf_loader/hakase.cc hakase/interrupt.cc hakase/interrupt/test/hakase.cc)
add_dependencies(hakase/interrupt/test/interrupt.bin generate_wrappers)
