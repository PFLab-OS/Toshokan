#!/bin/sh
mkdir -p bin
echo "#!/bin/sh
args=\"\$@\"
docker run --rm -v ${PWD}:${PWD} $1 sh -c \"cd \$PWD && g++ \$args\"" > bin/g++
chmod +x bin/g++
