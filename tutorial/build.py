#! /usr/bin/env python3
# -*- coding: utf-8 -*-
from jinja2 import Environment, FileSystemLoader
import yaml
import subprocess
env = Environment(loader=FileSystemLoader('./', encoding='utf8'))
tpl = env.get_template('README.md.tpl')

with open('settings.yml', encoding='utf_8') as stream:
    data = yaml.load(stream, Loader=yaml.FullLoader)

output = tpl.render(data)

subprocess.call('rm -rf docs', shell=True)
subprocess.call('mkdir -p docs', shell=True)

with open('./docs/README.md', 'w', encoding='utf_8') as stream:
    stream.write(output)
