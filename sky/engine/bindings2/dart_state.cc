// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_state.h"

namespace blink {

DartState::DartState(Dart_Isolate isolate, intptr_t library_id)
    : isolate_(isolate), library_id_(library_id) {
}

DartState::~DartState() {
}
}
