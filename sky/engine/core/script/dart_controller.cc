// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/script/dart_controller.h"

#include "base/logging.h"
#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/core/app/AbstractModule.h"
#include "sky/engine/core/app/Module.h"
#include "sky/engine/core/html/imports/HTMLImport.h"
#include "sky/engine/wtf/text/TextPosition.h"
#include "sky/engine/core/html/imports/HTMLImportChild.h"
#include "sky/engine/core/dom/Element.h"
#include "sky/engine/core/script/core_dart_state.h"
#include "sky/engine/bindings2/dart_api_scope.h"
#include "sky/engine/bindings2/dart_isolate_scope.h"
#include "sky/engine/bindings2/dart_state.h"

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

bool LogIfError(Dart_Handle handle) {
  if (Dart_IsError(handle)) {
    LOG(ERROR) << Dart_GetError(handle);
    return true;
  }
  return false;
}

void DartController::ExecuteModuleScript(AbstractModule& module,
                                         const String& source,
                                         const TextPosition& textPosition) {
  DartIsolateScope isolate_scope(core_dart_state_->isolate());
  DartApiScope dart_api_scope;

  Dart_Handle library = Dart_LoadLibrary(
      Dart_NewStringFromCString(module.url().utf8().data()),
      Dart_NewStringFromCString(source.utf8().data()),
      textPosition.m_line.zeroBasedInt(), textPosition.m_column.zeroBasedInt());
  if (LogIfError(library))
    return;

  Dart_FinalizeLoading(true);

  if (HTMLImport* parent = module.document()->import()) {
    for (HTMLImportChild* child = static_cast<HTMLImportChild*>(parent->firstChild());
         child; child = static_cast<HTMLImportChild*>(child->next())) {
      if (Element* link = child->link()) {
          String name = link->getAttribute(HTMLNames::asAttr);

        Module* childModule = child->module();
        if (childModule && !childModule->exports()->is_empty()) {
          Dart_Handle importResult = Dart_LibraryImportLibrary(library, childModule->exports()->dart_value(), Dart_NewStringFromCString(name.utf8().data()));
          if (LogIfError(importResult))
            return;
        }
      }
    }
  }

  if (!module.isApplication()) {
    static_cast<Module*>(&module)
        ->setExports(DartValue::Create(core_dart_state_.get(), library));
    return;
  }

  // TODO(dart): This will throw an API error if main() is absent. It would be
  // better to test whether main() is present first, then attempt to invoke it
  // so as to capture & report other errors.
  Dart_Handle invoke_result =
      Dart_Invoke(library, Dart_NewStringFromCString("main"), 0, nullptr);
  if (LogIfError(invoke_result))
    return;

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

  Dart_Handle blink = Dart_LookupLibrary(Dart_NewStringFromCString("dart:_blink"));
  LogIfError(blink);
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

void DartController::SetDocument(Document* document) {
  DCHECK(document);
  char* error = nullptr;
  core_dart_state_ = adoptPtr(new CoreDartState(document));
  Dart_Isolate isolate = Dart_CreateIsolate(
      document->url().string().utf8().data(), "main",
      mojo::dart::snapshot_buffer,
      static_cast<DartState*>(core_dart_state_.get()), &error);
  CHECK(isolate) << error;
  core_dart_state_->set_isolate(isolate);
  Dart_SetGcCallbacks(GcPrologue, GcEpilogue);
}

void DartController::ClearForClose() {
  DartIsolateScope scope(core_dart_state_->isolate());
  Dart_ShutdownIsolate();
  core_dart_state_.clear();
}

void DartController::InitVM() {
  CHECK(Dart_SetVMFlags(0, NULL));
  CHECK(Dart_Initialize(IsolateCreateCallback,
                        nullptr,  // Isolate interrupt callback.
                        UnhandledExceptionCallback, IsolateShutdownCallback,
                        // File IO callbacks.
                        nullptr, nullptr, nullptr, nullptr, nullptr,
                        ServiceIsolateCreateCallback));
}

} // namespace blink
