// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library dart.sky;

abstract class Node {
  Node get firstChild native "Node_firstChild_Getter";
  Node get lastChild native "Node_lastChild_Getter";
  Node get nextSibling native "Node_nextSibling_Getter";
  Node get previousSibling native "Node_previousSibling_Getter";
}

abstract class CharacterData extends Node {
}

class Text extends CharacterData {
  factory Text(String data) native "Text_constructorCallback";
}
