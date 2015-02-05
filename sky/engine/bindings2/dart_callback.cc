// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_callback.h"

namespace blink {

DartCallback::DartCallback(DartState* dart_state, Dart_Handle callback, Dart_Handle& exception)
    : callback_(dart_state, callback) {
  if (!Dart_IsClosure(callback)) {
    exception = Dart_NewStringFromCString("Callback must be a function");
    callback_.Clear();
  }
}

DartCallback::~DartCallback() {
}

bool DartCallback::handleEvent(int argc, Dart_Handle* argv) {
  return true;
}

} // namespace blink
