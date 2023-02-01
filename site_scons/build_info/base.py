# Copyright 2016-2022 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# -*- coding: utf-8 -*-
"""Classes for building external prerequisite components"""

import os
import sys
import datetime
import json


class BuildInfo():
    """A utility class to read build information"""

    def __init__(self, filename=None):
        self.info = {}
        if filename is None:
            return
        try:
            with open(filename, "r") as info_file:
                self.info = json.load(info_file)
        except ValueError:
            print("Could not load build_info")

    def update(self, var, value):
        """save a variable in the build info"""
        self.info[var] = value

    def get(self, var, default=None):
        """Get the value of a variable from build info script"""
        return self.info.get(var, default)

    def save(self, filename):
        """Create a file to store path information for a build"""
        with open(filename, "w") as build_info:
            json.dump(self.info, build_info, skipkeys=True, indent=2)

    def gen_script(self, script_name):
        """Generate a shell script to set PATH, LD_LIBRARY_PATH,
           and PREFIX variables"""
        with open(script_name, "w") as script:
            script.write("# Automatically generated by %s at %s\n\n" %
                         (sys.argv[0], datetime.datetime.today()))

            lib_paths = []
            paths = []
            components = []

            for var in sorted(self.info.keys()):
                if not isinstance(self.info[var], str):
                    continue
                if "PREFIX" not in var:
                    continue
                if self.info[var] == "/usr":
                    continue
                script.write("SL_%s=%s\n" % (var, self.info[var]))
                components.append(var)
                path = os.path.join(self.info[var], "bin")
                lib = os.path.join(self.info[var], "lib")
                lib64 = os.path.join(self.info[var], "lib64")
                if os.path.exists(path) and path not in paths:
                    paths.insert(0, path)
                if os.path.exists(lib) and lib not in lib_paths:
                    lib_paths.insert(0, lib)
                if os.path.exists(lib64) and lib64 not in lib_paths:
                    lib_paths.insert(0, lib64)
            script.write("SL_LD_LIBRARY_PATH=%s\n" %
                         os.pathsep.join(lib_paths))
            script.write("SL_PATH=%s\n" % os.pathsep.join(paths))
            script.write('SL_COMPONENTS="%s"\n' % ' '.join(components))
            script.write("SL_BUILD_DIR=%s\n" % self.info["BUILD_DIR"])


__all__ = ["BuildInfo"]