// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_PROMISE_H_
#define SKY_ENGINE_BINDINGS2_DART_PROMISE_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/dart_persistent_value.h"
#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"
#include "sky/engine/wtf/RefPtr.h"

namespace blink {

class DartPromise : public RefCounted<DartPromise> {
  WTF_MAKE_NONCOPYABLE(DartPromise);
 public:
  static PassRefPtr<DartPromise> Create() {
    return adoptRef(new DartPromise());
  }

  static PassRefPtr<DartPromise> Create(DartState* dart_state, Dart_Handle promise) {
    return adoptRef(new DartPromise(dart_state, promise));
  }

  ~DartPromise();

  bool is_empty() const { return value_.is_empty(); }
  void Clear();

  Dart_Handle dart_value() const { return value_.value(); }

 private:
  DartPromise();
  DartPromise(DartState* dart_state, Dart_Handle promise);

  DartPersistentValue value_;
};

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_PROMISE_H_
