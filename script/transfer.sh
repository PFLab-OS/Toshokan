#!/bin/sh -e

SFTP_COMMAND=$1
shift
echo $@
COMMAND=`for f in $@; do echo "put $f share"; done`
$SFTP_COMMAND <<EOF
mkdir share
put hakase/tests/test_hakase.sh share
put hakase/tests/test_library.sh share
put hakase/FriendLoader/run.sh share
put hakase/FriendLoader/friend_loader.ko share
$COMMAND
EOF
