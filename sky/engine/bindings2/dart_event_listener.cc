// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_event_listener.h"

namespace blink {

PassRefPtr<DartEventListener> DartEventListener::FromDart(Dart_Handle handle) {
  if (!Dart_IsClosure(handle))
    return nullptr;
  void* peer = nullptr;
  CHECK(!Dart_IsError(Dart_GetPeer(handle, &peer)));
  DartEventListener* listener = static_cast<DartEventListener*>(peer);
  if (listener)
    return listener;
  return adoptRef(new DartEventListener(handle));
}

DartEventListener::DartEventListener(Dart_Handle handle) {
  DCHECK(Dart_IsClosure(handle));
  ref();  // Balanced in Finalize
  closure_ = Dart_NewPrologueWeakPersistentHandle(handle, this, sizeof(*this),
                                                  &DartEventListener::Finalize);
  CHECK(!Dart_IsError(Dart_SetPeer(handle, this)));
}

DartEventListener::~DartEventListener() {
}

void DartEventListener::handleEvent(ExecutionContext*, Event*) {
}

void DartEventListener::Finalize(void* isolate_callback_data,
                                 Dart_WeakPersistentHandle handle,
                                 void* peer) {
  DartEventListener* listener = static_cast<DartEventListener*>(peer);
  listener->closure_ = nullptr;
  listener->deref();  // Balances ref in DartEventListener::DartEventListener
}

}  // namespace blink
