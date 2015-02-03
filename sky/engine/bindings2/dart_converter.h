// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_
#define SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_

#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/bindings2/dart_string_cache.h"
#include "sky/engine/bindings2/dart_value.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

// DartConvert converts types back and forth from Sky to Dart. The template
// parameter |T| determines what kind of type conversion to perform.
template<typename T>
struct DartConverter {};

template<>
struct DartConverter<bool> {
  static Dart_Handle ToDart(DartState* state, bool val) {
    return Dart_NewBoolean(val);
  }
};

template<>
struct DartConverter<int> {
  static Dart_Handle ToDart(DartState* state, int val) {
    return Dart_NewInteger(val);
  }
};

template<>
struct DartConverter<unsigned> {
  static Dart_Handle ToDart(DartState* state, unsigned val) {
    return Dart_NewInteger(val);
  }
};

template<>
struct DartConverter<long long> {
  static Dart_Handle ToDart(DartState* state, long long val) {
    return Dart_NewInteger(val);
  }
};

template<>
struct DartConverter<unsigned long long> {
  static Dart_Handle ToDart(DartState* state, unsigned long long val) {
    // FIXME: WebIDL unsigned long long is guaranteed to fit into 64-bit unsigned,
    // so we need a dart API for constructing an integer from uint64_t.
    DCHECK(val <= 0x7fffffffffffffffLL);
    return Dart_NewInteger(static_cast<int64_t>(val));
  }
};

template<>
struct DartConverter<double> {
  static Dart_Handle ToDart(DartState* state, double val) {
    return Dart_NewDouble(val);
  }
};

template<>
struct DartConverter<String> {
  static Dart_Handle ToDart(DartState* state, const String& val) {
    if (val.isEmpty())
      return Dart_EmptyString();
    return Dart_HandleFromWeakPersistent(state->string_cache().Get(val.impl()));
  }
};

template<>
struct DartConverter<AtomicString> {
  static Dart_Handle ToDart(DartState* state, const AtomicString& val) {
    return DartConverter<String>::ToDart(state, val.string());
  }
};

template<typename T>
struct DartConverter<Vector<T>> {
  static Dart_Handle ToDart(DartState* state, const Vector<T>& val) {
    Dart_Handle list = Dart_NewList(val.size());
    if (Dart_IsError(list))
        return list;
    for (size_t i = 0; i < val.size(); i++) {
        Dart_Handle result =
            Dart_ListSetAt(list, i, DartConverter<T>::ToDart(val[i]));
        if (Dart_IsError(result))
            return result;
    }
    return list;
  }
};

template<>
struct DartConverter<DartValue> {
  static Dart_Handle ToDart(DartState* state, DartValue* val) {
    return val->dart_value();
  }
};

inline Dart_Handle StringToDart(DartState* state, const String& val) {
  return DartConverter<String>::ToDart(state, val);
}

inline Dart_Handle StringToDart(DartState* state, const AtomicString& val) {
  return DartConverter<AtomicString>::ToDart(state, val);
}

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_
