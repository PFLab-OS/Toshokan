#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# INFO: You can preview the documents by running 'grip' inside of 'docs' directory.

from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))

subprocess.call('rm -rf docs', shell=True)
subprocess.call('mkdir -p docs docs/toshokan docs/paging docs/toshokan/architecture docs/toshokan/symbol_resolution docs/toshokan/function_offloading', shell=True)

def copy_code(dirname):
    subprocess.call('cp -r code_template/* docs/{0}/'.format(dirname), shell=True)
    subprocess.call('cp -r {0}/*.{{cc,h}}  docs/{0}/'.format(dirname), shell=True)

copy_code('toshokan/symbol_resolution')
copy_code('toshokan/function_offloading')

def generate_sub(ifile, ofile):
    tpl = env.get_template(ifile)

    with open('settings.yml', encoding='utf_8') as stream:
        data = yaml.load(stream, Loader=yaml.FullLoader)

    output = tpl.render(data)

    with open(ofile, 'w', encoding='utf_8') as stream:
        stream.write(output)

def generate(dirname):
    generate_sub('{0}/README.md.tpl'.format(dirname), './docs/{0}/README.md'.format(dirname))

generate('.')
generate('paging')
generate('toshokan')
generate('toshokan/architecture')
generate('toshokan/symbol_resolution')
generate('toshokan/function_offloading')
