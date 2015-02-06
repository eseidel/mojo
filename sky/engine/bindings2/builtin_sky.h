// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_BUILTIN_SKY_H_
#define SKY_ENGINE_BINDINGS2_BUILTIN_SKY_H_

#include "base/macros.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_class_provider.h"
#include "sky/engine/tonic/dart_persistent_value.h"

namespace blink {

class BuiltinSky : public DartClassProvider {
 public:
  explicit BuiltinSky(DartState* dart_state);
  ~BuiltinSky();

  static Dart_NativeFunction NativeLookup(Dart_Handle name,
                                          int argument_count,
                                          bool* auto_setup_scope);

  static const uint8_t* NativeSymbol(Dart_NativeFunction nf);

  // DartClassProvider:
  Dart_Handle GetClassByName(const char* class_name) override;

 private:
  DartPersistentValue library_;

  DISALLOW_COPY_AND_ASSIGN(BuiltinSky);
};

}  // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_BUILTIN_SKY_H_
