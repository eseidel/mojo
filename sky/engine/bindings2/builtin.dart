// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library sky_builtin;

// Corelib 'print' implementation.
void _print(arg) {
  _Logger._printString(arg.toString());
}

class _Logger {
  static void _printString(String s) native "Logger_PrintString";
}

void _scheduleMicrotask(void callback()) native "ScheduleMicrotask";

_getPrintClosure() => _print;
_getScheduleMicrotaskClosure() => _scheduleMicrotask;
