// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_promise.h"

#include "base/logging.h"

namespace blink {

DartPromise::DartPromise() {
}

DartPromise::DartPromise(DartState* dart_state, Dart_Handle promise)
    : value_(dart_state->isolate(), promise) {
  if (!promise || Dart_IsFuture(promise))
    return;
  DCHECK(false) << "DartPromise needs a promise.";
  value_.Clear();
}

DartPromise::~DartPromise() {
}

void DartPromise::Clear() {
  value_.Clear();
}

} // namespace blink
