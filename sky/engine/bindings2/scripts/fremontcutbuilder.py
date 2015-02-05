#!/usr/bin/python
# Copyright (c) 2011, the Dart project authors.  Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

import database
import databasebuilder
import logging.config
import os.path
import sys
import time

_logger = logging.getLogger('fremontcutbuilder')

def build_database(idl_files, database_dir, interfaces_info_path=None,
                   logging_level=logging.WARNING):
  """This code reconstructs the FremontCut IDL database from W3C,
  WebKit and Dart IDL files."""
  current_dir = os.path.dirname(__file__)
  logging.config.fileConfig(os.path.join(current_dir, "logging.conf"))

  _logger.setLevel(logging_level)

  db = database.Database(database_dir)

  # Delete all existing IDLs in the DB.
  db.Delete()

  builder = databasebuilder.DatabaseBuilder(db, interfaces_info_path=interfaces_info_path)

  webkit_options = databasebuilder.DatabaseBuilderOptions(
      logging_level=logging_level)

  # Import WebKit IDLs.
  builder.import_idl_files(idl_files, webkit_options)

  # Merging:
  builder.merge_imported_interfaces()

  builder.fetch_constructor_data(webkit_options)

  # Cleanup:
  builder.normalize_annotations(['WebKit', 'Dart'])

  return db
