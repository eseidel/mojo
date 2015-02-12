// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/script/dart_controller.h"

#include "base/bind.h"
#include "base/logging.h"
#include "sky/engine/bindings2/builtin.h"
#include "sky/engine/bindings2/builtin_natives.h"
#include "sky/engine/bindings2/builtin_sky.h"
#include "sky/engine/core/app/AbstractModule.h"
#include "sky/engine/core/app/Module.h"
#include "sky/engine/core/dom/Element.h"
#include "sky/engine/core/frame/LocalFrame.h"
#include "sky/engine/core/html/imports/HTMLImport.h"
#include "sky/engine/core/html/imports/HTMLImportChild.h"
#include "sky/engine/core/loader/FrameLoaderClient.h"
#include "sky/engine/core/script/dart_dependency_catcher.h"
#include "sky/engine/core/script/dart_loader.h"
#include "sky/engine/core/script/dom_dart_state.h"
#include "sky/engine/tonic/dart_api_scope.h"
#include "sky/engine/tonic/dart_class_library.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_gc_controller.h"
#include "sky/engine/tonic/dart_isolate_scope.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/wtf/text/TextPosition.h"

namespace blink {

static const char* kCheckedModeArgs[] = {
  "--enable_asserts",
  "--enable_type_checks",
  "--error_on_bad_type",
  "--error_on_bad_override",
};

extern const uint8_t* kDartSnapshotBuffer;

DartController::DartController() : weak_factory_(this) {
}

DartController::~DartController() {
}

void DartController::LoadModule(RefPtr<AbstractModule> module,
                                const String& source,
                                const TextPosition& textPosition) {
  DartIsolateScope isolate_scope(dart_state()->isolate());
  DartApiScope dart_api_scope;

  DartDependencyCatcher dependency_catcher(dart_state()->loader());

  Dart_Handle library = Dart_LoadLibrary(
      StringToDart(dart_state(), module->url()),
      StringToDart(dart_state(), source), textPosition.m_line.zeroBasedInt(),
      textPosition.m_column.zeroBasedInt());

  if (LogIfError(library))
    return;

  if (HTMLImport* parent = module->document()->import()) {
    for (HTMLImportChild* child = static_cast<HTMLImportChild*>(parent->firstChild());
         child; child = static_cast<HTMLImportChild*>(child->next())) {
      if (Element* link = child->link()) {
        String name = link->getAttribute(HTMLNames::asAttr);

        Module* childModule = child->module();
        if (childModule
            && childModule->library()
            && !childModule->library()->is_empty()) {
          if (LogIfError(Dart_LibraryImportLibrary(
                  library, childModule->library()->dart_value(),
                  StringToDart(dart_state(), name))))
            return;
        }
      }
    }
  }

  module->set_library(DartValue::Create(dart_state(), library));
  const auto& dependencies = dependency_catcher.dependencies();

  if (dependencies.isEmpty()) {
    ExecuteModule(module);
  } else {
    dart_state()->loader().WaitForDependencies(
        dependencies, base::Bind(&DartController::ExecuteModule,
                                 weak_factory_.GetWeakPtr(), module));
  }
}

void DartController::ExecuteModule(RefPtr<AbstractModule> module) {
  DCHECK(Dart_CurrentIsolate() == dart_state()->isolate());
  DartApiScope dart_api_scope;

  LogIfError(Dart_FinalizeLoading(true));
  Dart_Handle library = module->library()->dart_value();
  const char* name = module->isApplication() ? "main" : "init";
  Dart_Handle closure_name = Dart_NewStringFromCString(name);
  Dart_Handle result = Dart_Invoke(library, closure_name, 0, nullptr);

  if (module->isApplication()) {
    // TODO(dart): This will throw an API error if main() is absent. It would be
    // better to test whether main() is present first, then attempt to invoke it
    // so as to capture & report other errors.
    LogIfError(result);
  }
}

static void UnhandledExceptionCallback(Dart_Handle error) {
  DCHECK(!Dart_IsError(error));
  LOG(ERROR) << Dart_GetError(error);
}

static Dart_Handle LibraryTagHandler(Dart_LibraryTag tag,
                                     Dart_Handle library,
                                     Dart_Handle url) {
  DOMDartState* dart_state = DOMDartState::Current();
  return dart_state->loader().HandleLibraryTag(tag, library, url);
}

static void IsolateShutdownCallback(void* callback_data) {
  DartState* dart_state = static_cast<DartState*>(callback_data);
  DCHECK(dart_state);
  // TODO(dart)
}

void DartController::CreateIsolateFor(Document* document) {
  DCHECK(document);
  CHECK(kDartSnapshotBuffer);
  char* error = nullptr;
  dom_dart_state_ = adoptPtr(new DOMDartState(document));
  Dart_Isolate isolate = Dart_CreateIsolate(
      document->url().string().utf8().data(), "main", kDartSnapshotBuffer,
      static_cast<DartState*>(dom_dart_state_.get()), &error);
  CHECK(isolate) << error;
  dom_dart_state_->set_isolate(isolate);
  Dart_SetGcCallbacks(DartGCPrologue, DartGCEpilogue);
  CHECK(!LogIfError(Dart_SetLibraryTagHandler(LibraryTagHandler)));

  {
    DartApiScope apiScope;

    Builtin::SetNativeResolver(Builtin::kBuiltinLibrary);
    BuiltinNatives::Init();

    builtin_sky_ = adoptPtr(new BuiltinSky(dart_state()));
    dart_state()->class_library().set_provider(builtin_sky_.get());
    builtin_sky_->InstallWindow(dart_state());

    document->frame()->loaderClient()->didCreateIsolate(isolate);
  }
  Dart_ExitIsolate();
}

void DartController::ClearForClose() {
  DartIsolateScope scope(dom_dart_state_->isolate());
  Dart_ShutdownIsolate();
  dom_dart_state_.clear();
}

void DartController::InitVM() {
  int argc = 0;
  const char** argv = nullptr;

#if ENABLE(ASSERT)
  argc = arraysize(kCheckedModeArgs);
  argv = kCheckedModeArgs;
#endif

  CHECK(Dart_SetVMFlags(argc, argv));
  CHECK(Dart_Initialize(nullptr,
                        nullptr,  // Isolate interrupt callback.
                        UnhandledExceptionCallback, IsolateShutdownCallback,
                        // File IO callbacks.
                        nullptr, nullptr, nullptr, nullptr, nullptr));
}

} // namespace blink
