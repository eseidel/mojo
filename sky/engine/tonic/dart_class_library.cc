// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/tonic/dart_class_library.h"

namespace blink {

DartClassLibrary::DartClassLibrary() {
}

DartClassLibrary::~DartClassLibrary() {
}

Dart_PersistentHandle DartClassLibrary::GetClassById(intptr_t class_id) {
  return nullptr;
}

}  // namespace blink
