// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library dart.sky;

import "dart:nativewrappers";

abstract class Node extends NativeFieldWrapperClass2 {
  Node get firstChild native "Node_firstChild_Getter";
  Node get lastChild native "Node_lastChild_Getter";
  Node get nextSibling native "Node_nextSibling_Getter";
  Node get previousSibling native "Node_previousSibling_Getter";
}

abstract class CharacterData extends Node {
  String get data native "CharacterData_data_Getter";
}

class Text extends CharacterData {
  static Text _constructor(String data) native "Text_constructorCallback";
  factory Text(String data) => _constructor(data);
}
