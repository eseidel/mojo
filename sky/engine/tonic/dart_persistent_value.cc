// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/tonic/dart_persistent_value.h"

#include "sky/engine/tonic/dart_isolate_scope.h"
#include "sky/engine/tonic/dart_state.h"

namespace blink {

DartPersistentValue::DartPersistentValue() : value_(nullptr) {
}

DartPersistentValue::DartPersistentValue(DartState* dart_state,
                                         Dart_Handle value) {
  dart_state_ = dart_state->GetWeakPtr();
  value_ = Dart_NewPersistentHandle(value);
}

DartPersistentValue::~DartPersistentValue() {
  Clear();
}

Dart_Handle DartPersistentValue::Release() {
  if (!value_)
    return nullptr;
  Dart_Handle local = Dart_HandleFromPersistent(value_);
  Clear();
  return local;
}

void DartPersistentValue::Clear() {
  if (!value_ || !dart_state_.get())
    return;

  DartIsolateScope scope(dart_state_->isolate());
  Dart_DeletePersistentHandle(value_);
  dart_state_ = base::WeakPtr<DartState>();
  value_ = nullptr;
}
}
