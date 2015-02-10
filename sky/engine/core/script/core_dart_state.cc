// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/core/script/core_dart_state.h"

#include "sky/engine/core/dom/Document.h"
#include "sky/engine/core/script/dart_loader.h"

namespace blink {

CoreDartState::CoreDartState(Document* document)
    : document_(document), loader_(adoptPtr(new DartLoader(this))) {
  DCHECK(document_);
}

CoreDartState::~CoreDartState() {
}

CoreDartState* CoreDartState::Current() {
  return static_cast<CoreDartState*>(DartState::Current());
}

Document* CoreDartState::CurrentDocument() {
  return Current()->document_.get();
}

LocalFrame* CoreDartState::CurrentFrame() {
  return Current()->document_->frame();
}

LocalDOMWindow* CoreDartState::CurrentWindow() {
  return Current()->document_->domWindow();
}

}
