#!/usr/bin/python
# Copyright (c) 2011, the Dart project authors.  Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

"""This is the entry point to create Dart APIs from the IDL database."""

import os
import sys

# Setup all paths to find our PYTHON code

# dart_dir is the location of dart's enlistment dartium (dartium-git/src/dart)
# and Dart (dart-git/dart).
dart_dir = os.path.abspath(os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..', '..')))

# Dartium's third_party directory location is dartium-git/src/third_party
# and Dart's third_party directory location is dart-git/dart/third_party.
third_party_dir = os.path.join(dart_dir, 'third_party')

ply_dir = os.path.join(third_party_dir, 'ply')
# If ply directory found then we're a Dart enlistment; third_party location
# is dart-git/dart/third_party
if not os.path.exists(ply_dir):
  # For Dartium (ply directory is dartium-git/src/third_party/ply) third_party
  # location is dartium-git/src/third_party
  third_party_dir = os.path.join(dart_dir, '..', 'third_party')
  assert(os.path.exists(third_party_dir))
sys.path.insert(1, third_party_dir)


import dartgenerator
import database
import fremontcutbuilder
import logging
import multiemitter
import optparse
import shutil
import subprocess
import time
from generator import TypeRegistry
from htmleventgenerator import HtmlEventGenerator
from htmlrenamer import HtmlRenamer
from systemhtml import DartLibraryEmitter,\
                       HtmlDartInterfaceGenerator, DartLibrary, DartLibraries,\
                       HTML_LIBRARY_NAMES
from systemnative import CPPLibraryEmitter, DartiumBackend
from templateloader import TemplateLoader

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))

import utils


_logger = logging.getLogger('dartdomgenerator')

class GeneratorOptions(object):
  def __init__(self, templates, database, type_registry, renamer):
    self.templates = templates
    self.database = database
    self.type_registry = type_registry
    self.renamer = renamer

def LoadDatabase(database_dir, use_database_cache):
  common_database = database.Database(database_dir)
  if use_database_cache:
    common_database.LoadFromCache()
  else:
    common_database.Load()
  return common_database

def GenerateFromDatabase(common_database, dart2js_output_dir,
                         dartium_output_dir, update_dom_metadata=False,
                         logging_level=logging.WARNING):
  start_time = time.time()

  current_dir = os.path.dirname(__file__)
  auxiliary_dir = os.path.join(current_dir, '..', 'src')
  template_dir = os.path.join(current_dir, '..', 'templates')

  _logger.setLevel(logging_level)

  generator = dartgenerator.DartGenerator(logging_level)
  generator.LoadAuxiliary(auxiliary_dir)

  generator.FilterMembersWithUnidentifiedTypes(common_database)
  webkit_database = common_database.Clone()

  # Generate Dart interfaces for the WebKit DOM.
  generator.FilterInterfaces(database = webkit_database,
                             or_annotations = ['WebKit', 'Dart'],
                             exclude_displaced = ['WebKit'],
                             exclude_suppressed = ['WebKit', 'Dart'])
  generator.FixEventTargets(webkit_database)
  generator.AddMissingArguments(webkit_database)

  emitters = multiemitter.MultiEmitter(logging_level)
  renamer = HtmlRenamer(webkit_database)
  type_registry = TypeRegistry(webkit_database, renamer)

  print 'GenerateFromDatabase %s seconds' % round((time.time() - start_time), 2)

  def RunGenerator(dart_libraries, dart_output_dir,
                   template_loader, backend_factory):
    options = GeneratorOptions(
        template_loader, webkit_database, type_registry, renamer)
    dart_library_emitter = DartLibraryEmitter(
        emitters, dart_output_dir, dart_libraries)
    event_generator = HtmlEventGenerator(webkit_database, renamer, template_loader)

    def generate_interface(interface):
      backend = backend_factory(interface)
      interface_generator = HtmlDartInterfaceGenerator(
          options, dart_library_emitter, event_generator, interface, backend)
      interface_generator.Generate()

    generator.Generate(webkit_database, common_database, generate_interface)

    dart_library_emitter.EmitLibraries(auxiliary_dir)

  if dartium_output_dir:
    template_paths = ['html/dartium', 'html/impl', 'html/interface', '']
    template_loader = TemplateLoader(template_dir,
                                     template_paths,
                                     {'DARTIUM': True, 'DART2JS': False})
    backend_options = GeneratorOptions(
        template_loader, webkit_database, type_registry, renamer)
    cpp_output_dir = os.path.join(dartium_output_dir, 'cpp')
    cpp_library_emitter = CPPLibraryEmitter(emitters, cpp_output_dir)
    dart_output_dir = os.path.join(dartium_output_dir, 'dart')
    backend_factory = lambda interface:\
        DartiumBackend(interface, cpp_library_emitter, backend_options)
    dart_libraries = DartLibraries(
        HTML_LIBRARY_NAMES, template_loader, 'dartium', dartium_output_dir)

    print '\nGenerating dartium:\n'
    start_time = time.time()

    RunGenerator(dart_libraries, dart_output_dir,
                 template_loader, backend_factory)
    print 'Generated dartium in %s seconds' % round(time.time() - start_time, 2)

    cpp_library_emitter.EmitResolver(
        template_loader.Load('cpp_resolver.template'), dartium_output_dir)
    cpp_library_emitter.EmitClassIdTable(
        webkit_database, dartium_output_dir, type_registry, renamer)
    emitters.Flush()


def GenerateSingleFile(library_path, output_dir, generated_output_dir=None):
  library_dir = os.path.dirname(library_path)
  library_filename = os.path.basename(library_path)
  copy_dart_script = os.path.relpath('../../copy_dart.py',
      library_dir)
  output_dir = os.path.relpath(output_dir, library_dir)
  command = ' '.join(['cd', library_dir, ';',
                      copy_dart_script, output_dir, library_filename])
  subprocess.call([command], shell=True)
