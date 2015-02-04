// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_wrappable.h"

#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/bindings2/dart_wrapper.h"
#include "sky/engine/bindings2/dart_wrapper_info.h"

namespace blink {
namespace {
// TODO(abarth): Each IDL interface should generate its own DartWrapperInfo.
DartWrapperInfo kSingletonWrapperInfo = {0, 0, nullptr, nullptr};

void FinalizeWrapper(void* isolate_callback_data,
                     Dart_WeakPersistentHandle wrapper,
                     void* peer_value)
{
  DartWrappableBase* peer = reinterpret_cast<DartWrappableBase*>(peer_value);
  DCHECK(peer = DartWrapper::GetPeer(Dart_HandleFromWeakPersistent(wrapper)));
  DartWrappable* wrappable = static_cast<DartWrappable*>(peer);
  wrappable->set_dart_wrapper(nullptr);
  const DartWrapperInfo& info = wrappable->GetDartWrapperInfo();
  info.deref_object(peer);
}

}

DartWrappable::~DartWrappable() {
  CHECK(!dart_wrapper_);
}

const DartWrapperInfo& DartWrappable::GetDartWrapperInfo() const {
  return kSingletonWrapperInfo;
}

Dart_Handle DartWrappable::Wrap(DartState* dart_state) {
  DCHECK(!dart_wrapper_);
  const DartWrapperInfo& info = GetDartWrapperInfo();
  DartWrappableBase* peer = this;

  Dart_Handle wrapper = DartWrapper::CreateUnassociated(dart_state, info, peer);
  DCHECK(!Dart_IsError(wrapper));

  info.ref_object(peer);  // Balanced in FinalizeWrapper.
  dart_wrapper_ = Dart_NewPrologueWeakPersistentHandle(
      wrapper, peer, info.size_in_bytes, &FinalizeWrapper);

  return wrapper;
}

} // namespace blink
