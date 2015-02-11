// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/tonic/dart_wrappable.h"

#include "sky/engine/tonic/dart_class_library.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/tonic/dart_wrapper_info.h"

namespace blink {

DartWrappable::~DartWrappable() {
  CHECK(!dart_wrapper_);
}

void DartWrappable::AcceptDartGCVisitor(DartGCVisitor& visitor) const {
}

Dart_Handle DartWrappable::CreateDartWrapper(DartState* dart_state) {
  DCHECK(!dart_wrapper_);
  const DartWrapperInfo& info = GetDartWrapperInfo();

  Dart_PersistentHandle type = dart_state->class_library().GetClass(info);
  DCHECK(!LogIfError(type));

  intptr_t native_fields[kNumberOfNativeFields];
  native_fields[kPeerIndex] = reinterpret_cast<intptr_t>(this);
  native_fields[kWrapperInfoIndex] = reinterpret_cast<intptr_t>(&info);
  Dart_Handle wrapper =
      Dart_AllocateWithNativeFields(type, kNumberOfNativeFields, native_fields);
  DCHECK(!LogIfError(wrapper));

  info.ref_object(this);  // Balanced in FinalizeDartWrapper.
  dart_wrapper_ = Dart_NewPrologueWeakPersistentHandle(
      wrapper, this, info.size_in_bytes, &FinalizeDartWrapper);

  return wrapper;
}

void DartWrappable::FinalizeDartWrapper(void* isolate_callback_data,
                                        Dart_WeakPersistentHandle wrapper,
                                        void* peer) {
  DartWrappable* wrappable = reinterpret_cast<DartWrappable*>(peer);
  wrappable->dart_wrapper_ = nullptr;
  const DartWrapperInfo& info = wrappable->GetDartWrapperInfo();
  info.deref_object(wrappable);  // Balanced in CreateDartWrapper.
}

}  // namespace blink
