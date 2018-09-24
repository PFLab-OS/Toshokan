#!/bin/sh -x

docker run --rm -v $PWD:/workdir -w /workdir -i --network toshokan_net livadk/toshokan_ssh:$1 sftp -o ConnectTimeout=3 -o LogLevel=quiet -o StrictHostKeyChecking=no -o GlobalKnownHostsFile=/dev/null -o UserKnownHostsFile=/dev/null -i /id_rsa -P 2222 hakase@toshokan_qemu <<EOF
mkdir share
put hakase/tests/test_hakase.sh share
put hakase/tests/test_library.sh share
put hakase/FriendLoader/run.sh share
put hakase/FriendLoader/friend_loader.ko share
put hakase/callback/test/callback.bin share
EOF
