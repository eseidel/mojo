// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/builtin_sky.h"

#include "gen/sky/bindings/DartNode.h"
#include "gen/sky/bindings/DartText.h"

namespace blink {

#define TRY_RESOLVER(DartClass)                                                \
  native_function = DartClass::Resolver(name, argc, auto_setup_scope);         \
  if (native_function)                                                         \
    return native_function;

#define TRY_SYMBOLIZER(DartClass)                                              \
  symbol = DartClass::Symbolizer(native_function);                             \
  if (symbol)                                                                  \
    return symbol;

Dart_NativeFunction BuiltinSky::NativeLookup(
    Dart_Handle name, int argc, bool* auto_setup_scope) {
  Dart_NativeFunction native_function = nullptr;
  TRY_RESOLVER(DartNode);
  TRY_RESOLVER(DartText);
  return nullptr;
}

const uint8_t* BuiltinSky::NativeSymbol(Dart_NativeFunction native_function) {
 const uint8_t* symbol = nullptr;
 TRY_SYMBOLIZER(DartNode);
 TRY_SYMBOLIZER(DartText);
 return nullptr;
}

}  // namespace blink
