// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_wrappable.h"

namespace blink {

DartWrappable::~DartWrappable() {
  CHECK(!ContainsWrapper());
}

Dart_WeakPersistentHandle DartWrappable::Wrap(Dart_Isolate isolate) {
  return nullptr;
}

} // namespace blink
