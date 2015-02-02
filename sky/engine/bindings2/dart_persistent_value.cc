// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_persistent_value.h"

#include "sky/engine/bindings2/dart_isolate_scope.h"

namespace blink {

DartPersistentValue::DartPersistentValue()
    : isolate_(nullptr), value_(nullptr) {
}

DartPersistentValue::DartPersistentValue(Dart_Isolate isolate,
                                         Dart_Handle value) {
  isolate_ = isolate;
  value_ = Dart_NewPersistentHandle(value);
}

DartPersistentValue::~DartPersistentValue() {
  Clear();
}

void DartPersistentValue::Clear() {
  if (!value_)
    return;

  DartIsolateScope scope(isolate_);
  Dart_DeletePersistentHandle(value_);
  value_ = nullptr;
}

}
