#!/usr/bin/env python3

##############################################################################
# version.py - get application version for build systems                     #
#                                                                            #
# Copyright (c) 2019 Alexey Parfenov <zxed@alkatrazstudio.net>               #
#                                                                            #
# This library is free software: you can redistribute it and/or modify it    #
# under the terms of the GNU General Public License as published by          #
# the Free Software Foundation, either version 3 of the License,             #
# or (at your option) any later version.                                     #
#                                                                            #
# This library is distributed in the hope that it will be useful,            #
# but WITHOUT ANY WARRANTY; without even the implied warranty of             #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           #
# General Public License for more details: https://gnu.org/licenses/gpl.html #
##############################################################################

import sys
import os
import shutil
import subprocess
import re


def log(s):
    print(s, file=sys.stderr)


def get_git_info(git_exe):
    args = [git_exe, "describe"]
    try:
        stdout = subprocess.run(
            args,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
            universal_newlines=True
        ).stdout.strip()
    except Exception as e:
        raise RuntimeError("{}: {}".format(" ".join(args), e))

    # possible results:
    # - v1.2
    # - v1.2-1-g8f4e68a
    # - v1.2.3-1-g8f4e68a
    m = re.match(r"^v(\d+)\.(\d+)(?:[-.](\d+))?", stdout)
    if not m:
        raise RuntimeError("Cannot parse {} output: {}".format(" ".join(args), stdout))
    return [int(x) for x in m.groups("0")]


def get_file_info(filename):
    try:
        with open(filename) as version_file:
            content = version_file.read().strip()
    except Exception as e:
        raise RuntimeError("open: {}".format(e))
    parts = content.split(".")
    parts_count = 3
    parts += ["0"] * (parts_count - len(parts))  # pad array with zeroes
    return [int(x) for x in parts[:parts_count]]


#
# move to the parent's project dir
#
os.chdir(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", ".."))


#
# get version info
#
ver_maj = 0
ver_min = 0
ver_pat = 0

filename = "VERSION"
if os.path.isfile(filename):
    try:
        ver_maj, ver_min, ver_pat = get_file_info(filename)
    except Exception as e:
        log("Ignoring file: {}".format(e))
elif os.path.isdir(".git"):
    git_exe = shutil.which("git")
    if git_exe is None:
        log("Ignoring Git tags: Git not found.")
    else:
        try:
            ver_maj, ver_min, ver_pat = get_git_info(git_exe)
        except Exception as e:
            log("Ignoring Git tags: {}".format(e))


#
# output version number string
#
ver = "{}.{}.{}".format(ver_maj, ver_min, ver_pat)
sys.stdout.write(ver)  # use sys.stdout.write() instead of print() to not add a newline
