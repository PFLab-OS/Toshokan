#!/bin/sh
docker run --rm -v $PWD:/workdir$PWD -w /workdir$PWD livadk/jinja2:ae8602a4bdc5a04cecb552d01e717233eae46fa3 python3 /workdir$PWD/rule_generator/gen_build_rules.py