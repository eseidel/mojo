// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPER_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPER_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"

namespace blink {
class DartState;
class DartWrappableBase;
struct DartWrapperInfo;

class DartWrapper {
 public:
  enum NativeFields {
    kWrapperInfoIndex,
    kPeerIndex,
    kNumberOfNativeFields,
  };

  static DartWrappableBase* GetPeer(Dart_Handle handle) {
    intptr_t peer = 0;
    Dart_Handle result = Dart_GetNativeInstanceField(handle, kPeerIndex, &peer);
    DCHECK(!Dart_IsError(result));
    return reinterpret_cast<DartWrappableBase*>(peer);
  }

  static Dart_Handle CreateUnassociated(DartState* dart_state,
                                        const DartWrapperInfo& info,
                                        DartWrappableBase* peer);
};


} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_WRAPPER_H_
