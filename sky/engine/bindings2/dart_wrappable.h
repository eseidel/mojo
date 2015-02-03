// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"

namespace blink {
class DartWrapperInfo;
class ScriptWrappableBase;

class DartWrappableBase {
};

// DartWrappable is a base class that you can inherit from in order to be
// exposed to Dart code as an interface.
class DartWrappable : public DartWrappableBase {
 public:
  DartWrappable() : wrapper_(nullptr) {}

  virtual const DartWrapperInfo* GetDartWrapperInfo() const;
  virtual Dart_WeakPersistentHandle Wrap(Dart_Isolate isolate);

  void SetWrapper(Dart_WeakPersistentHandle wrapper) {
    wrapper_ = wrapper;
  }

  bool ContainsWrapper() {
    return !!wrapper_;
  }

 protected:
  virtual ~DartWrappable();

  Dart_WeakPersistentHandle wrapper_;
};

#define DEFINE_WRAPPERTYPEINFO()

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
