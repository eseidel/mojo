// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_state.h"

namespace blink {

DartState::DartState()
    : isolate_(NULL) {
}

DartState::~DartState() {
}

DartState* DartState::Current() {
  return static_cast<DartState*>(Dart_CurrentIsolateData());
}

}  // namespace blink
