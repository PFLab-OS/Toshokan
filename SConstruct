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

os.environ["PATH"] += os.pathsep + curdir
env = DefaultEnvironment().Clone(ENV=os.environ,
                               AS='bin/g++',
                               CC='bin/g++',
                               CXX='bin/g++')

def docker_cmd(container, arg, workdir=curdir):
  return ['docker run -i --rm -v {0}:{0} -w {1} {2} {3}'.format(curdir, workdir, container, arg)]
def docker_format_cmd(arg, workdir=curdir):
  return docker_cmd('-v /etc/group:/etc/group:ro -v /etc/passwd:/etc/passwd:ro -u `id -u $USER`:`id -g $USER` livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f', arg, workdir)

docker_tmp_dir = Command('docker/build', [], Mkdir("$TARGET"))

# BuildContainer only produces SHA1 (requires for dependency resolution of SCons)
# BuildContainerWithImage also produces docker container image (used for cache on circleci)
# If a container is an intermediate container (to build another container), you should use BuildContainer. (It will reduce cache size.)
# If the container is expected to be used by another part of the build, you have to use BuildContainerWithImage.

def build_container(env, name, base, source):
  script = name + '.sh'
  return env.Command('.docker_images/sha1_' + name, [docker_tmp_dir, 'docker/' + script] + source, [
    'docker rm -f $CONTAINER_NAME > /dev/null 2>&1 || :',
    Chmod('docker/' + script, '755'),
    'docker run --name=$CONTAINER_NAME -v {0}/docker:/mnt -w / {1} mnt/{2}'.format(curdir, base, script),
    'docker commit -c "CMD sh" $CONTAINER_NAME $IMG_NAME',
    'docker rm -f $CONTAINER_NAME',
    'docker images --digests -q --no-trunc $IMG_NAME > $TARGET'
  ], CONTAINER_NAME='toshokan_containerbuild_' + name, IMG_NAME='livadk/toshokan_' + name)
env.AddMethod(build_container, "BuildContainer")

huge_container_list = []
def build_container_with_image(env, name, base, source):
  container = env.Command('.docker_images/' + name + '.tar', env.BuildContainer(name, base, source), 'docker save -o $TARGET {0}'.format('livadk/toshokan_' + name))
  huge_container_list.append(container)
  return container
env.AddMethod(build_container_with_image, "BuildContainerWithImage")

build_intermediate_container = env.BuildContainerWithImage('build_intermediate', 'alpine:3.8', [])

#TODO: add libraries
#TODO: add include copy
build_container = env.BuildContainer('build', 'livadk/toshokan_build_intermediate', [build_intermediate_container, Glob('include/*.h')])
qemu_kernel_container = env.BuildContainerWithImage('qemu_kernel', 'ubuntu:16.04', [])
gdb_container = env.BuildContainer('gdb', 'alpine:3.8', [])
ssh_container = env.BuildContainerWithImage('ssh', 'alpine:3.8', ['docker/config', 'docker/id_rsa', 'docker/wait-for'])
qemu_kernel_image_container = env.BuildContainer('qemu_kernel_image', 'ubuntu:16.04', [])
rootfs_container = env.BuildContainer('rootfs', 'alpine:3.8', [qemu_kernel_image_container])
build_qemu_bin_container = env.BuildContainer('build_qemu_bin', 'ubuntu:16.04', [])
qemu_intermediate_container = env.BuildContainerWithImage('qemu_intermediate', 'alpine:3.8', [build_qemu_bin_container, qemu_kernel_image_container, rootfs_container])

AlwaysBuild(env.Alias('build_huge_docker_images', huge_container_list))

def create_wrapper(target, source, env):
  if type(target) == list:
    target = target[0]
  with open(str(target), mode='w') as f:
    f.write('#!/bin/sh\n'\
            'args="$@"\n' + 
            '\n'.join(docker_cmd('livadk/toshokan_build_intermediate', 'g++ $args')))

gcc_wrapper = Command('bin/g++', build_intermediate_container,[
        create_wrapper,
        Chmod("$TARGET", '775')])

def container_emitter(target, source, env):
  env.Depends(target, gcc_wrapper)
  return (target, source)

from SCons.Tool import createObjBuilders
static_obj, shared_obj = createObjBuilders(env)
static_obj.add_emitter('.cc', container_emitter)
static_obj.add_emitter('.c', container_emitter)
static_obj.add_emitter('.S', container_emitter)
static_obj.add_emitter('.o', container_emitter)

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

env.Command('hakase/FriendLoader/trampoline/bin.o', [build_intermediate_container, 'hakase/FriendLoader/trampoline/boot_trampoline.bin'],
    docker_cmd('livadk/toshokan_build_intermediate', 'objcopy -I binary -O elf64-x86-64 -B i386:x86-64 boot_trampoline.bin bin.o', curdir + '/hakase/FriendLoader/trampoline') +
    docker_cmd('livadk/toshokan_build_intermediate', 'script/check_trampoline_bin_size.sh $TARGET'))

AlwaysBuild(env.Command('hakase/FriendLoader/friend_loader.ko', [qemu_kernel_container, Glob('hakase/FriendLoader/*.h'), Glob('hakase/FriendLoader/*.c'), 'hakase/FriendLoader/trampoline/bin.o'], docker_cmd('livadk/toshokan_qemu_kernel', 'sh -c "KERN_VER=4.13.0-45-generic make all"', curdir + '/hakase/FriendLoader')))

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
    return docker_cmd('--network toshokan_net livadk/toshokan_ssh', 'wait-for toshokan_qemu:2222 -- ssh toshokan_qemu cd {0} \&\& {1}'.format(qemu_dir, arg))
def transfer_cmd():
    return docker_cmd('--network toshokan_net livadk/toshokan_ssh', 'wait-for toshokan_qemu:2222 -- rsync build/* toshokan_qemu:.')

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



AlwaysBuild(env.Alias('common_test', [build_intermediate_container, 'common/tests/cpputest'], docker_cmd('livadk/toshokan_build_intermediate', './common/tests/cpputest -c -v')))

# test pattern
test = AlwaysBuild(env.Alias('test', ['common_test', qemu_intermediate_container, 'build/friend_loader.ko', 'build/run.sh', 'build/test_hakase.sh', 'build/test_library.sh', ssh_container] + expand_hakase_test_targets_to_depends(), [
    'docker rm -f toshokan_qemu > /dev/null 2>&1 || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    'docker run -d --name toshokan_qemu --network toshokan_net livadk/toshokan_qemu_intermediate qemu-system-x86_64 -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic'] +
    transfer_cmd() +
    reduce(lambda list, ele: list + ssh_cmd('./test_hakase.sh ' + ele), expand_hakase_test_targets_to_lists('./'), []) +
    ['docker rm -f toshokan_qemu']))

Default(test)

env.Alias('buildtest', ['build', 'common_test', ssh_container] + expand_hakase_test_targets_to_depends(), [
    'docker rm -f toshokan_qemu > /dev/null 2>&1 || :',
    'docker network rm toshokan_net || :',
    'docker network create --driver bridge toshokan_net',
    'docker run -d --name toshokan_qemu --network toshokan_net -P hogehoge'] +
    transfer_cmd() +
    reduce(lambda list, ele: list + ssh_cmd('./test_hakase.sh ' + ele), expand_hakase_test_targets_to_lists('./'), []) +
    ['docker rm -f toshokan_qemu'])

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

AlwaysBuild(env.Alias('doc', '', 'find . \( -name \*.cc -or -name \*.c -or -name \*.h -or -name \*.S \) | xargs cat | awk \'/DOC START/,/DOC END/\' | grep -v "DOC START" | grep -v "DOC END" | grep -E --color=always "$|#.*$"'))

# support functions
AlwaysBuild(env.Alias('monitor', '', 'docker exec -it toshokan_qemu nc toshokan_qemu 4445'))
AlwaysBuild(env.Alias('ssh', ssh_container, docker_cmd('-t --network toshokan_net livadk/toshokan_ssh', 'ssh toshokan_qemu')))
