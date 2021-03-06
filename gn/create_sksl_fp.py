#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

def copy(src, dst):
    for line in src.readlines():
        if not line.startswith("#"):
            dst.write(line)
    src.close()

dst = open(sys.argv[3], 'wb')
dst.write("// *********************************************************\n")
dst.write("// ***  AUTOGENERATED BY create_sksl_fp.py, DO NOT EDIT  ***\n")
dst.write("// *********************************************************\n\n\n")
copy(open(sys.argv[1], 'r'), dst)
copy(open(sys.argv[2], 'r'), dst)
dst.close()
