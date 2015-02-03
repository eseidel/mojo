// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_STATE_H_
#define SKY_ENGINE_BINDINGS2_DART_STATE_H_

#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/Noncopyable.h"
#include "sky/engine/wtf/OwnPtr.h"
#include "sky/engine/wtf/PassRefPtr.h"
#include "sky/engine/wtf/RefCounted.h"

namespace blink {
class DartStringCache;

class DartState {
  WTF_MAKE_NONCOPYABLE(DartState);
 public:
  class Scope {
   public:
    Scope(DartState* dart_state);
    ~Scope();
  };

  ~DartState();

  static DartState* Current();

  base::WeakPtr<DartState> GetWeakPtr();

  Dart_Isolate isolate() { return isolate_; }
  DartStringCache& string_cache() { return *string_cache_; }

 private:
  explicit DartState();

  void set_isolate(Dart_Isolate isolate) {
    CHECK(!isolate_);
    isolate_ = isolate;
  }

  Dart_Isolate isolate_;
  OwnPtr<DartStringCache> string_cache_;

  base::WeakPtrFactory<DartState> weak_factory_;

  friend class DartController;
};

} // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_DART_STATE_H_
