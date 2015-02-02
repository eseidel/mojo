// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_CORE_SCRIPT_DART_CONTROLLER_H_
#define SKY_ENGINE_CORE_SCRIPT_DART_CONTROLLER_H_

#include "dart/runtime/include/dart_api.h"

#include "sky/engine/wtf/text/AtomicString.h"
#include "sky/engine/wtf/text/TextPosition.h"

namespace blink {

class AbstractModule;

class DartController {
public:

  static void InitVM();

  DartController();
  ~DartController();

  void executeModuleScript(AbstractModule& module, const String& source, const TextPosition& textPosition);

  void clearForClose();

};

}

#endif // SKY_ENGINE_CORE_SCRIPT_DART_CONTROLLER_H_
