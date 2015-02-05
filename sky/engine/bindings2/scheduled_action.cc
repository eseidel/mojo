// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/scheduled_action.h"

namespace blink {

ScheduledAction::ScheduledAction(DartState* dart_state, Dart_Handle action)
    : action_(dart_state, action) {
}

ScheduledAction::~ScheduledAction() {
}

void ScheduledAction::Execute(ExecutionContext*) {
}

}  // namespace blink
