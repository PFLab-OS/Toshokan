EnsureSConsVersion(3, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

curdir = Dir('.').abspath
container_tag = "3a5eabf0fab92b8099129a3185f5fc98808ec8f3"

docker_cmd = 'docker run --rm -v {0}:{0} -w {0} '.format(curdir)
docker_build_cmd = docker_cmd + 'livadk/toshokan_build:' + container_tag + ' '
docker_module_build_cmd = docker_cmd + 'livadk/toshokan_qemu_kernel:' + container_tag + ' '
docker_format_cmd = docker_cmd + 'livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f '

import os
import errno
import stat
def build_wrapper(env, target, source):
  with open("bin/g++", mode='w') as f:
    f.write('#!/bin/sh\n' \
            'args="$@"\n' \
            + docker_build_cmd + \
	    'g++ $args')
  os.chmod('bin/g++', os.stat('bin/g++').st_mode | stat.S_IEXEC)
  return None
  
Command('bin/g++', None, build_wrapper)

base_env = DefaultEnvironment().Clone(AS='{0}/bin/g++'.format(curdir),
                       CC='{0}/bin/g++'.format(curdir),
		       CXX='{0}/bin/g++'.format(curdir))

hakase_flag = '-g -O0 -MMD -MP -Wall --std=c++14 -static -isystem {0}/hakase -iquote {0} -D __HAKASE__'.format(curdir)
friend_flag = '-O0 -Wall --std=c++14 -nostdinc -nostdlib -isystem {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__'.format(curdir)
friend_elf_flag = friend_flag + ' -T {0}/friend/friend.ld'.format(curdir)
trampoline_flag = '-Os --std=c++14 -nostdinc -nostdlib -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__ -T {0}/hakase/FriendLoader/trampoline/boot_trampoline.ld'.format(curdir)
trampoline_ld_flag = '-Os -nostdlib -T {0}/boot_trampoline.ld'.format(curdir)

hakase_env = base_env.Clone(ASFLAGS=hakase_flag, CXXFLAGS=hakase_flag, LINKFLAGS=hakase_flag)
friend_env = base_env.Clone(ASFLAGS=friend_flag, CXXFLAGS=friend_flag, LINKFLAGS=friend_flag)
friend_elf_env = base_env.Clone(ASFLAGS=friend_elf_flag, CXXFLAGS=friend_elf_flag, LINKFLAGS=friend_elf_flag)

Export('hakase_env friend_env friend_elf_env')
hakase_test_targets = SConscript(dirs=['hakase/tests'])

# FriendLoader & trampoline
trampoline_env = base_env.Clone(ASFLAGS=trampoline_flag, LINKFLAGS=trampoline_flag, CFLAGS=trampoline_flag, CXXFLAGS=trampoline_flag)
trampoline_env.Program(target='hakase/FriendLoader/trampoline/boot_trampoline.bin', source=['hakase/FriendLoader/trampoline/bootentry.S', 'hakase/FriendLoader/trampoline/main.cc'])
Command('hakase/FriendLoader/trampoline/bin.o', 'hakase/FriendLoader/trampoline/boot_trampoline.bin', [
    docker_module_build_cmd + 'sh -c "cd hakase/FriendLoader/trampoline; objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o"',
    docker_module_build_cmd + 'script/check_trampoline_bin_size.sh $TARGET'])
Command('hakase/FriendLoader/friend_loader.ko', [Glob('hakase/FriendLoader/*.h'), Glob('hakase/FriendLoader/*.c'), 'hakase/FriendLoader/trampoline/bin.o'], docker_module_build_cmd + 'sh -c "cd {0}; KERN_VERSION=4.13.0-45-generic make all"'.format('hakase/FriendLoader'))

# format
AlwaysBuild(Alias('format', [], [
    'echo "Formatting with clang-format. Please wait..."',
    docker_format_cmd + 'sh -c "git ls-files . | grep -E \'.*\\.cc$$|.*\\.h$$\' | xargs -n 1 clang-format -style=\'{BasedOnStyle: Google}\' -i"',
    'echo "Done."']))
#$(if $(CI),&& git diff && git diff | wc -l | xargs test 0 -eq)

AlwaysBuild(Alias('test2', '', ['echo {0}'.format('@'.join(map((lambda target: ','.join(target)), hakase_test_targets)))]))

ssh_cmd = docker_cmd + '-it --network toshokan_net livadk/toshokan_ssh:' + container_tag + ' ssh -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -p 2222 hakase@toshokan_qemu'
sftp_cmd = docker_cmd + '-i --network toshokan_net livadk/toshokan_ssh:' + container_tag + ' sftp -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -P 2222 hakase@toshokan_qemu'

hakase_test_bin = ['hakase/tests/callback/callback.bin', 'hakase/tests/print/print.bin', 'hakase/tests/memrw/reading_signature.bin', 'hakase/tests/memrw/rw_small.bin', 'hakase/tests/memrw/rw_large.bin', 'hakase/tests/simple_loader/simple_loader.bin', 'hakase/tests/simple_loader/raw', 'hakase/tests/elf_loader/elf_loader.bin', 'hakase/tests/elf_loader/elf_loader.elf', 'hakase/tests/interrupt/interrupt.bin', 'hakase/tests/interrupt/interrupt.elf']
qemu_dir = '/home/hakase/share'

AlwaysBuild(Alias('prepare', '', 'script/build_container.sh ' + container_tag))

# test pattern
test = AlwaysBuild(Alias('test', hakase_test_bin.append('prepare'), [
    'docker rm -f toshokan_qemu || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    docker_cmd + '-d --name toshokan_qemu --network toshokan_net -P toshokan_qemu_back',
    'script/transfer.sh "{0}" {1}'.format(sftp_cmd, ' '.join(hakase_test_bin)),
    '{0} {1}/test_hakase.sh {1}/callback.bin'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/print.bin'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/reading_signature.bin'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/rw_small.bin'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/rw_large.bin'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/simple_loader.bin {1}/raw'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/elf_loader.bin {1}/elf_loader.elf'.format(ssh_cmd, qemu_dir),
    '{0} {1}/test_hakase.sh {1}/interrupt.bin {1}/interrupt.elf'.format(ssh_cmd, qemu_dir),
    'docker rm -f toshokan_qemu']))

Default(test)