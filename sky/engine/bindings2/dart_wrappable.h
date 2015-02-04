// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"

namespace blink {
class DartState;
struct DartWrapperInfo;

class DartWrappableBase {
};

// DartWrappable is a base class that you can inherit from in order to be
// exposed to Dart code as an interface.
class DartWrappable : public DartWrappableBase {
 public:
  DartWrappable() : dart_wrapper_(nullptr) {}

  virtual const DartWrapperInfo& GetDartWrapperInfo() const;

  Dart_Handle Wrap(DartState* dart_state);
  Dart_WeakPersistentHandle dart_wrapper() const { return dart_wrapper_; }

  void set_dart_wrapper(Dart_WeakPersistentHandle dart_wrapper) {
    DCHECK(!dart_wrapper_ || !dart_wrapper);
    dart_wrapper_ = dart_wrapper;
  }

 protected:
  virtual ~DartWrappable();

 private:
  Dart_WeakPersistentHandle dart_wrapper_;
};

#define DEFINE_WRAPPERTYPEINFO()

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
