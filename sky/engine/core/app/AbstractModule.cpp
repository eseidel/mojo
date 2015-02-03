// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/app/AbstractModule.h"

#include "sky/engine/core/app/Module.h"

namespace blink {

AbstractModule::AbstractModule(ExecutionContext* context,
                               PassRefPtr<Document> document,
                               const String& url)
  : ContextLifecycleObserver(context),
    document_(document),
    url_(url) {
  document_->setModule(this);
}

AbstractModule::~AbstractModule() {
  document_->setModule(nullptr);
}

ExecutionContext* AbstractModule::executionContext() const {
  return ContextLifecycleObserver::executionContext();
}

PassRefPtr<DartPromise> AbstractModule::import(DartState* dart_state,
                                               const String& url_string) {
  KURL url = document()->completeURL(url_string);
  RefPtr<DartPromiseResolver> resolver =
      DartPromiseResolver::Create(dart_state);
  OwnPtr<ModuleLoader> loader =
      adoptPtr(new ModuleLoader(this, GetApplication(), url));
  loaders_.set(loader.release(), resolver);
  return resolver->GetPromise();
}

void AbstractModule::OnModuleLoadComplete(ModuleLoader* loader,
                                          Module* module) {
  RefPtr<DartPromiseResolver> resolver = loaders_.take(loader);
  DartState::Scope scope(resolver->dart_state());
  // TODO(abarth): Resolve the promise.
}

} // namespace blink
