// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/builtin.h"

namespace mojo {
namespace dart {

Builtin::builtin_lib_props Builtin::builtin_libraries_[] = {
    /* { url_, has_natives_, native_symbol_, native_resolver_ } */
    {"dart:sky_builtin", true, Builtin::NativeSymbol, Builtin::NativeLookup}
};

void Builtin::SetNativeResolver(BuiltinLibraryId id) {
  static_assert((sizeof(builtin_libraries_) / sizeof(builtin_lib_props)) ==
                kInvalidLibrary, "Unexpected number of builtin libraries");
  DCHECK_GE(id, kBuiltinLibrary);
  DCHECK_LT(id, kInvalidLibrary);
  if (builtin_libraries_[id].has_natives_) {
    Dart_Handle url = Dart_NewStringFromCString(builtin_libraries_[id].url_);
    Dart_Handle library = Dart_LookupLibrary(url);
    DCHECK(!Dart_IsError(library));
    // Setup the native resolver for built in library functions.
    DART_CHECK_VALID(
        Dart_SetNativeResolver(library,
                               builtin_libraries_[id].native_resolver_,
                               builtin_libraries_[id].native_symbol_));
  }
}

Dart_Handle Builtin::LoadAndCheckLibrary(BuiltinLibraryId id) {
  static_assert((sizeof(builtin_libraries_) / sizeof(builtin_lib_props)) ==
                kInvalidLibrary, "Unexpected number of builtin libraries");
  DCHECK_GE(id, kBuiltinLibrary);
  DCHECK_LT(id, kInvalidLibrary);
  Dart_Handle url = Dart_NewStringFromCString(builtin_libraries_[id].url_);
  Dart_Handle library = Dart_LookupLibrary(url);
  DART_CHECK_VALID(library);
  return library;
}

}  // namespace dart
}  // namespace mojo
