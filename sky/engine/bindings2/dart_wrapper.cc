// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/dart_wrapper.h"

#include "sky/engine/bindings2/dart_class_library.h"
#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/bindings2/dart_wrapper_info.h"

namespace blink {

Dart_Handle DartWrapper::CreateUnassociated(DartState* dart_state,
                                            const DartWrapperInfo& info,
                                            DartWrappableBase* peer) {
  Dart_PersistentHandle type =
      dart_state->class_library().GetClassById(info.class_id);
  DCHECK(!Dart_IsError(type));

  intptr_t native_fields[kNumberOfNativeFields];
  native_fields[kWrapperInfoIndex] = reinterpret_cast<intptr_t>(&info);
  native_fields[kPeerIndex] = reinterpret_cast<intptr_t>(peer);

  return Dart_AllocateWithNativeFields(
      type, kNumberOfNativeFields, native_fields);
}

} // namespace blink
