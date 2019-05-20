#!python
EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

import os
import errno
import stat
import random
import string
from functools import reduce

curdir = Dir('.').abspath
ci = True if int(ARGUMENTS.get('CI', 0)) == 1 else False

os.environ["PATH"] += os.pathsep + curdir
env = DefaultEnvironment().Clone(ENV=os.environ,
                                 AR='bin/ar',
                                 AS='bin/g++',
                                 CC='bin/g++',
                                 CXX='bin/g++',
                                 RANLIB='bin/ranlib')

def docker_cmd(container, arg, workdir=curdir):
  return ['docker run -i --rm -v {0}:{0} -w {1} {2} {3}'.format(curdir, workdir, container, arg)]
def docker_format_cmd(arg, workdir=curdir):
  return docker_cmd('-v /etc/group:/etc/group:ro -v /etc/passwd:/etc/passwd:ro -u `id -u $USER`:`id -g $USER` livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f', arg, workdir)

def build_container(env, name, base, source):
  script = name + '.sh'
  return env.Command('.docker_tmp/sha1_' + name, ['docker/' + script] + source, [
    'docker rm -f $CONTAINER_NAME > /dev/null 2>&1 || :',
    Chmod('docker/' + script, '755'),
    'docker run --name=$CONTAINER_NAME -v {0}/docker:/mnt -v {0}/.docker_tmp:/share -w / {1} mnt/{2}'.format(curdir, base, script),
    'docker commit -c "CMD sh" $CONTAINER_NAME $IMG_NAME',
    'docker rm -f $CONTAINER_NAME',
    'docker images --digests -q --no-trunc $IMG_NAME > $TARGET'
  ], CONTAINER_NAME='toshokan_containerbuild_' + name, IMG_NAME='livadk/toshokan_' + name)
env.AddMethod(build_container, "BuildContainer")

build_intermediate_container = env.BuildContainer('build_intermediate', 'alpine:3.8', [])

qemu_kernel_container = env.BuildContainer('qemu_kernel', 'ubuntu:16.04', [])
gdb_container = env.BuildContainer('gdb', 'alpine:3.8', [])
ssh_container = env.BuildContainer('ssh', 'alpine:3.8', ['docker/config', 'docker/id_rsa', 'docker/wait-for'])
qemu_kernel_image_container = env.BuildContainer('qemu_kernel_image', 'ubuntu:16.04', [])
rootfs_container = env.BuildContainer('rootfs', 'alpine:3.8', [qemu_kernel_image_container])

def create_wrapper(target, source, env):
  if type(target) == list:
    target = target[0]
  target = str(target)
  with open(target, mode='w') as f:
    f.write('#!/bin/sh\n'\
            'args="$@"\n' + 
            '\n'.join(docker_cmd('livadk/toshokan_build_intermediate', os.path.basename(target) + ' $args')))

hakase_headers = env.Alias('hakase_headers', [
  Install('.docker_tmp/hakase_include/toshokan/', Glob('common/*.h')),
  Install('.docker_tmp/hakase_include/toshokan/', Glob('common/arch/hakase/*.h')),
  Install('.docker_tmp/hakase_include/toshokan/hakase', Glob('hakase/*.h'))])
friend_headers = env.Alias('friend_headers', [
  Install('.docker_tmp/friend_include/', Glob('stdinc/friend/*.h')),
  Install('.docker_tmp/friend_include/toshokan/', Glob('common/*.h')),
  Install('.docker_tmp/friend_include/toshokan/', Glob('common/arch/friend/*.h')),
  Install('.docker_tmp/friend_include/toshokan/friend', Glob('friend/*.h'))])
cpputest_headers = env.Alias('cpputest_headers', [
  Install('.docker_tmp/cpputest_include/', Glob('stdinc/cpputest/*.h')),
  Install('.docker_tmp/cpputest_include/toshokan/', Glob('common/*.h')),
  Install('.docker_tmp/cpputest_include/toshokan/', Glob('common/arch/cpputest/*.h'))])
FriendLoader_headers = env.Alias('FriendLoader_headers', [
  Install('.docker_tmp/FriendLoader_include/toshokan/', Glob('common/*.h'))])
headers=[hakase_headers, friend_headers, cpputest_headers, FriendLoader_headers]

wrappers = []
for binary in ['g++', 'ar', 'ranlib', 'objdump', 'objcopy']:
  wrappers.append(Command('bin/' + binary, build_intermediate_container,[
    create_wrapper,
    Chmod("$TARGET", '775')]))

def container_emitter(target, source, env):
  env.Depends(target, [wrappers, headers])
  return (target, source)

from SCons.Tool import createObjBuilders
static_obj, shared_obj = createObjBuilders(env)
static_obj.add_emitter('.cc', container_emitter)
static_obj.add_emitter('.c', container_emitter)
static_obj.add_emitter('.S', container_emitter)
static_obj.add_emitter('.o', container_emitter)
static_obj.add_emitter('.a', container_emitter)

hakase_flag = '-g -O0 -Wall --std=c++14 -static -fno-pie -no-pie'
friend_flag = '-g -O0 -Wall --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie'
friend_elf_flag = friend_flag + ' -T {0}/friend/friend.ld'.format(curdir)
cpputest_flag = '--std=c++14 --coverage -pthread'

def extract_include_path(list_):
    return list(map(lambda str: str.format(curdir), list_))

hakase_include_path = extract_include_path(['{0}/.docker_tmp/hakase_include'])
friend_include_path = extract_include_path(['{0}/.docker_tmp/friend_include'])
cpputest_include_path = extract_include_path(['{0}/.docker_tmp/cpputest_include'])

hakase_env = env.Clone(ASFLAGS=hakase_flag, CXXFLAGS=hakase_flag, LINKFLAGS=hakase_flag, CPPPATH=hakase_include_path, LIBPATH='#.docker_tmp/lib/')
friend_env = env.Clone(ASFLAGS=friend_flag, CXXFLAGS=friend_flag, LINKFLAGS=friend_flag, CPPPATH=friend_include_path, LIBPATH='#.docker_tmp/lib/')
friend_elf_env = env.Clone(ASFLAGS=friend_elf_flag, CXXFLAGS=friend_elf_flag, LINKFLAGS=friend_elf_flag, CPPPATH=friend_include_path, LIBPATH='#.docker_tmp/lib/')
cpputest_env = env.Clone(ASFLAGS=cpputest_flag, CXXFLAGS=cpputest_flag, LINKFLAGS=cpputest_flag, CPPPATH=cpputest_include_path)

Export('hakase_env friend_env friend_elf_env cpputest_env')

common_lib = SConscript(dirs=['common'])
Export('common_lib')

hakase_lib = SConscript(dirs=['hakase'])
hakase_ldscript = Command('.docker_tmp/$SOURCE', 'hakase/hakase.ld', Copy("$TARGET", "$SOURCE"))
hakase_build_container = env.BuildContainer('build_hakase', 'livadk/toshokan_build_intermediate', [build_intermediate_container, hakase_headers, hakase_lib, hakase_ldscript])

friend_lib = SConscript(dirs=['friend'])
friend_ldscript = Command('.docker_tmp/$SOURCE', 'friend/friend.ld', Copy("$TARGET", "$SOURCE"))
friend_build_container = env.BuildContainer('build_friend', 'livadk/toshokan_build_intermediate', [build_intermediate_container, friend_headers, friend_lib, friend_ldscript])

#TODO: refactoring
# if we prepare build container, we won't need this.
libs = hakase_lib
Export('libs')

SConscript(dirs=['common/tests'])
test_bins = []
test_bins += SConscript(dirs=['tests/boot'])
test_bins += SConscript(dirs=['tests/elf'])
test_bins += SConscript(dirs=['tests/symbol'])
test_bins += SConscript(dirs=['tests/clang'])

###############################################################################
# build FriendLoader & qemu container
###############################################################################
AlwaysBuild(env.Command('FriendLoader/friend_loader.ko', [qemu_kernel_container, Glob('FriendLoader/*.h'), Glob('FriendLoader/*.c')], docker_cmd('livadk/toshokan_qemu_kernel', 'sh -c "KERN_VER=4.13.0-45-generic make all"', curdir + '/FriendLoader')))

qemu_intermediate_container = env.BuildContainer('qemu_intermediate', 'livadk/toshokan_ssh', [
  ssh_container,
  qemu_kernel_image_container,
  rootfs_container,
  env.Command(".docker_tmp/friend_loader.ko", "FriendLoader/friend_loader.ko", Copy("$TARGET", "$SOURCE"))
  ])
Clean(qemu_intermediate_container, 'build')
qemu_container = env.BuildContainer('qemu', 'alpine:3.8', [qemu_intermediate_container])

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

# common tests
AlwaysBuild(env.Alias('common_test', [build_intermediate_container, 'common/tests/cpputest'], docker_cmd('livadk/toshokan_build_intermediate', './common/tests/cpputest -c -v')))

cleanup_containers = AlwaysBuild(env.Alias('cleanup_containers', [], [
  'docker ps -a -f name=toshokan_qemu_ -q | xargs -L 1 docker rm -f || :',
  'docker network ls -f name=toshokan_net_ -q | xargs -L 1 docker network rm || :',
]))

###############################################################################
# test pattern
###############################################################################
test_targets = []
for test_bin in test_bins:
  test_bin_name = str(test_bin)
  random_str = ''.join(random.choice(string.ascii_letters) for i in range(10))
  test_target = AlwaysBuild(env.Alias('test_' + test_bin_name, [cleanup_containers, qemu_container, ssh_container, test_bin], [
    'docker network create --driver bridge toshokan_net_{0}'.format(random_str),
    'docker run -d --name toshokan_qemu_{0} --network toshokan_net_{0} --net-alias toshokan_qemu livadk/toshokan_qemu qemu-system-x86_64 -cpu Haswell -s -d cpu_reset -no-reboot -smp 5 -m 4G -D /qemu.log -loadvm snapshot1 -hda /backing.qcow2 -net nic -net user,hostfwd=tcp::2222-:22 -serial telnet::4444,server,nowait -monitor telnet::4445,server,nowait -nographic'.format(random_str)] +
    docker_cmd('--network toshokan_net_{0} livadk/toshokan_ssh'.format(random_str), 'wait-for-rsync toshokan_qemu') +
    docker_cmd('--network toshokan_net_{0} livadk/toshokan_ssh'.format(random_str), 'rsync {0} toshokan_qemu:build/'.format(test_bin_name)) +
    docker_cmd('--network toshokan_net_{0} livadk/toshokan_ssh'.format(random_str), 'ssh toshokan_qemu sudo ' + test_bin_name) +
    ['docker rm -f toshokan_qemu_{0}'.format(random_str)]))
  test_targets.append(test_target)

# TODO remove build_container
test = AlwaysBuild(env.Alias('test', [hakase_build_container, friend_build_container, 'bin/objdump', 'common_test'] + test_targets))

Clean(test, '.docker_tmp')
Default(test)

# generate documents
AlwaysBuild(env.Alias('doc', '', 'find . \( -name \*.cc -or -name \*.c -or -name \*.h -or -name \*.S \) | xargs cat | awk \'/DOC START/,/DOC END/\' | grep -v "DOC START" | grep -v "DOC END" | grep -E --color=always "$|#.*$"'))

# push containers
def push_container(name):
  container_name = 'livadk/toshokan_' + name
  return AlwaysBuild(env.Alias('push_' + name, 'test', [
    'docker tag {0} {0}:v0.1'.format(container_name),
    'docker push {0}'.format(container_name),
    'docker push {0}:v0.1'.format(container_name),
  ]))

AlwaysBuild(env.Alias('push', [
    push_container('qemu'),
    push_container('build_hakase'),
    push_container('build_friend'),
    push_container('ssh'),
  ], []))

###############################################################################
# support functions
###############################################################################
AlwaysBuild(env.Alias('monitor', '', 'docker exec -it toshokan_qemu_{0} nc toshokan_qemu 4445'.format(ARGUMENTS.get('SIGNATURE'))))
AlwaysBuild(env.Alias('ssh', ssh_container, docker_cmd('-t --network toshokan_net_{0} livadk/toshokan_ssh'.format(ARGUMENTS.get('SIGNATURE')), 'ssh toshokan_qemu')))
