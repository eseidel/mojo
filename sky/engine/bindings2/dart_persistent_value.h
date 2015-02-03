// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_PERSISTENT_VALUE_H_
#define SKY_ENGINE_BINDINGS2_DART_PERSISTENT_VALUE_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/Noncopyable.h"

namespace blink {

class DartPersistentValue {
  WTF_MAKE_NONCOPYABLE(DartPersistentValue);
 public:
  DartPersistentValue();
  DartPersistentValue(Dart_Isolate isolate, Dart_Handle value);
  ~DartPersistentValue();

  Dart_PersistentHandle value() const { return value_; }
  bool is_empty() const { return !value_; }

  void Clear();

 private:
  // TODO(abarth): This class should really be a DartIsolateDestructionObserver.
  Dart_Isolate isolate_;
  Dart_PersistentHandle value_;
};

}

#endif // SKY_ENGINE_BINDINGS2_DART_PERSISTENT_VALUE_H_
