#!python
EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

import os
import errno
import stat
from functools import reduce

curdir = Dir('.').abspath
container_tag = "979612c1397cade043aafa2fef4eea2c6613c246"
ci = True if int(ARGUMENTS.get('CI', 0)) == 1 else False

def docker_cmd(container, arg, workdir=curdir):
  if ci:
    return ['docker rm -f toshokan_scons_container > /dev/null 2>&1 || :',
            'docker run -d -it -w {0} --name toshokan_scons_container {1} sh'.format(workdir, container),
	    'docker cp {0}/. toshokan_scons_container:{0}'.format(curdir),
	    'docker exec -i toshokan_scons_container {0}'.format(arg),
	    'docker cp toshokan_scons_container:{0}/. {0}'.format(curdir),
	    'docker rm -f toshokan_scons_container']
  else:
    return ['docker run -i --rm -v {0}:{0} -w {1} {2} {3}'.format(curdir, workdir, container, arg)]
def docker_module_build_cmd(arg, workdir=curdir):
    return docker_cmd('livadk/toshokan_qemu_kernel:' + container_tag, arg, workdir)
def docker_format_cmd(arg, workdir=curdir):
    return docker_cmd('-v /etc/group:/etc/group:ro -v /etc/passwd:/etc/passwd:ro -u `id -u $USER`:`id -g $USER` livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f', arg, workdir)

os.environ["PATH"] += os.pathsep + curdir
env = DefaultEnvironment().Clone(ENV=os.environ,
                               AS='bin/g++',
                               CC='bin/g++',
                               CXX='bin/g++')

def container_emitter(target, source, env):
  env.Depends(target, '#bin/g++')
  env.Depends(target, '#.dummyfile_toshokan_build')
  return (target, source)

from SCons.Tool import createObjBuilders
static_obj, shared_obj = createObjBuilders(env)
static_obj.add_emitter('.cc', container_emitter)
static_obj.add_emitter('.c', container_emitter)
static_obj.add_emitter('.S', container_emitter)
static_obj.add_emitter('.o', container_emitter)

Command('bin/g++', '.dummyfile_toshokan_build_intermediate',
        list(map(lambda str: 'echo "{0}" >> bin/g++'.format(str) , ['#!/bin/sh', 'args="\\$$@"'] + docker_cmd('livadk/toshokan_build_intermediate', 'g++ \\$$args'))) +
        [Chmod("$TARGET", '775')])

hakase_flag = '-g -O0 -MMD -MP -Wall --std=c++14 -static -D __HAKASE__'
friend_flag = '-O0 -Wall --std=c++14 -nostdinc -nostdlib -D__FRIEND__'
friend_elf_flag = friend_flag + ' -T {0}/friend/friend.ld'.format(curdir)
trampoline_flag = '-Os --std=c++14 -nostdinc -nostdlib -ffreestanding -fno-builtin -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -D__FRIEND__ -T {0}/hakase/FriendLoader/trampoline/boot_trampoline.ld'.format(curdir)
trampoline_ld_flag = '-Os -nostdlib -T {0}/boot_trampoline.ld'.format(curdir)
cpputest_flag = '--std=c++14 --coverage -D__CPPUTEST__ -pthread'

def extract_include_path(list_):
    return list(map(lambda str: str.format(curdir), list_))

hakase_include_path = extract_include_path(['{0}/hakase', '{0}/include', '{0}'])
friend_include_path = extract_include_path(['{0}/friend', '{0}/include', '{0}'])
cpputest_include_path = extract_include_path(['{0}/common/tests/mock', '{0}/hakase', '{0}/include', '{0}'])

hakase_env = env.Clone(ASFLAGS=hakase_flag, CXXFLAGS=hakase_flag, LINKFLAGS=hakase_flag, CPPPATH=hakase_include_path)
friend_env = env.Clone(ASFLAGS=friend_flag, CXXFLAGS=friend_flag, LINKFLAGS=friend_flag, CPPPATH=friend_include_path)
friend_elf_env = env.Clone(ASFLAGS=friend_elf_flag, CXXFLAGS=friend_elf_flag, LINKFLAGS=friend_elf_flag, CPPPATH=friend_include_path)
cpputest_env = env.Clone(ASFLAGS=cpputest_flag, CXXFLAGS=cpputest_flag, LINKFLAGS=cpputest_flag, CPPPATH=cpputest_include_path)

Export('hakase_env friend_env friend_elf_env cpputest_env')
hakase_test_targets = SConscript(dirs=['hakase/tests'])

SConscript(dirs=['common/tests'])

# FriendLoader & trampoline
trampoline_env = env.Clone(ASFLAGS=trampoline_flag, LINKFLAGS=trampoline_flag, CFLAGS=trampoline_flag, CXXFLAGS=trampoline_flag, CPPPATH=friend_include_path)
trampoline_env.Program(target='hakase/FriendLoader/trampoline/boot_trampoline.bin', source=['hakase/FriendLoader/trampoline/bootentry.S', 'hakase/FriendLoader/trampoline/main.cc'])
env.Command('hakase/FriendLoader/trampoline/bin.o', 'hakase/FriendLoader/trampoline/boot_trampoline.bin',
    docker_module_build_cmd('objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o', curdir + '/hakase/FriendLoader/trampoline') +
    docker_module_build_cmd('script/check_trampoline_bin_size.sh $TARGET'))
AlwaysBuild(env.Command('hakase/FriendLoader/friend_loader.ko', [Glob('hakase/FriendLoader/*.h'), Glob('hakase/FriendLoader/*.c'), 'hakase/FriendLoader/trampoline/bin.o'], docker_module_build_cmd('sh -c "KERN_VER=4.13.0-45-generic make all"', curdir + '/hakase/FriendLoader')))

# local circleci
AlwaysBuild(env.Alias('circleci', [], 
    ['circleci config validate',
    'circleci build --job build_python2',
    'circleci build --job build_python3']))

# format
AlwaysBuild(env.Alias('format', [], 
    ['echo "Formatting with clang-format. Please wait..."'] +
    docker_format_cmd('sh -c "git ls-files . | grep -E \'.*\\.cc$$|.*\\.h$$\' | xargs -n 1 clang-format -i -style=\'{{BasedOnStyle: Google}}\' {0}"'.format('&& git diff && git diff | wc -l | xargs test 0 -eq' if ci else '')) +
    ['echo "Done."']))

qemu_dir = '/home/hakase/'

def ssh_cmd(arg):
    return docker_cmd('--network toshokan_net livadk/toshokan_ssh:' + container_tag, 'wait-for toshokan_qemu:2222 -- ssh toshokan_qemu cd {0} \&\& {1}'.format(qemu_dir, arg))
def transfer_cmd():
    return docker_cmd('--network toshokan_net livadk/toshokan_ssh:' + container_tag, 'wait-for toshokan_qemu:2222 -- rsync build/* toshokan_qemu:.')

hakase_test_bin = ['hakase/tests/callback/callback.bin', 'hakase/tests/print/print.bin', 'hakase/tests/memrw/reading_signature.bin', 'hakase/tests/memrw/rw_small.bin', 'hakase/tests/memrw/rw_large.bin', 'hakase/tests/simple_loader/simple_loader.bin', 'hakase/tests/simple_loader/raw', 'hakase/tests/elf_loader/elf_loader.bin', 'hakase/tests/elf_loader/elf_loader.elf', 'hakase/tests/interrupt/interrupt.bin', 'hakase/tests/interrupt/interrupt.elf']

def expand_hakase_test_targets_to_depends():
    add_path_func = lambda ele: './build/' + ele
    return reduce(lambda list_, ele: list_ + list(map(add_path_func, ele)), hakase_test_targets, [])

def expand_hakase_test_targets_to_lists(prefix):
    add_path_func = lambda str, ele: str + ' ' + prefix + ele
    return list(map(lambda ele: reduce(add_path_func, ele, ''), hakase_test_targets))

env.Command("build/friend_loader.ko", "hakase/FriendLoader/friend_loader.ko", Copy("$TARGET", "$SOURCE"))
env.Command("build/run.sh", "hakase/FriendLoader/run.sh", Copy("$TARGET", "$SOURCE"))
env.Command("build/test_hakase.sh", "hakase/tests/test_hakase.sh", Copy("$TARGET", "$SOURCE"))
env.Command("build/test_library.sh", "hakase/tests/test_library.sh", Copy("$TARGET", "$SOURCE"))



AlwaysBuild(env.Alias('common_test', ['common/tests/cpputest'], docker_cmd('livadk/toshokan_build_intermediate', './common/tests/cpputest -c -v')))

# test pattern
test = AlwaysBuild(env.Alias('test', ['common_test', 'build/friend_loader.ko', 'build/run.sh', 'build/test_hakase.sh', 'build/test_library.sh'] + expand_hakase_test_targets_to_depends(), [
    'docker rm -f toshokan_qemu > /dev/null 2>&1 || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    'docker run -d --name toshokan_qemu --network toshokan_net -P livadk/toshokan_qemu:{0}'.format(container_tag)] +
    transfer_cmd() +
    reduce(lambda list, ele: list + ssh_cmd('./test_hakase.sh ' + ele), expand_hakase_test_targets_to_lists('./'), []) +
    ['docker rm -f toshokan_qemu']))

Default(test)

# build docker container
env.Alias('build', ['build/friend_loader.ko', 'build/run.sh', 'build/test_hakase.sh', 'build/test_library.sh'], [
    'docker rm -f toshokan_qemu > /dev/null 2>&1 || :',
    'docker run -d --name toshokan_qemu --network toshokan_net -P livadk/toshokan_qemu:{0} sh -c "qemu-img create -f qcow2 /backing2.qcow2 5G && qemu-system-x86_64 -cpu Haswell -d cpu_reset -no-reboot -smp 5 -m 4G -D /tmp/qemu.log -hda /backing2.qcow2 -kernel /vmlinuz-4.13.0-45-generic -initrd /rootfs -append \'root=/dev/ram rdinit=/sbin/init memmap=0x70000\$4K memmap=0x40000000\$0x40000000 console=ttyS0,115200\' -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic"'.format(container_tag),
    'sh -c "while ! docker run -t --rm --network toshokan_net livadk/toshokan_ssh:' + container_tag + ' ssh toshokan_qemu exit 0 > /dev/null 2>&1 ; do sleep 1; done"'] +
    transfer_cmd() +
    ['docker run -t --rm --network toshokan_net livadk/toshokan_ssh:' + container_tag + ' ssh toshokan_qemu sudo insmod friend_loader.ko',
    'docker exec -t toshokan_qemu sh -c "echo \'stop\' | nc toshokan_qemu 4445"',
    'docker exec -t toshokan_qemu sh -c "echo \'savevm snapshot1\' | nc toshokan_qemu 4445"',
    'docker exec -t toshokan_qemu ls /',
    'docker exec -t toshokan_qemu sh -c "echo \'quit\' | nc toshokan_qemu 4445"',
    'docker commit -c "CMD qemu-system-x86_64 -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing2.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic" toshokan_qemu hogehoge',
    'docker rm -f toshokan_qemu'])

env.Command('.dummyfile_toshokan_build_intermediate', [], [
    'docker rm -f toshokan_build_intermediate > /dev/null 2>&1 || :',
    'docker run -d -it --name toshokan_build_intermediate alpine:3.8 sh'.format(container_tag),
    'docker exec toshokan_build_intermediate apk add --no-cache make g++ cpputest',
    'docker kill toshokan_build_intermediate',
    'docker commit -c "CMD sh" toshokan_build_intermediate livadk/toshokan_build_intermediate',
    'docker rm -f toshokan_build_intermediate',
    'docker images --digests -q --no-trunc livadk/toshokan_build_intermediate > $TARGET'])

#TODO: add libraries
env.Command('.dummyfile_toshokan_build', ['.dummyfile_toshokan_build_intermediate', Glob('include/*.h')], [
    'docker rm -f toshokan_build > /dev/null 2>&1 || :',
    'docker run -d -it --name toshokan_build alpine:3.8 sh'.format(container_tag),
    'docker exec toshokan_build mkdir -p /usr/local/include/toshokan',
    'docker cp include toshokan_build:/usr/local/include/toshokan',
    'docker kill toshokan_build',
    'docker commit -c "CMD sh" toshokan_build livadk/toshokan_build',
    'docker rm -f toshokan_build',
    'docker images --digests -q --no-trunc livadk/toshokan_build > $TARGET'])

env.Alias('buildtest', ['build', 'common_test'] + expand_hakase_test_targets_to_depends(), [
    'docker rm -f toshokan_qemu > /dev/null 2>&1 || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    'docker run -d --name toshokan_qemu --network toshokan_net -P hogehoge'] +
    transfer_cmd() +
    reduce(lambda list, ele: list + ssh_cmd('./test_hakase.sh ' + ele), expand_hakase_test_targets_to_lists('./'), []) +
    ['docker rm -f toshokan_qemu'])

AlwaysBuild(env.Alias('doc', '', 'find . \( -name \*.cc -or -name \*.c -or -name \*.h -or -name \*.S \) | xargs cat | awk \'/DOC START/,/DOC END/\' | grep -v "DOC START" | grep -v "DOC END" | grep -E --color=always "$|#.*$"'))

# support functions
AlwaysBuild(env.Alias('monitor', '', 'docker exec -it toshokan_qemu nc toshokan_qemu 4445'))
AlwaysBuild(env.Alias('ssh', '', docker_cmd('-t --network toshokan_net livadk/toshokan_ssh:' + container_tag, 'ssh toshokan_qemu')))
