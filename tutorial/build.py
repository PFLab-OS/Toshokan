#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# INFO: You can preview the documents by running 'grip' inside of 'docs' directory.

from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))

with open('pages.yml', encoding='utf_8') as stream:
    pstruct = yaml.load(stream, Loader=yaml.FullLoader)

subprocess.call('rm -rf docs', shell=True)

def generate_dir(dirname):
    subprocess.call('mkdir -p docs/' + dirname, shell=True)
    tpl = env.get_template('{0}/README.md.tpl'.format(dirname))

    with open('settings.yml', encoding='utf_8') as stream:
        data = yaml.load(stream, Loader=yaml.FullLoader)

    output = tpl.render(data)

    with open('./docs/{0}/README.md'.format(dirname), 'w', encoding='utf_8') as stream:
        stream.write(output)
    
    subprocess.call('ls {0}/*.svg >/dev/null 2>&1; if [ $? -eq 0 ]; then rsync -avq {0}/*.svg docs/{0}/; fi'.format(dirname), shell=True)

def copy_code(dirname):
    subprocess.call('rsync -avq code_template/* docs/{0}/'.format(dirname), shell=True)
    subprocess.call('rsync -avq {0}/*.{{cc,h}}  docs/{0}/'.format(dirname), shell=True, executable='/bin/bash')

def generate(pst, prefix):
    dirname = prefix + pst['name']
    generate_dir(dirname)
    if pst.get('code') is True:
        copy_code(dirname)
    if 'pages' in pst:
        for page in pst['pages']:
            generate(page, dirname + '/')

generate(pstruct, "")
