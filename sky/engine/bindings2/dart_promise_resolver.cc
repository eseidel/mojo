// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_promise_resolver.h"

namespace blink {

DartPromiseResolver::DartPromiseResolver(DartState* dart_state)
    : dart_state_(dart_state) {
}

DartPromiseResolver::~DartPromiseResolver() {
}

PassRefPtr<DartPromise> DartPromiseResolver::GetPromise() {
  return DartPromise::Create();
}

} // namespace blink
