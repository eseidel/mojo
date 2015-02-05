#!/usr/bin/python
# Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
# for details. All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

"""This module provides shared functionality to provide Dart metadata for
DOM APIs.
"""

from generator import ConstantOutputOrder
import copy
import json
import logging
import monitored
import os
import re

_logger = logging.getLogger('dartmetadata')

# Annotations to be placed on native members.  The table is indexed by the IDL
# interface and member name, and by IDL return or field type name.  Both are
# used to assemble the annotations:
#
#   INTERFACE.MEMBER: annotations for member.
#   +TYPE:            add annotations only if there are member annotations.
#   -TYPE:            add annotations only if there are no member annotations.
#   TYPE:             add regardless of member annotations.

_blink_experimental_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@Experimental()",
]

_indexed_db_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.FIREFOX, '15')",
  "@SupportedBrowser(SupportedBrowser.IE, '10')",
  "@Experimental()",
]

_file_system_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@Experimental()",
]

_all_but_ie9_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.FIREFOX)",
  "@SupportedBrowser(SupportedBrowser.IE, '10')",
  "@SupportedBrowser(SupportedBrowser.SAFARI)",
]

_history_annotations = _all_but_ie9_annotations

_no_ie_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.FIREFOX)",
  "@SupportedBrowser(SupportedBrowser.SAFARI)",
]

_performance_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.FIREFOX)",
  "@SupportedBrowser(SupportedBrowser.IE)",
]

_rtc_annotations = [ # Note: Firefox nightly builds also support this.
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@Experimental()",
]

_shadow_dom_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME, '26')",
  "@Experimental()",
]

_speech_recognition_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME, '25')",
  "@Experimental()",
]

_svg_annotations = _all_but_ie9_annotations;

_web_sql_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.SAFARI)",
  "@Experimental()",
]

_webgl_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.FIREFOX)",
  "@Experimental()",
]

_web_audio_annotations = _webgl_annotations

_webkit_experimental_annotations = [
  "@SupportedBrowser(SupportedBrowser.CHROME)",
  "@SupportedBrowser(SupportedBrowser.SAFARI)",
  "@Experimental()",
]

# Annotations to be placed on generated members.
# The table is indexed as:
#   INTERFACE:     annotations to be added to the interface declaration
#   INTERFACE.MEMBER: annotation to be added to the member declaration
_annotations = monitored.Dict('dartmetadata._annotations', {
  'CSSHostRule': _shadow_dom_annotations,
  'WebKitCSSMatrix': _webkit_experimental_annotations,
  'Crypto': _webkit_experimental_annotations,
  'Database': _web_sql_annotations,
  'DatabaseSync': _web_sql_annotations,
  'ApplicationCache': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.FIREFOX)",
    "@SupportedBrowser(SupportedBrowser.IE, '10')",
    "@SupportedBrowser(SupportedBrowser.OPERA)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
  'AudioBufferSourceNode': _web_audio_annotations,
  'AudioContext': _web_audio_annotations,
  'DOMFileSystem': _file_system_annotations,
  'DOMFileSystemSync': _file_system_annotations,
  'WebKitPoint': _webkit_experimental_annotations,
  'Window.indexedDB': _indexed_db_annotations,
  'Window.openDatabase': _web_sql_annotations,
  'Window.performance': _performance_annotations,
  'Window.webkitNotifications': _webkit_experimental_annotations,
  'Window.webkitRequestFileSystem': _file_system_annotations,
  'Window.webkitResolveLocalFileSystemURL': _file_system_annotations,
  'Element.createShadowRoot': [
    "@SupportedBrowser(SupportedBrowser.CHROME, '25')",
    "@Experimental()",
  ],
  'Element.ontransitionend': _all_but_ie9_annotations,
  # Placeholder to add experimental flag, implementation for this is
  # pending in a separate CL.
  'Element.webkitMatchesSelector': ['@Experimental()'],
  'Event.clipboardData': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.FIREFOX)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
    "@Experimental()",
  ],
  'FormData': _all_but_ie9_annotations,
  'HashChangeEvent': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.FIREFOX)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
  'History.pushState': _history_annotations,
  'History.replaceState': _history_annotations,
  'HTMLContentElement': _shadow_dom_annotations,
  'HTMLDataListElement': _all_but_ie9_annotations,
  'HTMLDetailsElement': _webkit_experimental_annotations,
  'HTMLEmbedElement': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.IE)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
  'HTMLKeygenElement': _webkit_experimental_annotations,
  'HTMLMeterElement': _no_ie_annotations,
  'HTMLObjectElement': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.IE)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
  'HTMLOutputElement': _no_ie_annotations,
  'HTMLProgressElement': _all_but_ie9_annotations,
  'HTMLShadowElement': _shadow_dom_annotations,
  'HTMLTemplateElement': _blink_experimental_annotations,
  'HTMLTrackElement': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.IE, '10')",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
  'IDBFactory': _indexed_db_annotations,
  'IDBDatabase': _indexed_db_annotations,
  'MediaStream': _rtc_annotations,
  'MediaStreamEvent': _rtc_annotations,
  'MediaStreamTrack': _rtc_annotations,
  'MediaStreamTrackEvent': _rtc_annotations,
  'MutationObserver': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.FIREFOX)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
    "@Experimental()",
  ],
  'NotificationCenter': _webkit_experimental_annotations,
  'Performance': _performance_annotations,
  'PopStateEvent': _history_annotations,
  'RTCIceCandidate': _rtc_annotations,
  'RTCPeerConnection': _rtc_annotations,
  'RTCSessionDescription': _rtc_annotations,
  'ShadowRoot': _shadow_dom_annotations,
  'SpeechRecognition': _speech_recognition_annotations,
  'SpeechRecognitionAlternative': _speech_recognition_annotations,
  'SpeechRecognitionError': _speech_recognition_annotations,
  'SpeechRecognitionEvent': _speech_recognition_annotations,
  'SpeechRecognitionResult': _speech_recognition_annotations,
  'SVGAltGlyphElement': _no_ie_annotations,
  'SVGAnimateElement': _no_ie_annotations,
  'SVGAnimateMotionElement': _no_ie_annotations,
  'SVGAnimateTransformElement': _no_ie_annotations,
  'SVGFEBlendElement': _svg_annotations,
  'SVGFEColorMatrixElement': _svg_annotations,
  'SVGFEComponentTransferElement': _svg_annotations,
  'SVGFEConvolveMatrixElement': _svg_annotations,
  'SVGFEDiffuseLightingElement': _svg_annotations,
  'SVGFEDisplacementMapElement': _svg_annotations,
  'SVGFEDistantLightElement': _svg_annotations,
  'SVGFEFloodElement': _svg_annotations,
  'SVGFEFuncAElement': _svg_annotations,
  'SVGFEFuncBElement': _svg_annotations,
  'SVGFEFuncGElement': _svg_annotations,
  'SVGFEFuncRElement': _svg_annotations,
  'SVGFEGaussianBlurElement': _svg_annotations,
  'SVGFEImageElement': _svg_annotations,
  'SVGFEMergeElement': _svg_annotations,
  'SVGFEMergeNodeElement': _svg_annotations,
  'SVGFEMorphologyElement': _svg_annotations,
  'SVGFEOffsetElement': _svg_annotations,
  'SVGFEPointLightElement': _svg_annotations,
  'SVGFESpecularLightingElement': _svg_annotations,
  'SVGFESpotLightElement': _svg_annotations,
  'SVGFETileElement': _svg_annotations,
  'SVGFETurbulenceElement': _svg_annotations,
  'SVGFilterElement': _svg_annotations,
  'SVGForeignObjectElement': _no_ie_annotations,
  'SVGSetElement': _no_ie_annotations,
  'SQLTransaction': _web_sql_annotations,
  'SQLTransactionSync': _web_sql_annotations,
  'WebGLRenderingContext': _webgl_annotations,
  'WebSocket': _all_but_ie9_annotations,
  'Worker': _all_but_ie9_annotations,
  'XMLHttpRequest.overrideMimeType': _no_ie_annotations,
  'XMLHttpRequest.response': _all_but_ie9_annotations,
  'XMLHttpRequestEventTarget.onloadend': _all_but_ie9_annotations,
  'XMLHttpRequestEventTarget.onprogress': _all_but_ie9_annotations,
  'XSLTProcessor': [
    "@SupportedBrowser(SupportedBrowser.CHROME)",
    "@SupportedBrowser(SupportedBrowser.FIREFOX)",
    "@SupportedBrowser(SupportedBrowser.SAFARI)",
  ],
})

# TODO(blois): minimize noise and enable by default.
_monitor_type_metadata = False

class DartMetadata(object):
  def __init__(self, logging_level=logging.WARNING):
    _logger.setLevel(logging_level)
    self._api_status_path = None
    self._types = {}
    self._doc_comments = {}

    if _monitor_type_metadata:
      monitored_interfaces = {}
      for interface_id, interface_data in self._types.iteritems():
        monitored_interface = interface_data.copy()
        monitored_interface['members'] = monitored.Dict(
            'dartmetadata.%s' % interface_id, interface_data['members'])

        monitored_interfaces[interface_id] = monitored_interface

      self._monitored_types = monitored.Dict('dartmetadata._monitored_types',
          monitored_interfaces)
    else:
      self._monitored_types = self._types

  def GetFormattedMetadata(self, library_name, interface, member_id=None,
      indentation=''):
    """ Gets all comments and annotations for an interface or member.
    """
    return self.FormatMetadata(
        self.GetMetadata(library_name, interface, member_id),
        indentation)

  def GetMetadata(self, library_name, interface,
        member_name=None, source_member_name=None):
    """ Gets all comments and annotations for an interface or member.

    Args:
      source_member_name: If the member is dependent on a different member
        then this is used to apply the support annotations from the other
        member.
    """
    annotations = self._GetComments(library_name, interface, member_name)
    annotations = annotations + self._GetCommonAnnotations(
        interface, member_name, source_member_name)

    return annotations

  def GetDart2JSMetadata(self, idl_type, library_name,
      interface, member_name,):
    return self.GetMetadata(library_name, interface, member_name)

  def IsSuppressed(self, interface, member_name):
    annotations = self._GetSupportLevelAnnotations(interface.id, member_name)
    return any(
        annotation.startswith('@removed') for annotation in annotations)

  def _GetCommonAnnotations(self, interface, member_name=None,
      source_member_name=None):
    if member_name:
      key = '%s.%s' % (interface.id, member_name)
      dom_name = '%s.%s' % (interface.javascript_binding_name, member_name)
    else:
      key = interface.id
      dom_name = interface.javascript_binding_name

    annotations = ["@DomName('" + dom_name + "')"]

    # Only add this for members, so we don't add DocsEditable to templated
    # classes (they get it from the default class template)
    if member_name:
      annotations.append('@DocsEditable()');

    if key in _annotations:
      annotations.extend(_annotations[key])

    if source_member_name:
      member_name = source_member_name

    support_annotations = self._GetSupportLevelAnnotations(
        interface.id, member_name)

    for annotation in support_annotations:
      if annotation not in annotations:
        annotations.append(annotation)

    return annotations

  def _GetComments(self, library_name, interface, member_name=None):
    """ Gets all comments for the interface or member and returns a list. """

    # Add documentation from JSON.
    comments = []
    library_name = 'dart.dom.%s' % library_name
    if library_name in self._doc_comments:
      library_info = self._doc_comments[library_name]
      if interface.id in library_info:
        interface_info = library_info[interface.id]
        if member_name:
          if 'members' in interface_info and member_name in interface_info['members']:
            comments = interface_info['members'][member_name]
        elif 'comment' in interface_info:
          comments = interface_info['comment']

    if comments:
      comments = ['\n'.join(comments)]

    return comments


  def AnyConversionAnnotations(self, idl_type, interface_name, member_name):
    if (_annotations.get('%s.%s' % (interface_name, member_name))):
      return True
    else:
      return False

  def FormatMetadata(self, metadata, indentation):
    if metadata:
      newline = '\n%s' % indentation
      result = newline.join(metadata) + newline
      return result
    return ''

  def _GetSupportInfo(self, interface_id, member_id=None):
    """ Looks up the interface or member in the DOM status list and returns the
    support level for it.
    """
    if interface_id in self._monitored_types:
      type_info = self._monitored_types[interface_id]
    else:
      type_info = {
        'members': {},
        'support_level': 'untriaged',
      }
      self._types[interface_id] = type_info

    if not member_id:
      return type_info

    members = type_info['members']

    if member_id in members:
      member_info = members[member_id]
    else:
      if member_id == interface_id:
        member_info = {}
      else:
        member_info = {'support_level': 'untriaged'}
      members[member_id] = member_info

    return member_info

  def _GetSupportLevelAnnotations(self, interface_id, member_id=None):
    """ Gets annotations for API support status.
    """
    support_info = self._GetSupportInfo(interface_id, member_id)

    dart_action = support_info.get('dart_action')
    support_level = support_info.get('support_level')
    comment = support_info.get('comment')
    annotations = []
    # TODO(blois): should add an annotation for the comment, but keeping out
    # to keep the initial diff a bit more localized.
    #if comment:
    #  annotations.append('// %s' % comment)

    if dart_action:
      if dart_action == 'unstable':
        annotations.append('@Unstable()')
      elif dart_action == 'experimental':
        if comment:
          annotations.append('// %s' % comment)
        annotations.append('@Experimental() // %s' % support_level)
      elif dart_action == 'suppress':
        if comment:
          annotations.append('// %s' % comment)
        anAnnotation = 'deprecated'
        if member_id:
          anAnnotation = 'removed'
        annotations.append('@%s // %s' % (anAnnotation, support_level))
        pass
      elif dart_action == 'stable':
        pass
      else:
        _logger.warn('Unknown dart_action - %s:%s' % (interface_id, member_id))
    elif support_level == 'untriaged':
      annotations.append('@Experimental() // untriaged')
    elif support_level == 'experimental':
      if comment:
        annotations.append('// %s' % comment)
      annotations.append('@Experimental()')
    elif support_level == 'nonstandard':
      if comment:
        annotations.append('// %s' % comment)
      annotations.append('@Experimental() // non-standard')
    elif support_level == 'stable':
      pass
    elif support_level == 'deprecated':
      if comment:
        annotations.append('// %s' % comment)
      annotations.append('@deprecated')
    elif support_level is None:
      pass
    else:
      _logger.warn('Unknown support_level - %s:%s' % (interface_id, member_id))

    return annotations

  def Flush(self):
    json_file = open(self._api_status_path, 'w+')
    json.dump(self._types, json_file, indent=2, separators=(',', ': '), sort_keys=True)
    json_file.close()
