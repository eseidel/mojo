// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"

namespace blink {
class DartWrapperInfo;

class DartWrappable {
 public:
  DartWrappable() : wrapper_(nullptr) {}

  virtual const DartWrapperInfo* GetDartWrapperInfo() const = 0;
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

#define DEFINE_WRAPPERTYPEINFO() \
public: \
  const DartWrapperInfo* GetDartWrapperInfo() const override \
  { \
    return &wrapper_type_info_; \
  } \
private: \
  static const DartWrapperInfo& wrapper_type_info_;

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
