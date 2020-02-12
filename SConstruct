#!python
EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(2, 5)
Decider('MD5-timestamp')

import os
import errno
import stat
from functools import reduce

base_env = DefaultEnvironment().Clone(tools = ['textfile'])

def gen_docker_cmd(env, container, arg):
  return 'docker run -i --rm -v {0}:{0} -w {0} {1} {2}'.format(curdir, container, arg)
base_env.AddMethod(gen_docker_cmd, "GenerateDockerCommand")

# run `scons generate_tools` after you change the version
tag_version = "v0.05b"

curdir = Dir('.').abspath
ci = True if int(ARGUMENTS.get('CI', 0)) == 1 else False

os.environ["PATH"] += os.pathsep + curdir
env = base_env.Clone(ENV=os.environ,
                     AR='bin/ar',
                     AS='bin/g++',
                     CC='bin/g++',
                     CXX='bin/g++',
                     RANLIB='bin/ranlib')

containers = {}

def build_container(env, name, base, source):
  script = name + '.sh'
  containers[name] = env.Command('.docker_tmp/sha1_' + name, ['docker/' + script] + source, [
    'docker rm -f $CONTAINER_NAME > /dev/null 2>&1 || :',
    Chmod('docker/' + script, '755'),
    'docker run --network host --name=$CONTAINER_NAME -v {0}/docker:/mnt -v {0}/.docker_tmp:/share -w / {1} mnt/{2}'.format(curdir, base, script),
    'docker commit -c "CMD sh" $CONTAINER_NAME $IMG_NAME',
    'docker rm -f $CONTAINER_NAME',
    'docker images --digests -q --no-trunc $IMG_NAME > $TARGET'
  ], CONTAINER_NAME='toshokan_containerbuild_' + name, IMG_NAME='livadk/toshokan_' + name)
env.AddMethod(build_container, "BuildContainer")

env.BuildContainer('build_intermediate', 'alpine:3.8', [])

env.BuildContainer('qemu_kernel', 'ubuntu:16.04', [])
env.BuildContainer('gdb', 'alpine:3.8', [])
env.BuildContainer('ssh_intermediate', 'alpine:3.8', ['docker/config', 'docker/id_rsa'])
#env.BuildContainer('ssh', 'livadk/toshokan_ssh_intermediate', [containers["ssh_intermediate"], 'docker/wait-for', 'docker/wait-for-rsync'])
env.BuildContainer('qemu_kernel_image', 'ubuntu:16.04', [])
env.BuildContainer('rootfs', 'alpine:3.8', [containers["qemu_kernel_image"]])

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

def container_emitter(target, source, env):
  env.Depends(target, [containers["build_intermediate"], headers])
  return (target, source)

from SCons.Tool import createObjBuilders
static_obj, shared_obj = createObjBuilders(env)
static_obj.add_emitter('.cc', container_emitter)
static_obj.add_emitter('.c', container_emitter)
static_obj.add_emitter('.S', container_emitter)
static_obj.add_emitter('.o', container_emitter)
static_obj.add_emitter('.a', container_emitter)

hakase_flag = '-O0 -Wall -Werror=unused-result --std=c++14 -static -fno-pie -no-pie'
friend_flag = '-O0 -Wall -Werror=unused-result --std=c++14 -nostdinc -nostdlib -fno-pie -no-pie'
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
cpputest_env = env.Clone(ASFLAGS=cpputest_flag, CXXFLAGS=cpputest_flag, LINKFLAGS=cpputest_flag, CPPPATH=cpputest_include_path, LIBPATH='#.docker_tmp/lib/')

Export('base_env hakase_env friend_env friend_elf_env cpputest_env')

build_tools = [Install('.docker_tmp/tools/wrapper/', Glob('tools/wrapper/*')),
               Install('.docker_tmp/tools/', 'tools/build_rules.mk')]
env.BuildContainer('tools', 'tianon/true', build_tools)

common_lib = SConscript(dirs=['common'])
Export('common_lib')

hakase_lib = SConscript(dirs=['hakase'])
hakase_ldscript = Command('.docker_tmp/$SOURCE', 'hakase/hakase.ld', Copy("$TARGET", "$SOURCE"))

env.BuildContainer('build_hakase', 'livadk/toshokan_build_intermediate', [containers["build_intermediate"], hakase_headers, hakase_lib, hakase_ldscript, common_lib])

friend_lib = SConscript(dirs=['friend'])
friend_ldscript = Command('.docker_tmp/$SOURCE', 'friend/friend.ld', Copy("$TARGET", "$SOURCE"))
env.BuildContainer('build_friend', 'livadk/toshokan_build_intermediate', [containers["build_intermediate"], friend_headers, friend_lib, friend_ldscript, common_lib])

SConscript(dirs=['common/tests'])

cloned_qemu = Command('.docker_tmp/qemu', [], 
    ['rm -rf .docker_tmp/.qemu',
     'git clone -b v4.2.0 --depth=1 https://github.com/qemu/qemu.git .docker_tmp/.qemu',
     'mv .docker_tmp/.qemu .docker_tmp/qemu'])

###############################################################################
# build FriendLoader & qemu container
###############################################################################
AlwaysBuild(env.Command('FriendLoader/friend_loader.ko', [containers["qemu_kernel"], Glob('FriendLoader/*.h'), Glob('FriendLoader/*.c'), FriendLoader_headers], env.GenerateDockerCommand('livadk/toshokan_qemu_kernel', 'sh -c "cd FriendLoader; KERN_VER=4.13.0-45-generic make all"')))

local_friendLoader = AlwaysBuild(env.Command('FriendLoader_local/friend_loader.ko', [Glob('FriendLoader/*.h'), Glob('FriendLoader/*.c'), FriendLoader_headers], [
  'rm -rf FriendLoader_local',
  'cp -r FriendLoader FriendLoader_local',
  'sh -c "cd FriendLoader_local; make all"',
]))

AlwaysBuild(env.Alias('insmod', [local_friendLoader], 
    ['sudo insmod FriendLoader_local/friend_loader.ko']))

AlwaysBuild(env.Alias('rmmod', [], 
    ['sudo rmmod friend_loader.ko']))

env.Command(".docker_tmp/friend_loader.ko", "FriendLoader/friend_loader.ko", Copy("$TARGET", "$SOURCE"))

env.BuildContainer('qemu_intermediate', 'livadk/toshokan_ssh_intermediate', [
  containers["ssh_intermediate"],
  containers["qemu_kernel_image"],
  containers["rootfs"],
  ".docker_tmp/friend_loader.ko",
  ])
env.BuildContainer('qemu', 'livadk/toshokan_ssh_intermediate', [containers["ssh_intermediate"], containers["qemu_intermediate"]])
env.BuildContainer('qemu_debug', 'ubuntu:16.04', [
  cloned_qemu,
  containers["ssh_intermediate"],
  containers["qemu_kernel_image"],
  containers["rootfs"],
  ".docker_tmp/friend_loader.ko",
  ])

# local circleci
AlwaysBuild(env.Alias('circleci', [], 
    ['circleci config validate',
    'circleci build --job build_python2',
    'circleci build --job build_python3',
    'circleci build --job doccheck']))

# format
def docker_format_cmd(arg):
  return env.GenerateDockerCommand('-v /etc/group:/etc/group:ro -v /etc/passwd:/etc/passwd:ro -u `id -u $USER`:`id -g $USER` livadk/clang-format:9f1d281b0a30b98fbb106840d9504e2307d3ad8f', arg)
AlwaysBuild(env.Alias('format', [], 
    ['echo "Formatting with clang-format. Please wait..."',
    docker_format_cmd('sh -c "git ls-files . | grep -E \'.*\\.cc$$|.*\\.c$$|.*\\.h$$\' | xargs -n 1 clang-format -i -style=\'{{BasedOnStyle: Google}}\' {0}"'.format('&& git diff && git diff | wc -l | xargs test 0 -eq' if ci else '')),
    'echo "Done."']))

AlwaysBuild(env.Alias('doccheck', [], 
    ['cd tutorial_template; ./build.py',
     'git diff && git diff | wc -l | xargs test 0 -eq']))

# common tests
AlwaysBuild(env.Alias('common_test', [containers["build_intermediate"], 'common/tests/cpputest'], env.GenerateDockerCommand('livadk/toshokan_build_intermediate', './common/tests/cpputest -c -v')))

Export('containers')
test = SConscript(dirs=['tests'])

Clean(test, '.docker_tmp')
Default(test)

# generate documents
AlwaysBuild(env.Alias('doc', '', 'find . \( -name \*.cc -or -name \*.c -or -name \*.h -or -name \*.S \) | xargs cat | awk \'/DOC START/,/DOC END/\' | grep -v "DOC START" | grep -v "DOC END" | grep -E --color=always "$|#.*$"'))

# push containers
def tag_container(name):
  container_name = 'livadk/toshokan_' + name
  return AlwaysBuild(env.Alias('tag_' + name, ['test', containers[name]], [
    'docker tag {0} {0}:{1}'.format(container_name, tag_version),
  ]))

def push_container(name):
  container_name = 'livadk/toshokan_' + name
  return AlwaysBuild(env.Alias('push_' + name, ['test', 'tag_' + name], [
    'docker push {0}'.format(container_name),
    'docker push {0}:{1}'.format(container_name, tag_version),
  ]))

def pull_container(name):
  container_name = 'livadk/toshokan_' + name
  return AlwaysBuild(env.Alias('pull_' + name, [], [
    'docker pull {0}:{1}'.format(container_name, tag_version),
  ]))

output_containers = ['qemu', 'qemu_debug', 'tools', 'build_hakase', 'build_friend'] #, 'ssh'

AlwaysBuild(env.Alias('tag', list(map(tag_container, output_containers)), []))

AlwaysBuild(env.Alias('push', list(map(push_container, output_containers)), []))

AlwaysBuild(env.Alias('imagecheck', list(map(pull_container, output_containers)), []))

# sample version & tutorial version should be updated manually
# It is because their version update often requires source code changes.
# If these version information is updated automatically, no one will care about the changes anymore and samples & tutorials will be ruined.
# To prevent this, we should update them by hand, and check it with CI.
AlwaysBuild(env.Alias('versioncheck', [], 
                      'grep {0} sample/Makefile | wc -l | xargs test 0 -ne && grep {0} tutorial_template/build_misc/settings.yml | wc -l | xargs test 0 -ne'.format(tag_version)))

###############################################################################
# automatic generation
###############################################################################
def build_binscript(env, name, target_env, binname = "\"$$(basename \"$$0\")\""):
  return env.Command(name, [], [
    "echo '#!/bin/sh' > $FNAME",
    "echo 'exec \"$$(dirname \"$$0\")/base\" $TARGET_ENV $BIN_NAME \"$$@\"' >> $FNAME",
    "chmod +x $FNAME",
  ], FNAME=name, TARGET_ENV=target_env, BIN_NAME=binname)
def build_basescript(env, name, test):
  if not test:
    version = ':' + tag_version
    docker_flag = '-i'
  else:
    version = ''
    docker_flag = '-i'
  return env.Command(name, [], [
    "echo '#!/bin/sh' > $FNAME",
    "echo 'CONTAINER_TYPE=$$1' >> $FNAME",
    "echo 'BINNAME=$$2' >> $FNAME",
    "echo 'shift 2' >> $FNAME",
    "echo 'PROJECT_ROOT=$$(cd \"$${PROJECT_ROOT:=$${PWD}}\" && pwd)' >> $FNAME",
    "echo 'exec docker run $DOCKER_FLAG --rm --network host -v $${PROJECT_ROOT}:$${PROJECT_ROOT} -w $${PROJECT_ROOT} livadk/toshokan_build_$${CONTAINER_TYPE}$VERSION $${BINNAME} \"$$@\"' >> $FNAME",
    "chmod +x $FNAME",
  ], FNAME=name, DOCKER_FLAG=docker_flag, VERSION=version)
env.AddMethod(build_binscript, "BuildBinScript")
env.AddMethod(build_basescript, "BuildBaseScript")

# always run this when you release new version
AlwaysBuild(env.Alias('generate_tools', [
  env.BuildBaseScript('bin/base', True),
  env.BuildBinScript('bin/ar', 'intermediate'),
  env.BuildBinScript('bin/g++', 'intermediate'),
  env.BuildBinScript('bin/objcopy', 'intermediate'),
  env.BuildBinScript('bin/objdump', 'intermediate'),
  env.BuildBinScript('bin/ranlib', 'intermediate'),
  env.BuildBaseScript('tests/bin/base', True),
  env.BuildBinScript('tests/bin/addr2line', 'hakase'),
  env.BuildBinScript('tests/bin/objcopy', 'hakase'),
  env.BuildBinScript('tests/bin/objdump', 'hakase'),
  env.BuildBinScript('tests/bin/hakase-g++', 'hakase', 'g++'),
  env.BuildBinScript('tests/bin/friend-g++', 'friend', 'g++'),
  env.BuildBaseScript('tools/wrapper/base', False),
  env.BuildBinScript('tools/wrapper/addr2line', 'hakase'),
  env.BuildBinScript('tools/wrapper/objcopy', 'hakase'),
  env.BuildBinScript('tools/wrapper/objdump', 'hakase'),
  env.BuildBinScript('tools/wrapper/strip', 'hakase'),
  env.BuildBinScript('tools/wrapper/hakase-g++', 'hakase', 'g++'),
  env.BuildBinScript('tools/wrapper/friend-g++', 'friend', 'g++'),
], []))

AlwaysBuild(env.Alias('tutorial', ['generate_tools'], 'cd tutorial_template; ./build.py'))

###############################################################################
# support functions
###############################################################################
AlwaysBuild(env.Alias('monitor', '', 'docker exec -it toshokan_qemu_{0} busybox nc localhost 4445'.format(ARGUMENTS.get('SIGNATURE'))))
AlwaysBuild(env.Alias('ssh', '', 'docker exec -it toshokan_qemu_{0} ssh toshokan_qemu'.format(ARGUMENTS.get('SIGNATURE'))))
