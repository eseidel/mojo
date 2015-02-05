// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_API_SCOPE_H_
#define SKY_ENGINE_BINDINGS2_DART_API_SCOPE_H_

#include "dart/runtime/include/dart_api.h"

namespace blink {

class DartApiScope {
 public:
  DartApiScope() { Dart_EnterScope(); }
  ~DartApiScope() { Dart_ExitScope(); }
};
}

#endif  // SKY_ENGINE_BINDINGS2_DART_API_SCOPE_H_
