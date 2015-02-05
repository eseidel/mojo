// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPER_INFO_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPER_INFO_H_

#include "sky/engine/wtf/Noncopyable.h"

namespace blink {
class DartWrappable;

struct DartWrapperInfo {
  const intptr_t class_id;
  const size_t size_in_bytes;
  const void (*ref_object)(DartWrappable*);
  const void (*deref_object)(DartWrappable*);

 private:
  WTF_MAKE_NONCOPYABLE(DartWrapperInfo);
};

}  // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_DART_WRAPPER_INFO_H_
