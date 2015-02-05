// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_EVENT_LISTENER_H_
#define SKY_ENGINE_BINDINGS2_DART_EVENT_LISTENER_H_

#include "dart/runtime/include/dart_api.h"
#include "sky/engine/core/events/EventListener.h"

namespace blink {

class DartEventListener : public EventListener {
 public:
  ~DartEventListener() override;

  bool operator==(const EventListener& other) override { return this == &other; }
  void handleEvent(ExecutionContext*, Event*) override;

 private:
  DartEventListener();
};

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_EVENT_LISTENER_H_
