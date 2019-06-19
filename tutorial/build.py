#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# INFO: You can preview the documents by running 'grip' inside of 'docs' directory.

from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))

subprocess.call('rm -rf docs', shell=True)
subprocess.call('mkdir -p docs docs/toshokan docs/paging docs/toshokan/architecture docs/toshokan/symbol_resolution docs/toshokan/function_offloading docs/toshokan/makefile docs/toshokan/monitor docs/toshokan/q_and_a', shell=True)

def copy_code(dirname):
    subprocess.call('rsync -avq code_template/* docs/{0}/'.format(dirname), shell=True)
    subprocess.call('rsync -avq {0}/*.{{cc,h}}  docs/{0}/'.format(dirname), shell=True, executable='/bin/bash')

copy_code('toshokan/symbol_resolution')
copy_code('toshokan/function_offloading')
copy_code('toshokan/monitor')

def generate(dirname):
    tpl = env.get_template('{0}/README.md.tpl'.format(dirname))

    with open('settings.yml', encoding='utf_8') as stream:
        data = yaml.load(stream, Loader=yaml.FullLoader)

    output = tpl.render(data)

    with open('./docs/{0}/README.md'.format(dirname), 'w', encoding='utf_8') as stream:
        stream.write(output)
    
    subprocess.call('ls {0}/*.svg >/dev/null 2>&1; if [ $? -eq 0 ]; then rsync -avq {0}/*.svg docs/{0}/; fi'.format(dirname), shell=True)

generate('.')
generate('paging')
generate('toshokan')
generate('toshokan/architecture')
generate('toshokan/symbol_resolution')
generate('toshokan/function_offloading')
generate('toshokan/makefile')
generate('toshokan/monitor')
generate('toshokan/q_and_a')
