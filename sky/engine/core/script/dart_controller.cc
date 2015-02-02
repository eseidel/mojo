// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/script/dart_controller.h"

#include "base/logging.h"
#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/core/app/AbstractModule.h"
#include "sky/engine/wtf/text/TextPosition.h"

namespace mojo {
namespace dart {

extern const uint8_t* snapshot_buffer;
}
}

namespace blink {

DartController::DartController() {
}

DartController::~DartController() {
}

void DartController::clearForClose() {
}

void DartController::executeModuleScript(AbstractModule& module, const String& source, const TextPosition& textPosition) {

  Dart_EnterScope();

  Dart_Handle library = Dart_LoadLibrary(
      Dart_NewStringFromCString(module.url().utf8().data()),
      Dart_NewStringFromCString(source.utf8().data()),
      textPosition.m_line.zeroBasedInt(), textPosition.m_column.zeroBasedInt());

  Dart_FinalizeLoading(true);

  if (Dart_IsError(library)) {
    LOG(INFO) << Dart_GetError(library);
    abort();
  }

  Dart_Handle invoke_result =
      Dart_Invoke(library, Dart_NewStringFromCString("main"), 0, nullptr);
  if (Dart_IsError(invoke_result)) {
    LOG(INFO) << Dart_GetError(invoke_result);
    abort();
  }

  Dart_Handle str = Dart_ToString(invoke_result);
  const char* xyz = "invalid";
  Dart_StringToCString(str, &xyz);
  LOG(INFO) << xyz;
}


static Dart_Isolate IsolateCreateCallback(const char* script_uri,
                                          const char* main,
                                          const char* package_root,
                                          void* callback_data,
                                          char** error) {
  DartState* parent_dart_state = static_cast<DartState*>(callback_data);
  DCHECK(parent_dart_state);
  // TODO(dart)
  return nullptr;
}


static void UnhandledExceptionCallback(Dart_Handle error) {
  // TODO(dart)
}


static void IsolateShutdownCallback(void* callback_data) {
  DartState* dart_state = static_cast<DartState*>(callback_data);
  DCHECK(dart_state);
  // TODO(dart)
}


static Dart_Isolate ServiceIsolateCreateCallback(void* callback_data,
                                                 char** error) {
  // TODO(dart)
  return nullptr;
}


static void GcPrologue() {
  Dart_EnterScope();
  DartState* dart_state = DartState::Current();
  DCHECK(dart_state);
  // TODO(dart)
}


static void GcEpilogue() {
  Dart_ExitScope();
}


void DartController::InitVM() {
  bool result = Dart_SetVMFlags(0, NULL);

  result = Dart_Initialize(IsolateCreateCallback,
                           nullptr,  // Isolate interrupt callback.
                           UnhandledExceptionCallback, IsolateShutdownCallback,
                           // File IO callbacks.
                           nullptr, nullptr, nullptr, nullptr, nullptr,
                           ServiceIsolateCreateCallback);


  // TODO(dart): Need to setup GC callbacks

  char* error;

  // TODO(dart): What URI to use
  DartState* dart_state = new DartState();
  Dart_Isolate isolate = Dart_CreateIsolate(
      "what:uri", "main", mojo::dart::snapshot_buffer, nullptr, &error);
  if (!isolate) {
    delete dart_state;
    abort();
  }
  dart_state->set_isolate(isolate);
  Dart_SetGcCallbacks(GcPrologue, GcEpilogue);
}

} // namespace blink
