// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.sky;

abstract class Event extends NativeFieldWrapperClass2 {
}

typedef void EventListener(Event b);

abstract class EventTarget extends NativeFieldWrapperClass2 {
  void addEventListener(String type, EventListener listener, [bool useCapture = false]) native "EventTarget_addEventListener_Callback";
  void removeEventListener(String type, EventListener listener, [bool useCapture = false]) native "EventTarget_removeEventListener_Callback";
  bool dispatchEvent(Event event) native "EventTarget_dispatchEvent_Callback";
}

abstract class Node extends EventTarget {
  Node get parentNode native "Node_parentNode_Getter";
  Node get firstChild native "Node_firstChild_Getter";
  Node get lastChild native "Node_lastChild_Getter";
  Node get previousSibling native "Node_previousSibling_Getter";
  Node get nextSibling native "Node_nextSibling_Getter";
  Document get ownerDocument native "Node_ownerDocument_Getter";
  Node get ownerScope native "Node_ownerScope_Getter";

  Node insertBefore(Node newChild, Node refChild) native "Node_insertBefore_Callback";
  Node replaceChild(Node newChild, Node oldChild) native "Node_replaceChild_Callback";
  Node removeChild(Node oldChild) native "Node_removeChild_Callback";
  Node appendChild(Node newChild) native "Node_appendChild_Callback";

  bool hasChildNodes() native "Node_hasChildNodes_Callback";
  Node cloneNode([boolean deep = true]) native "Node_cloneNode_Callback";

  String get textContent native "Node_textContent_Getter";
  bool contains(Node other) native "Node_contains_Callback";

  Element get parentElement native "Node_parentElement_Getter";
}

class Attr extends NativeFieldWrapperClass2 {
  String get name native "Attr_name_Getter";
  String get value native "Attr_value_Getter";
}

abstract class Element extends Node {
  String get tagName native "Element_tagName_Getter";

  bool hasAttribute(@nonnull String name) native "Element_hasAttribute_Callback";
  String getAttribute(@nonnull String name) native "Element_getAttribute_Callback";
  void setAttribute(@nonnull String name, [@nonnull String value = '']) native "Element_setAttribute_Callback";
  void removeAttribute(@nonnull String name) native "Element_removeAttribute_Callback";

  bool hasAttributes() native "Element_hasAttributes_Callback";
  List<Attr> getAttributes() native "Element_getAttributes_Callback";

  // style

  String get id native "Element_id_Getter";
  void set id(String value) native "Element_id_Setter";

  String get localName native "Element_localName_Getter";

  bool matches(String selectors) native "Element_matches_Callback";

  int get offsetLeft native "Element_offsetLeft_Getter";
  int get offsetTop native "Element_offsetTop_Getter";
  int get offsetWidth native "Element_offsetWidth_Getter";
  int get offsetHeight native "Element_offsetHeight_Getter";
  Element get offsetParent native "Element_offsetParent_Getter";
  int get clientLeft native "Element_clientLeft_Getter";
  int get clientTop native "Element_clientTop_Getter";
  int get clientWidth native "Element_clientWidth_Getter";
  int get clientHeight native "Element_clientHeight_Getter";

  void focus() native "Element_focus_Callback";
  void blur() native "Element_blur_Callback";

  // classList

  ShadowRoot ensureShadowRoot() native "Element_ensureShadowRoot_Callback";
  ShadowRoot get shadowRoot native "Element_shadowRoot_Getter";

  List<Node> getDestinationInsertionPoints() native "Element_getDestinationInsertionPoints_Callback";

  // ClientRectList getClientRects();
  // ClientRect getBoundingClientRect();

  String get lang native "Element_lang_Getter";
  String get dir native "Element_dir_Getter";

  int get tabIndex native "Element_tabIndex_Getter";
  void set tabIndex(int value) native "Element_tabIndex_Setter";

  String get contentEditable native "Element_contentEditable_Getter";
  void set contentEditable(String value) native "Element_contentEditable_Setter";

  bool get isContentEditable native "Element_isContentEditable_Getter";

  bool get spellcheck native "Element_spellcheck_Getter";
  void set spellcheck(bool value) native "Element_spellcheck_Setter";

  Element get firstElementChild native "Element_firstElementChild_Getter";
  Element get lastElementChild native "Element_lastElementChild_Getter";
  int get childElementCount native "Element_childElementCount_Getter";

  Element querySelector(String selectors) native "Element_querySelector_Callback";
  List<Node> querySelectorAll(String selectors) native "Element_querySelectorAll_Callback";

  Element get previousElementSibling native "Element_previousElementSibling_Getter";
  Element get nextElementSibling native "Element_nextElementSibling_Getter";
  void remove() native "Element_remove_Callback";
}

class CharacterData extends Node {
  String get data native "CharacterData_data_Getter";
  void set data(String value) native "CharacterData_data_Setter";

  int get length native "CharacterData_length_Getter";

  String substringData(int offset, int length) native "CharacterData_substringData_Callback";
  void appendData(String data) native "CharacterData_appendData_Callback";
  void insertData(int offset, String data) native "CharacterData_insertData_Callback";
  void deleteData(int offset, int length) native "CharacterData_deleteData_Callback";
  void replaceData(int offset, int length, String data) native "CharacterData_replaceData_Callback";

  Element get previousElementSibling native "CharacterData_previousElementSibling_Getter";
  Element get nextElementSibling native "CharacterData_nextElementSibling_Getter";
  void remove() native "CharacterData_remove_Callback";
}

class Text extends CharacterData {
  static Text _constructor(String data) native "Text_constructorCallback";
  factory Text([String data = '']) => _constructor(data);

  Text splitText(int offset) native "Text_splitText_Callback";
  List<Node> getDestinationInsertionPoints() native "Text_getDestinationInsertionPoints_Callback";
}

class DocumentFragment extends Node {
  static DocumentFragment _constructor() native "DocumentFragment_constructorCallback";
  factory DocumentFragment() => _constructor();

  Element getElementById(String elementId) native "DocumentFragment_getElementById_Callback";

  Element get firstElementChild native "DocumentFragment_firstElementChild_Getter";
  Element get lastElementChild native "DocumentFragment_lastElementChild_Getter";
  int get childElementCount native "DocumentFragment_childElementCount_Getter";

  Element querySelector(String selectors) native "DocumentFragment_querySelector_Callback";
  List<Node> querySelectorAll(String selectors) native "DocumentFragment_querySelectorAll_Callback";
}

class ShadowRoot extends DocumentFragment {
  Element get activeElement native "ShadowRoot_activeElement_Getter";

  Selection getSelection() native "ShadowRoot_getSelection_Callback";

  Element getElementById(String elementId) native "ShadowRoot_getElementById_Callback";
  Element elementFromPoint(int x, int y) native "ShadowRoot_elementFromPoint_Callback";

  Element get host native "ShadowRoot_host_Getter";
}

class Document extends Node {
  static Document _constructor() native "Document_constructorCallback";
  factory Document() => _constructor();

  Element get documentElement native "Document_documentElement_Getter";

  Element createElement(String tagName) native "Document_createElement_Callback";
  DocumentFragment createDocumentFragment() native "Document_createDocumentFragment_Callback";

  Node importNode(Node node, [bool deep = false]) native "Document_importNode_Callback";
  Element getElementById(DOMString elementId) native "Document_getElementById_Callback";

  Node adoptNode(Node node) native "Document_adoptNode_Callback";

  String get baseURI native "Document_baseURI_Getter";
  String get contentType native "Document_contentType_Getter";

  String get dir native "Document_dir_Getter";
  void set dir(String value) native "Document_dir_Setter";

  String get title native "Document_title_Getter";
  void set title(String value) native "Document_title_Setter";

  String get referrer native "Document_referrer_Getter";
  String get URL native "Document_URL_Getter";
  String get readyState native "Document_readyState_Getter";

  Element elementFromPoint(int x, int y) native "Document_elementFromPoint_Callback";

  Element get activeElement native "Document_activeElement_Getter";
  bool hasFocus() native "Document_hasFocus_Callback";

  String get visibilityState native "Document_visibilityState_Getter";
  bool get hidden native "Document_hidden_Getter";

  Element get currentScript native "Document_currentScript_Getter";

  Element get firstElementChild native "Document_firstElementChild_Getter";
  Element get lastElementChild native "Document_lastElementChild_Getter";
  int get childElementCount native "Document_childElementCount_Getter";

  Element querySelector(String selectors) native "Document_querySelector_Callback";
  List<Node> querySelectorAll(String selectors) native "Document_querySelectorAll_Callback";
}

abstract class Window extends EventTarget {
  Document get document native "Window_document_Getter";
}

abstract class HTMLAnchorElement extends Element {
}

abstract class HTMLCanvasElement extends Element {
}

abstract class HTMLContentElement extends Element {
}

abstract class HTMLIFrameElement extends Element {
}

abstract class HTMLImageElement extends Element {
}

abstract class HTMLImportElement extends Element {
}

abstract class HTMLScriptElement extends Element {
}

abstract class HTMLStyleElement extends Element {
}

abstract class HTMLTElement extends Element {
}

abstract class HTMLTemplateElement extends Element {
}

abstract class HTMLTitleElement extends Element {
}
