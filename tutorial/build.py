#! /usr/bin/env python3
# -*- coding: utf-8 -*-
from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))

subprocess.call('rm -rf docs', shell=True)
subprocess.call('mkdir -p docs docs/paging', shell=True)

def generate(ifile, ofile):
    tpl = env.get_template(ifile)

    with open('settings.yml', encoding='utf_8') as stream:
        data = yaml.load(stream, Loader=yaml.FullLoader)

    output = tpl.render(data)

    with open(ofile, 'w', encoding='utf_8') as stream:
        stream.write(output)

generate('README.md.tpl', './docs/README.md')
generate('paging/README.md.tpl', './docs/paging/README.md')
