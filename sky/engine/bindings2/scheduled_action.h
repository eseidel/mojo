// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_SCHEDULED_ACTION_H_
#define SKY_ENGINE_BINDINGS2_SCHEDULED_ACTION_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_persistent_value.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/wtf/RefPtr.h"

namespace blink {

class ExecutionContext;

class ScheduledAction {
 public:
  ScheduledAction(DartState* dart_state, Dart_Handle action);
  ~ScheduledAction();

  void Execute(ExecutionContext*);

 private:
  DartPersistentValue action_;
};

}  // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_SCHEDULED_ACTION_H_
