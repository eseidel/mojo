// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/logging.h"
#include "base/macros.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/builtin.h"
#include "sky/engine/config.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace mojo {
namespace dart {

// Lists the native functions implementing basic functionality in
// the Mojo embedder dart, such as printing, and file I/O.
#define BUILTIN_NATIVE_LIST(V)                                                 \
  V(Logger_PrintString, 1)

BUILTIN_NATIVE_LIST(DECLARE_FUNCTION);

static struct NativeEntries {
  const char* name;
  Dart_NativeFunction function;
  int argument_count;
} BuiltinEntries[] = {BUILTIN_NATIVE_LIST(REGISTER_FUNCTION)};

Dart_NativeFunction Builtin::NativeLookup(Dart_Handle name,
                                          int argument_count,
                                          bool* auto_setup_scope) {
  const char* function_name = nullptr;
  Dart_Handle result = Dart_StringToCString(name, &function_name);
  DART_CHECK_VALID(result);
  DCHECK(function_name != nullptr);
  DCHECK(auto_setup_scope != nullptr);
  *auto_setup_scope = true;
  size_t num_entries = arraysize(BuiltinEntries);
  for (size_t i = 0; i < num_entries; i++) {
    const struct NativeEntries& entry = BuiltinEntries[i];
    if (!strcmp(function_name, entry.name) &&
        (entry.argument_count == argument_count)) {
      return entry.function;
    }
  }
  return nullptr;
}

const uint8_t* Builtin::NativeSymbol(Dart_NativeFunction nf) {
  size_t num_entries = arraysize(BuiltinEntries);
  for (size_t i = 0; i < num_entries; i++) {
    const struct NativeEntries& entry = BuiltinEntries[i];
    if (entry.function == nf) {
      return reinterpret_cast<const uint8_t*>(entry.name);
    }
  }
  return nullptr;
}

// Implementation of native functions which are used for some
// test/debug functionality in standalone dart mode.
void Logger_PrintString(Dart_NativeArguments args) {
  intptr_t length = 0;
  uint8_t* chars = nullptr;
  Dart_Handle str = Dart_GetNativeArgument(args, 0);
  Dart_Handle result = Dart_StringToUTF8(str, &chars, &length);
  if (Dart_IsError(result)) {
    Dart_PropagateError(result);
  } else {

  String message(chars, length);
// TODO(dart): Hook up to developer console (if/when that's a thing).
#if OS(ANDROID)
    LOG(INFO) << "CONSOLE: " << message.utf8().data();
#else
    printf("CONSOLE: %s\n", message.utf8().data());
#endif

  }
  fflush(stdout);
}

}  // namespace bin
}  // namespace dart
