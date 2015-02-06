// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/config.h"
#include "sky/engine/bindings2/builtin_sky.h"

#include "sky/engine/bindings2/builtin.h"

namespace blink {

BuiltinSky::BuiltinSky(DartState* dart_state) {
  Builtin::SetNativeResolver(Builtin::kSkyLibrary);
  library_.Set(dart_state, Builtin::LoadAndCheckLibrary(Builtin::kSkyLibrary));
}

BuiltinSky::~BuiltinSky() {
}

Dart_Handle BuiltinSky::GetClassByName(const char* class_name) {
  Dart_Handle name_handle = Dart_NewStringFromCString(class_name);
  Dart_Handle class_handle = Dart_GetType(library_.value(), name_handle, 0, nullptr);
  DCHECK(!Dart_IsError(class_handle)) << class_name;
  return class_handle;
}

}  // namespace blink
