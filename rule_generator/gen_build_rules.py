# -*- coding: utf-8 -*-
import yaml
import copy
import re
from jinja2 import Template, Environment, FileSystemLoader

def regex_replace(s, find, replace):
    return re.sub(find, replace, s)

env = Environment(loader=FileSystemLoader('.'))
root = yaml.load(open("rule_generator/dirs.yml", "r+"))
base_vars = yaml.load(open("rule_generator/base.yml", "r+"))
rules = []
clean_targets = []

env.filters['regex_replace'] = regex_replace

for dir in root["include"]:
    vars = copy.deepcopy(base_vars)
    
    data = yaml.load(open(dir + "/rules.yml", "r+"))
    for var_name in data["rules"][0]["path_extract_vars"]:
        if isinstance(data["rules"][0]["variables"][var_name], list):
            data["rules"][0]["variables"][var_name] = [dir + "/" + v for v in data["rules"][0]["variables"][var_name]]
        else:
            data["rules"][0]["variables"][var_name] = dir + "/" + data["rules"][0]["variables"][var_name]

    vars.update(data["rules"][0]["variables"])
    vars["dir"] = dir
    rules.append(env.get_template("rule_generator/" + data["rules"][0]["template"] + ".tpl").render(vars))
    clean_targets.extend(data["rules"][0]["variables"]["clean_targets"])

base_vars.update({"rules": rules})
base_vars.update({"clean_targets": clean_targets})
open("Makefile", mode='w').write(env.get_template("rule_generator/makefile.tpl").render(base_vars))
    

