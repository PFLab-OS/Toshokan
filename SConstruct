EnsureSConsVersion(3, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

curdir = Dir('.').abspath
container_tag = "3a5eabf0fab92b8099129a3185f5fc98808ec8f3"
build_container = "livadk/toshokan_build:" + container_tag

docker_cmd = 'docker run --rm -v {0}:{0} {1} '.format(curdir,build_container)

import os
import errno
import stat
def build_wrapper(env, target, source):
  with open("bin/g++", mode='w') as f:
    f.write('#!/bin/sh\n' \
            'args="$@"\n' \
            + docker_cmd + \
	    'sh -c "cd $PWD && g++ $args"')
  os.chmod('bin/g++', os.stat('bin/g++').st_mode | stat.S_IEXEC)
  return None
  
Command('bin/g++', None, build_wrapper)

base_env = Environment(AS='{0}/bin/g++'.format(curdir),
                       CC='{0}/bin/g++'.format(curdir),
		       CXX='{0}/bin/g++'.format(curdir))

hakase_flag = '-g -O0 -MMD -MP -Wall --std=c++14 -static -isystem {0}/hakase -iquote {0} -D __HAKASE__'.format(curdir)
friend_flag = '-O0 -Wall --std=c++14 -nostdinc -nostdlib -isystem {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__'.format(curdir)
trampoline_flag = '-Os --std=c++14 -nostdinc -nostdlib -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__ -T {0}/hakase/FriendLoader/trampoline/boot_trampoline.ld'.format(curdir)
friend_elf_flag = friend_flag + ' -T {0}/friend/friend.ld'.format(curdir)

hakase_env = base_env.Clone(ASFLAGS=hakase_flag, CXXFLAGS=hakase_flag, LINKFLAGS=hakase_flag)
friend_env = base_env.Clone(ASFLAGS=friend_flag, CXXFLAGS=friend_flag, LINKFLAGS=friend_flag)
friend_elf_env = base_env.Clone(ASFLAGS=friend_elf_flag, CXXFLAGS=friend_elf_flag, LINKFLAGS=friend_elf_flag)

Export('hakase_env friend_env friend_elf_env')
SConscript(dirs=['hakase/tests'])

base_env.Clone(ASFLAGS=trampoline_flag, LINKFLAGS=trampoline_flag, CFLAGS=trampoline_flag, CXXFLAGS=trampoline_flag).Program(target='hakase/FriendLoader/trampoline/boot_trampoline.bin', source=['hakase/FriendLoader/trampoline/bootentry.S', 'hakase/FriendLoader/trampoline/main.cc'])

#Command('hakase/FriendLoader/friend_loader.ko', [Glob('hakase/FriendLoader/*.h'), Glob('hakase/FriendLoader/*.c'), 'hakase/FriendLoader/trampoline/bootentry.S', 'hakase/FriendLoader/trampoline/main.cc'], [
#	docker_cmd + 'gcc -Os -c -o hakase/FriendLoader/bootentry.o hakase/FriendLoader/bootentry.S'])

#${FRIENDLOADER_DIR}/friend_loader.ko: $(FRIENDLOADER_DIR)/*.h $(FRIENDLOADER_DIR)/*.c $(TRAMPOLINE_DIR)/bootentry.S $(TRAMPOLINE_DIR)/main.cc
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) gcc -Os -c -o $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/bootentry.S
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) g++ $(TRAMPOLINE_FLAGS) -c -o $(TRAMPOLINE_DIR)/main.o $(TRAMPOLINE_DIR)/main.cc
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) ld $(TRAMPOLINE_LDFLAGS) -T $(TRAMPOLINE_DIR)/boot_trampoline.ld -o $(TRAMPOLINE_DIR)/boot_trampoline.bin $(TRAMPOLINE_DIR)/bootentry.o $(TRAMPOLINE_DIR)/main.o
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) sh -c "cd $(TRAMPOLINE_DIR); objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o"
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) script/check_trampoline_bin_size.sh $(TRAMPOLINE_DIR)/bin.o
#	$(DOCKER_CMD) $(KERNEL_SRC_CONTAINER) sh -c "cd $(FRIENDLOADER_DIR); KERN_VERSION=4.13.0-45-generic make all"
