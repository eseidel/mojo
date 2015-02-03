// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_PROMISE_RESOLVER_H_
#define SKY_ENGINE_BINDINGS2_DART_PROMISE_RESOLVER_H_

#include "sky/engine/bindings2/dart_promise.h"

namespace blink {

class DartPromiseResolver : public RefCounted<DartPromiseResolver> {
  WTF_MAKE_NONCOPYABLE(DartPromiseResolver);
 public:
  static PassRefPtr<DartPromiseResolver> Create(DartState* dart_state) {
    return adoptRef(new DartPromiseResolver(dart_state));
  }

  ~DartPromiseResolver();

  DartState* dart_state() { return dart_state_.get(); }

  PassRefPtr<DartPromise> GetPromise();

 private:
  explicit DartPromiseResolver(DartState*);

  RefPtr<DartState> dart_state_;
};

} // namespace blink

#endif // #ifndef SKY_ENGINE_BINDINGS2_DART_PROMISE_RESOLVER_H_
