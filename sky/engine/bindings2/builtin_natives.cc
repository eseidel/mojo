// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/builtin_natives.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/bind.h"
#include "base/logging.h"
#include "base/macros.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/builtin.h"
#include "sky/engine/core/dom/Microtask.h"
#include "sky/engine/tonic/dart_api_scope.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_isolate_scope.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/tonic/dart_value.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

#define REGISTER_FUNCTION(name, count)                                         \
  { "" #name, name, count },
#define DECLARE_FUNCTION(name, count)                                          \
  extern void name(Dart_NativeArguments args);

// Lists the native functions implementing basic functionality in
// the Mojo embedder dart, such as printing, and file I/O.
#define BUILTIN_NATIVE_LIST(V) \
  V(Logger_PrintString, 1)     \
  V(ScheduleMicrotask, 1)

BUILTIN_NATIVE_LIST(DECLARE_FUNCTION);

static struct NativeEntries {
  const char* name;
  Dart_NativeFunction function;
  int argument_count;
} BuiltinEntries[] = {BUILTIN_NATIVE_LIST(REGISTER_FUNCTION)};

Dart_NativeFunction BuiltinNatives::NativeLookup(Dart_Handle name,
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

const uint8_t* BuiltinNatives::NativeSymbol(Dart_NativeFunction native_function) {
  size_t num_entries = arraysize(BuiltinEntries);
  for (size_t i = 0; i < num_entries; i++) {
    const struct NativeEntries& entry = BuiltinEntries[i];
    if (entry.function == native_function) {
      return reinterpret_cast<const uint8_t*>(entry.name);
    }
  }
  return nullptr;
}

static void InitPrintClosure(Dart_Handle builtin_library) {
  // Setup the internal library's 'internalPrint' function.
  Dart_Handle print =
      Dart_Invoke(builtin_library,
                  Dart_NewStringFromCString("_getPrintClosure"), 0, nullptr);
  DART_CHECK_VALID(print);
  Dart_Handle url = Dart_NewStringFromCString("dart:_internal");
  DART_CHECK_VALID(url);
  Dart_Handle internal_lib = Dart_LookupLibrary(url);
  DART_CHECK_VALID(internal_lib);
  Dart_Handle result_handle = Dart_SetField(internal_lib,
                         Dart_NewStringFromCString("_printClosure"),
                         print);
  DART_CHECK_VALID(result_handle);
}

static void InitAyncLibrary(Dart_Handle builtin_library) {
  Dart_Handle schedule_microtask = Dart_Invoke(
      builtin_library,
      Dart_NewStringFromCString("_getScheduleMicrotaskClosure"), 0, nullptr);
  DART_CHECK_VALID(schedule_microtask);
  Dart_Handle async_library =
      Dart_LookupLibrary(Dart_NewStringFromCString("dart:async"));
  DART_CHECK_VALID(async_library);
  DART_CHECK_VALID(Dart_Invoke(
      async_library, Dart_NewStringFromCString("_setScheduleImmediateClosure"),
      1, &schedule_microtask));
}

void BuiltinNatives::Init() {
  Dart_Handle builtin_library =
      Builtin::LoadAndCheckLibrary(Builtin::kBuiltinLibrary);
  DART_CHECK_VALID(builtin_library);
  InitPrintClosure(builtin_library);
  InitAyncLibrary(builtin_library);
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

static void ExecuteMicrotask(base::WeakPtr<DartState> dart_state,
                             RefPtr<DartValue> callback) {
  if (!dart_state)
    return;
  DartIsolateScope scope(dart_state->isolate());
  DartApiScope api_scope;
  Dart_InvokeClosure(callback->dart_value(), 0, nullptr);
}

void ScheduleMicrotask(Dart_NativeArguments args) {
  DartState* dart_state = DartState::Current();
  Dart_Handle closure = Dart_GetNativeArgument(args, 0);
  if (LogIfError(closure) || !Dart_IsClosure(closure))
    return;
  Microtask::enqueueMicrotask(base::Bind(&ExecuteMicrotask,
    dart_state->GetWeakPtr(), DartValue::Create(dart_state, closure)));
}

}  // namespace blink
