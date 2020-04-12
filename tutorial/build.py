#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# INFO: You can preview the documents by running '$ grip 8080'.

from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
import glob
import os
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))

with open('build_misc/pages.yml', encoding='utf_8') as stream:
    pstruct = yaml.load(stream, Loader=yaml.FullLoader)

def generate_dir(dirname):
    subprocess.call('mkdir -p ../tutorial/' + dirname, shell=True)
    tplfile_list = glob.glob('{0}/*.tpl'.format(dirname))
    for tplfile in tplfile_list:
        tpl = env.get_template(tplfile)

        with open('build_misc/settings.yml', encoding='utf_8') as stream:
            data = yaml.load(stream, Loader=yaml.FullLoader)

        output = tpl.render(data)
        fname = os.path.splitext(os.path.basename(tplfile))[0]
        with open('../tutorial/{0}/{1}'.format(dirname, fname), 'w', encoding='utf_8') as stream:
            stream.write(output)
    
    subprocess.call('rsync -avq --exclude "*.tpl" {0}/* ../tutorial/{0}/'.format(dirname), shell=True, executable='/bin/bash')

def copy_code(dirname):
    #subprocess.call('rsync -avq code_template/* ../tutorial/{0}/'.format(dirname), shell=True)
    return

def generate(pst, prefix):
    dirname = prefix + pst['name']
    generate_dir(dirname)
    if pst.get('code') is True:
        copy_code(dirname)
    if 'pages' in pst:
        for page in pst['pages']:
            generate(page, dirname + '/')

generate(pstruct, "")
