// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/builtin_sky.h"

namespace blink {

Dart_NativeFunction BuiltinSky::NativeLookup(
    Dart_Handle name, int argument_count, bool* auto_setup_scope) {
 return nullptr;
}

const uint8_t* BuiltinSky::NativeSymbol(Dart_NativeFunction native_function) {
 return nullptr;
}

}  // namespace blink
