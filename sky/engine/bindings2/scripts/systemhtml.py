#!/usr/bin/python
# Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

"""This module provides shared functionality for the system to generate
Dart:html APIs from the IDL database."""

import emitter
import logging
import monitored
import os
import re
from generator import *
from htmldartgenerator import *

_logger = logging.getLogger('systemhtml')

HTML_LIBRARY_NAMES = ['html', 'web_gl']


# Classes that offer only static methods, and therefore we should suppress
# constructor creation.
_static_classes = set(['Url'])

# Information for generating element constructors.
#
# TODO(sra): maybe remove all the argument complexity and use cascades.
#
#   var c = new CanvasElement(width: 100, height: 70);
#   var c = new CanvasElement()..width = 100..height = 70;
#
class ElementConstructorInfo(object):
  def __init__(self, name=None, tag=None,
               params=[], opt_params=[],
               factory_provider_name='document'):
    self.name = name          # The constructor name 'h1' in 'HeadingElement.h1'
    self.tag = tag or name    # The HTML or SVG tag
    self.params = params
    self.opt_params = opt_params
    self.factory_provider_name = factory_provider_name

  def ConstructorInfo(self, interface_name):
    info = OperationInfo()
    info.overloads = None
    info.declared_name = interface_name
    info.name = interface_name
    info.constructor_name = self.name
    info.js_name = None
    info.type_name = interface_name
    info.param_infos = map(lambda tXn: ParamInfo(tXn[1], tXn[0], True),
                           self.opt_params)
    info.requires_named_arguments = True
    info.factory_parameters = ['"%s"' % self.tag]
    info.pure_dart_constructor = True
    return info

_html_element_constructors = monitored.Dict(
      'systemhtml._html_element_constructors', {
  'HTMLAnchorElement' :
    ElementConstructorInfo(tag='a', opt_params=[('DOMString', 'href')]),
  'HTMLCanvasElement':
    ElementConstructorInfo(tag='canvas',
                           opt_params=[('int', 'width'), ('int', 'height')]),
  'HTMLContentElement': 'content',
  'HTMLDivElement': 'div',
  'HTMLIFrameElement': 'iframe',
  'HTMLImageElement':
    ElementConstructorInfo(tag='img',
                           opt_params=[('DOMString', 'src'),
                                       ('int', 'width'), ('int', 'height')]),
  'HTMLMetaElement': 'meta',
  'HTMLScriptElement': 'script',
  'HTMLStyleElement': 'style',
  'HTMLTemplateElement': 'template',
  'HTMLTitleElement': 'title',
})

_element_constructors = {
  'html': _html_element_constructors,
  'typed_data': {},
  'web_gl': {},
}

_factory_ctr_strings = {
  'html': {
      'provider_name': 'document',
      'constructor_name': 'createElement'
  },
  'typed_data': {
      'provider_name': 'document',
      'constructor_name': 'createElement'
  },
  'web_gl': {
    'provider_name': 'document',
    'constructor_name': 'createElement'
  },
}

def ElementConstructorInfos(typename, element_constructors,
                            factory_provider_name='_Elements'):
  """Returns list of ElementConstructorInfos about the convenience constructors
  for an Element or SvgElement."""
  # TODO(sra): Handle multiple and named constructors.
  if typename not in element_constructors:
    return []
  infos = element_constructors[typename]
  if isinstance(infos, str):
    infos = ElementConstructorInfo(tag=infos,
        factory_provider_name=factory_provider_name)
  if not isinstance(infos, list):
    infos = [infos]
  return infos

# ------------------------------------------------------------------------------

class HtmlDartInterfaceGenerator(object):
  """Generates dart interface and implementation for the DOM IDL interface."""

  def __init__(self, options, library_emitter, event_generator, interface,
               backend):
    self._renamer = options.renamer
    self._database = options.database
    self._template_loader = options.templates
    self._type_registry = options.type_registry
    self._options = options
    self._library_emitter = library_emitter
    self._event_generator = event_generator
    self._interface = interface
    self._backend = backend
    self._interface_type_info = self._type_registry.TypeInfo(self._interface.id)
    self._library_name = self._renamer.GetLibraryName(self._interface)

  def Generate(self):
    if IsCustomType(self._interface.id):
      pass
    elif 'Callback' in self._interface.ext_attrs:
      self.GenerateCallback()
    else:
      self.GenerateInterface()

  def GenerateCallback(self):
    """Generates a typedef for the callback interface."""
    typedef_name = self._renamer.RenameInterface(self._interface)
    if not typedef_name:
      return

    info = GetCallbackInfo(self._interface)
    code = self._library_emitter.FileEmitter(self._interface.id,
        self._library_name)
    code.Emit(self._template_loader.Load('callback.darttemplate'))

    code.Emit('$(ANNOTATIONS)typedef void $NAME($PARAMS);\n',
              ANNOTATIONS=None,
              NAME=typedef_name,
              PARAMS=info.ParametersAsDeclaration(self._DartType))
    self._backend.GenerateCallback(info)

  def GenerateInterface(self):
    interface_name = self._interface_type_info.interface_name()
    implementation_name = self._interface_type_info.implementation_name()
    self._library_emitter.AddTypeEntry(self._library_name,
                                       self._interface.id, implementation_name)

    factory_provider = None
    if interface_name in interface_factories:
      factory_provider = interface_factories[interface_name]
    factory_constructor_name = None

    constructors = []
    if interface_name in _static_classes:
      constructor_info = None
    else:
      constructor_info = AnalyzeConstructor(self._interface)
    if constructor_info:
      constructors.append(constructor_info)
      # TODO(antonm): consider removing it later.
      factory_provider = interface_name

    # HTML Elements and SVG Elements have convenience constructors.
    infos = ElementConstructorInfos(self._interface.id,
        _element_constructors[self._library_name], factory_provider_name=
        _factory_ctr_strings[self._library_name]['provider_name'])

    if infos:
      factory_constructor_name = _factory_ctr_strings[
          self._library_name]['constructor_name']

    for info in infos:
      constructors.append(info.ConstructorInfo(self._interface.id))
      if factory_provider and factory_provider != info.factory_provider_name:
        _logger.warn('Conflicting factory provider names: %s != %s' %
          (factory_provider, info.factory_provider_name))
      factory_provider = info.factory_provider_name

    implementation_emitter = self._ImplementationEmitter()

    base_type_info = None
    if self._interface.parents:
      supertype = self._interface.parents[0].type.id
      if not IsDartCollectionType(supertype) and not IsPureInterface(supertype):
        base_type_info = self._type_registry.TypeInfo(supertype)

    if base_type_info:
      base_class = base_type_info.implementation_name()
    else:
      base_class = self._backend.RootClassName()

    implements = self._backend.AdditionalImplementedInterfaces()
    for parent in self._interface.parents:
      parent_type_info = self._type_registry.TypeInfo(parent.type.id)
      if parent_type_info.interface_name() != base_class and \
          parent_type_info != base_type_info:
        implements.append(parent_type_info.interface_name())

    secure_base_name = self._backend.SecureBaseName(interface_name)
    if secure_base_name:
      implements.append(secure_base_name)

    implements_str = ''
    if implements:
      implements_str = ' implements ' + ', '.join(set(implements))

    mixins = self._backend.Mixins()
    mixins_str = ''
    if mixins:
      mixins_str = ' with ' + ', '.join(mixins)
      if not base_class:
        base_class = 'Interceptor'

    class_modifiers = ''
    if (self._renamer.ShouldSuppressInterface(self._interface) or
        IsPureInterface(self._interface.id)):
      class_modifiers = 'abstract '

    native_spec = ''
    if not IsPureInterface(self._interface.id):
      native_spec = self._backend.NativeSpec()

    implementation_members_emitter = implementation_emitter.Emit(
        self._backend.ImplementationTemplate(),
        LIBRARYNAME='dart.dom.%s' % self._library_name,
        ANNOTATIONS=None,
        CLASS_MODIFIERS=class_modifiers,
        CLASSNAME=self._interface_type_info.implementation_name(),
        EXTENDS=' extends %s' % base_class if base_class else '',
        IMPLEMENTS=implements_str,
        MIXINS=mixins_str,
        DOMNAME=self._interface.doc_js_name,
        NATIVESPEC=native_spec)
    stream_getter_signatures_emitter = None
    element_stream_getters_emitter = None
    if type(implementation_members_emitter) == tuple:
        # We add event stream getters for both Element and ElementList, so in
        # impl_Element.darttemplate, we have two additional "holes" for emitters
        # to fill in, with small variations. These store these specialized
        # emitters.
        assert len(implementation_members_emitter) == 3;
        stream_getter_signatures_emitter = \
            implementation_members_emitter[0]
        element_stream_getters_emitter = implementation_members_emitter[1]
        implementation_members_emitter = \
            implementation_members_emitter[2]
    self._backend.StartInterface(implementation_members_emitter)
    self._backend.EmitHelpers(base_class)
    self._event_generator.EmitStreamProviders(
        self._interface,
        self._backend.CustomJSMembers(),
        implementation_members_emitter,
        self._library_name)
    self._backend.AddConstructors(
        constructors, factory_provider, factory_constructor_name)

    isElement = False
    for parent in self._database.Hierarchy(self._interface):
      if parent.id == 'Element':
        isElement = True
    if isElement and self._interface.id != 'Element':
      implementation_members_emitter.Emit(
          '  /**\n'
          '   * Constructor instantiated by the DOM when a custom element has been created.\n'
          '   *\n'
          '   * This can only be called by subclasses from their created constructor.\n'
          '   */\n'
          '  $CLASSNAME.created() : super.created();\n',
          CLASSNAME=self._interface_type_info.implementation_name())

    merged_interface = self._interface_type_info.merged_interface()
    if merged_interface:
      self._backend.AddMembers(self._database.GetInterface(merged_interface),
        not self._backend.ImplementsMergedMembers())

    self._backend.AddMembers(self._interface)
    self._backend.AddSecondaryMembers(self._interface)
    self._event_generator.EmitStreamGetters(
        self._interface,
        [],
        implementation_members_emitter,
        self._library_name, stream_getter_signatures_emitter,
        element_stream_getters_emitter)
    self._backend.FinishInterface()

  def _ImplementationEmitter(self):
    basename = self._interface_type_info.implementation_name()
    if (self._interface_type_info.merged_into() and
        self._backend.ImplementsMergedMembers()):
      # Merged members are implemented in target interface implementation.
      return emitter.Emitter()
    return self._library_emitter.FileEmitter(basename, self._library_name)

  def _DartType(self, type_name):
    return self._type_registry.DartType(type_name)


# ------------------------------------------------------------------------------

class DartLibraryEmitter():
  def __init__(self, multiemitter, dart_sources_dir, dart_libraries):
    self._multiemitter = multiemitter
    self._dart_sources_dir = dart_sources_dir
    self._path_to_emitter = {}
    self._dart_libraries = dart_libraries

  def FileEmitter(self, basename, library_name, template=None):
    aux_dir = os.path.join(self._dart_sources_dir, library_name)
    path = os.path.join(aux_dir, '%s.dart' % basename)
    if not path in self._path_to_emitter:
      emitter = self._multiemitter.FileEmitter(path)
      if not template is None:
        emitter = emitter.Emit(template)
      self._path_to_emitter[path] = emitter

      self._dart_libraries.AddFile(basename, library_name, path)
    return self._path_to_emitter[path]

  def AddTypeEntry(self, basename, idl_name, dart_name):
    self._dart_libraries.AddTypeEntry(basename, idl_name, dart_name)

  def EmitLibraries(self, auxiliary_dir):
    self._dart_libraries.Emit(self._multiemitter, auxiliary_dir)

# ------------------------------------------------------------------------------
class DartLibrary():
  def __init__(self, name, template_loader, library_type, output_dir):
    self._template = template_loader.Load(
        '%s_%s.darttemplate' % (name, library_type))
    self._dart_path = os.path.join(
        output_dir, '%s_%s.dart' % (name, library_type))
    self._paths = []
    self._typeMap = {}

  def AddFile(self, path):
    self._paths.append(path)

  def AddTypeEntry(self, idl_name, dart_name):
    self._typeMap[idl_name] = dart_name

  def Emit(self, emitter, auxiliary_dir):
    def massage_path(path):
      # The most robust way to emit path separators is to use / always.
      return path.replace('\\', '/')

    library_emitter = emitter.FileEmitter(self._dart_path)
    library_file_dir = os.path.dirname(self._dart_path)
    auxiliary_dir = os.path.relpath(auxiliary_dir, library_file_dir)
    emitters = library_emitter.Emit(
        self._template, AUXILIARY_DIR=massage_path(auxiliary_dir))
    if isinstance(emitters, tuple):
      imports_emitter, map_emitter = emitters
    else:
      imports_emitter, map_emitter = emitters, None


    for path in sorted(self._paths):
      relpath = os.path.relpath(path, library_file_dir)
      imports_emitter.Emit(
          "part '$PATH';\n", PATH=massage_path(relpath))

    if map_emitter:
      items = self._typeMap.items()
      items.sort()
      for (idl_name, dart_name) in items:
        map_emitter.Emit(
          "  '$IDL_NAME': () => $DART_NAME,\n",
          IDL_NAME=idl_name,
          DART_NAME=dart_name)


# ------------------------------------------------------------------------------

class DartLibraries():
  def __init__(self, libraries, template_loader, library_type, output_dir):
    self._libraries = {}
    for library_name in libraries:
      self._libraries[library_name] = DartLibrary(
          library_name, template_loader, library_type, output_dir)

  def AddFile(self, basename, library_name, path):
    self._libraries[library_name].AddFile(path)

  def AddTypeEntry(self, library_name, idl_name, dart_name):
    self._libraries[library_name].AddTypeEntry(idl_name, dart_name)

  def Emit(self, emitter, auxiliary_dir):
    for lib in self._libraries.values():
      lib.Emit(emitter, auxiliary_dir)
