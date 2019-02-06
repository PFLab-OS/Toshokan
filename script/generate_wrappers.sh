#!/bin/sh
mkdir -p bin
echo "#!/bin/sh
docker run --rm -v ${PWD}:${PWD} -w ${PWD} $1 g++ \"\$@\"" > bin/g++
chmod +x bin/g++
