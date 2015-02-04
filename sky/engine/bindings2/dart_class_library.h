// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_CLASS_LIBRARY_H_
#define SKY_ENGINE_BINDINGS2_DART_CLASS_LIBRARY_H_

#include "dart/runtime/include/dart_api.h"

namespace blink {

class DartClassLibrary {
 public:
  DartClassLibrary();
  ~DartClassLibrary();

  Dart_PersistentHandle GetClassById(intptr_t class_id);

 private:
};

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_CLASS_LIBRARY_H_
