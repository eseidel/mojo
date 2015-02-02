// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_STATE_H_
#define SKY_ENGINE_BINDINGS2_DART_STATE_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"

namespace blink {

class DartState : public RefCounted<DartState> {
  WTF_MAKE_NONCOPYABLE(DartState);
 public:
  static PassRefPtr<DartState> Create(Dart_Isolate isolate,
                                      intptr_t library_id) {
    return adoptRef(new DartState(isolate, library_id));
  }

  ~DartState();

  Dart_Isolate isolate() { return isolate_; }
  intptr_t library_id() { return library_id_; }

 private:
  explicit DartState(Dart_Isolate isolate, intptr_t library_id);

  Dart_Isolate isolate_;
  intptr_t library_id_;
};
}

#endif  // SKY_ENGINE_BINDINGS2_DART_STATE_H_
