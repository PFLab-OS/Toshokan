EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

import os
import errno
import stat
from functools import reduce

curdir = Dir('.').abspath
container_tag = "3a5eabf0fab92b8099129a3185f5fc98808ec8f3"

env = DefaultEnvironment().Clone(
                  ENV=os.environ,
                  AS='{0}/bin/g++'.format(curdir),
                  CC='{0}/bin/g++'.format(curdir),
                  CXX='{0}/bin/g++'.format(curdir))

def docker_cmd(container, arg, workdir=curdir):
  if int(ARGUMENTS.get('CI', 0)) == 1:
    return ['docker rm -f toshokan_scons_container > /dev/null 2>&1 || :',
            'docker run -d -it -w {0} --name toshokan_scons_container {1} sh'.format(workdir, container),
	    'docker cp {0}/. toshokan_scons_container:{0}'.format(curdir),
	    'docker exec -i toshokan_scons_container {0}'.format(arg),
	    'docker cp toshokan_scons_container:{0}/. {0}'.format(curdir),
	    'docker rm -f toshokan_scons_container']
  else:
    return ['docker run -i --rm -v {0}:{0} -w {1} {2} {3}'.format(curdir, workdir, container, arg)]
def docker_build_cmd(arg, workdir=curdir):
    return docker_cmd('livadk/toshokan_build:' + container_tag, arg, workdir)
def docker_module_build_cmd(arg, workdir=curdir):
    return docker_cmd('livadk/toshokan_qemu_kernel:' + container_tag, arg, workdir)
def docker_format_cmd(arg, workdir=curdir):
    return docker_cmd('livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f', arg, workdir)

def build_wrapper(env, target, source):
  with open("bin/g++", mode='w') as f:
    f.write('\n'.join(['#!/bin/sh',
                       'args="$@"'] +
                       docker_build_cmd('g++ $args')))
  os.chmod('bin/g++', os.stat('bin/g++').st_mode | stat.S_IEXEC)
  return None
  
env.Command('bin/g++', None, build_wrapper)

hakase_flag = '-g -O0 -MMD -MP -Wall --std=c++14 -static -isystem {0}/hakase -iquote {0} -D __HAKASE__'.format(curdir)
friend_flag = '-O0 -Wall --std=c++14 -nostdinc -nostdlib -isystem {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__'.format(curdir)
friend_elf_flag = friend_flag + ' -T {0}/friend/friend.ld'.format(curdir)
trampoline_flag = '-Os --std=c++14 -nostdinc -nostdlib -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -iquote {0}/friend -iquote {0}/hakase -iquote {0} -D__FRIEND__ -T {0}/hakase/FriendLoader/trampoline/boot_trampoline.ld'.format(curdir)
trampoline_ld_flag = '-Os -nostdlib -T {0}/boot_trampoline.ld'.format(curdir)

hakase_env = env.Clone(ASFLAGS=hakase_flag, CXXFLAGS=hakase_flag, LINKFLAGS=hakase_flag)
friend_env = env.Clone(ASFLAGS=friend_flag, CXXFLAGS=friend_flag, LINKFLAGS=friend_flag)
friend_elf_env = env.Clone(ASFLAGS=friend_elf_flag, CXXFLAGS=friend_elf_flag, LINKFLAGS=friend_elf_flag)

Export('hakase_env friend_env friend_elf_env')
hakase_test_targets = SConscript(dirs=['hakase/tests'])

# FriendLoader & trampoline
trampoline_env = env.Clone(ASFLAGS=trampoline_flag, LINKFLAGS=trampoline_flag, CFLAGS=trampoline_flag, CXXFLAGS=trampoline_flag)
trampoline_env.Program(target='hakase/FriendLoader/trampoline/boot_trampoline.bin', source=['hakase/FriendLoader/trampoline/bootentry.S', 'hakase/FriendLoader/trampoline/main.cc'])
env.Command('hakase/FriendLoader/trampoline/bin.o', 'hakase/FriendLoader/trampoline/boot_trampoline.bin',
    docker_module_build_cmd('objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o', curdir + '/hakase/FriendLoader/trampoline') +
    docker_module_build_cmd('script/check_trampoline_bin_size.sh $TARGET'))
env.Command('hakase/FriendLoader/friend_loader.ko', [Glob('hakase/FriendLoader/*.h'), Glob('hakase/FriendLoader/*.c'), 'hakase/FriendLoader/trampoline/bin.o'], docker_module_build_cmd('KERN_VERSION=4.13.0-45-generic make all', curdir + 'hakase/FriendLoader'))

# local circleci
AlwaysBuild(env.Alias('circleci', [], 
    ['circleci config validate',
    'circleci build']))

# format
AlwaysBuild(env.Alias('format', [], 
    ['echo "Formatting with clang-format. Please wait..."'] +
    docker_format_cmd('sh -c "git ls-files . | grep -E \'.*\\.cc$$|.*\\.h$$\' | xargs -n 1 clang-format -style=\'{BasedOnStyle: Google}\' -i"') +
    ['echo "Done."']))
#$(if $(CI),&& git diff && git diff | wc -l | xargs test 0 -eq)

qemu_dir = '/home/hakase/'

def ssh_cmd(arg):
    return docker_cmd('--network toshokan_net livadk/toshokan_ssh:' + container_tag, 'ssh -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -p 2222 hakase@toshokan_qemu cd {0} \&\& {1}'.format(qemu_dir, arg))
def transfer_cmd(fname):
    return ssh_cmd('dd status=none of={0} < {1} \&\& chmod +rx {0}'.format(os.path.basename(fname), fname))

hakase_test_bin = ['hakase/tests/callback/callback.bin', 'hakase/tests/print/print.bin', 'hakase/tests/memrw/reading_signature.bin', 'hakase/tests/memrw/rw_small.bin', 'hakase/tests/memrw/rw_large.bin', 'hakase/tests/simple_loader/simple_loader.bin', 'hakase/tests/simple_loader/raw', 'hakase/tests/elf_loader/elf_loader.bin', 'hakase/tests/elf_loader/elf_loader.elf', 'hakase/tests/interrupt/interrupt.bin', 'hakase/tests/interrupt/interrupt.elf']

AlwaysBuild(env.Alias('prepare', '', 'script/build_container.sh ' + container_tag))

def expand_hakase_test_targets_to_depends():
    add_path_func = lambda ele: './build/' + ele
    return reduce(lambda list, ele: list + map(add_path_func, ele), hakase_test_targets, [])

def expand_hakase_test_targets_to_lists(prefix):
    add_path_func = lambda str, ele: str + ' ' + prefix + ele
    return list(map(lambda ele: reduce(add_path_func, ele, ''), hakase_test_targets))

# test pattern
test = AlwaysBuild(env.Alias('test', ['bin/g++'] + expand_hakase_test_targets_to_depends() + ['prepare'], [
    'docker rm -f toshokan_qemu 2>&1 || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    'docker run -d --name toshokan_qemu --network toshokan_net -P toshokan_qemu_back'] +
    transfer_cmd('hakase/FriendLoader/friend_loader.ko') +
    transfer_cmd('hakase/tests/test_hakase.sh') +
    transfer_cmd('hakase/tests/test_library.sh') +
    transfer_cmd('hakase/FriendLoader/run.sh') +
    reduce(lambda list, ele: list + transfer_cmd(ele), expand_hakase_test_targets_to_depends(), []) +
    reduce(lambda list, ele: list + ssh_cmd('./test_hakase.sh ' + ele), expand_hakase_test_targets_to_lists('./'), []) +
    ['docker rm -f toshokan_qemu']))

Default(test)