#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import dartdomgenerator
import fremontcutbuilder
import os.path
import sys
import tempfile
import argparse


def main(argv):
    parser = argparse.ArgumentParser(description='Generate .dart from .idl')
    parser.add_argument('idl_list_path')
    parser.add_argument('interfaces_info_path')
    parser.add_argument('output_directory')
    args = parser.parse_args()

    with open(args.idl_list_path, 'r') as idl_list_file:
        idl_files = [line.strip() for line in idl_list_file]

    # Get all ASTs.

    database = fremontcutbuilder.build_database(idl_files, tempfile.mkdtemp(), interfaces_info_path=args.interfaces_info_path)
    database.Load()
    dartdomgenerator.GenerateFromDatabase(database, None, args.output_directory)
    database.Delete()


if __name__ == '__main__':
    sys.exit(main(sys.argv))
