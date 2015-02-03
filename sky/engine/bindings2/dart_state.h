// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_STATE_H_
#define SKY_ENGINE_BINDINGS2_DART_STATE_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"

namespace blink {

class DartState : public RefCounted<DartState> {
  WTF_MAKE_NONCOPYABLE(DartState);
 public:
  ~DartState();

  static DartState* Current();

  Dart_Isolate isolate() { return isolate_; }

 private:
  explicit DartState();

  void set_isolate(Dart_Isolate isolate) {
    CHECK(!isolate_);
    isolate_ = isolate;
  }

  Dart_Isolate isolate_;

  friend class DartController;
};

} // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_DART_STATE_H_
