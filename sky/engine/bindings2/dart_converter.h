// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_
#define SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_

#include "sky/engine/bindings2/dart_state.h"
#include "sky/engine/bindings2/dart_string.h"
#include "sky/engine/bindings2/dart_string_cache.h"
#include "sky/engine/bindings2/dart_value.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace blink {

// DartConvert converts types back and forth from Sky to Dart. The template
// parameter |T| determines what kind of type conversion to perform.
template<typename T, typename Enable = void>
struct DartConverter {};

template<>
struct DartConverter<bool> {
  static Dart_Handle ToDart(bool val) {
    return Dart_NewBoolean(val);
  }

  static void SetReturnValue(Dart_NativeArguments args, bool val) {
    Dart_SetBooleanReturnValue(args, val);
  }

  static bool FromArguments(Dart_NativeArguments args, int index, Dart_Handle& exception) {
    bool result = false;
    Dart_Handle handle = Dart_GetNativeBooleanArgument(args, index, &result);
    if (Dart_IsError(handle))
      return false;
    return result;
  }
};

template<typename T>
struct DartConverterInteger {
  static Dart_Handle ToDart(T val) {
    return Dart_NewInteger(val);
  }

  static void SetReturnValue(Dart_NativeArguments args, T val) {
    Dart_SetIntegerReturnValue(args, val);
  }

  static bool FromArguments(Dart_NativeArguments args, int index, Dart_Handle& exception) {
    int64_t result = 0;
    Dart_Handle handle = Dart_GetNativeIntegerArgument(args, index, &result);
    if (Dart_IsError(handle))
      return 0;
    return result;
  }
};

template<>
struct DartConverter<int> : public DartConverterInteger<int> { };

template<>
struct DartConverter<unsigned> : public DartConverterInteger<unsigned> { };

template<>
struct DartConverter<long long> : public DartConverterInteger<long long> { };

template<>
struct DartConverter<unsigned long long> {
  static Dart_Handle ToDart(unsigned long long val) {
    // FIXME: WebIDL unsigned long long is guaranteed to fit into 64-bit unsigned,
    // so we need a dart API for constructing an integer from uint64_t.
    DCHECK(val <= 0x7fffffffffffffffLL);
    return Dart_NewInteger(static_cast<int64_t>(val));
  }

  static void SetReturnValue(Dart_NativeArguments args,
                             unsigned long long val) {
    DCHECK(val <= 0x7fffffffffffffffLL);
    Dart_SetIntegerReturnValue(args, val);
  }

  static bool FromArguments(Dart_NativeArguments args, int index, Dart_Handle& exception) {
    int64_t result = 0;
    Dart_Handle handle = Dart_GetNativeIntegerArgument(args, index, &result);
    if (Dart_IsError(handle))
      return 0;
    return result;
  }
};

template<>
struct DartConverter<double> {
  static Dart_Handle ToDart(double val) {
    return Dart_NewDouble(val);
  }

  static void SetReturnValue(Dart_NativeArguments args, double val) {
    Dart_SetDoubleReturnValue(args, val);
  }

  static bool FromArguments(Dart_NativeArguments args, int index, Dart_Handle& exception) {
    double result = 0;
    Dart_Handle handle = Dart_GetNativeDoubleArgument(args, index, &result);
    if (Dart_IsError(handle))
      return 0;
    return result;
  }
};

template<>
struct DartConverter<String> {
  static Dart_Handle ToDart(DartState* state, const String& val) {
    if (val.isEmpty())
      return Dart_EmptyString();
    return Dart_HandleFromWeakPersistent(state->string_cache().Get(val.impl()));
  }

  static void SetReturnValue(Dart_NativeArguments args, const String& val, bool auto_scope = true) {
    // TODO(abarth): What should we do with auto_scope?
    if (val.isEmpty()) {
      Dart_SetReturnValue(args, Dart_EmptyString());
      return;
    }
    DartState* state = DartState::Current();
    Dart_SetWeakHandleReturnValue(args, state->string_cache().Get(val.impl()));
  }

  static void SetReturnValueWithNullCheck(Dart_NativeArguments args, const String& val, bool auto_scope = true) {
    if (val.isNull())
      Dart_SetReturnValue(args, Dart_Null());
    else
      SetReturnValue(args, val, auto_scope);
  }

  static String FromDart(Dart_Handle handle) {
    intptr_t char_size = 0;
    intptr_t length = 0;
    void* peer = nullptr;
    Dart_Handle result = Dart_StringGetProperties(handle, &char_size, &length, &peer);
    if (peer)
      return String(static_cast<StringImpl*>(peer));
    if (Dart_IsError(result))
      return String();
    return ExternalizeDartString(handle);
  }

  static String FromArguments(Dart_NativeArguments args, int index, Dart_Handle& exception, bool auto_scope = true) {
    // TODO(abarth): What should we do with auto_scope?
    void* peer = nullptr;
    Dart_Handle handle = Dart_GetNativeStringArgument(args, index, &peer);
    if (peer)
      return reinterpret_cast<StringImpl*>(peer);
    if (Dart_IsError(handle))
      return String();
    return ExternalizeDartString(handle);
  }

  static String FromArgumentsWithNullCheck(Dart_NativeArguments args, int index, Dart_Handle& exception, bool auto_scope = true) {
    // TODO(abarth): What should we do with auto_scope?
    void* peer = nullptr;
    Dart_Handle handle = Dart_GetNativeStringArgument(args, index, &peer);
    if (peer)
      return reinterpret_cast<StringImpl*>(peer);
    if (Dart_IsError(handle) || Dart_IsNull(handle))
        return String();
    return ExternalizeDartString(handle);
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

  static void SetReturnValue(Dart_NativeArguments args, DartValue* val) {
    Dart_SetReturnValue(args, val->dart_value());
  }
};

inline Dart_Handle StringToDart(DartState* state, const String& val) {
  return DartConverter<String>::ToDart(state, val);
}

inline Dart_Handle StringToDart(DartState* state, const AtomicString& val) {
  return DartConverter<AtomicString>::ToDart(state, val);
}

inline String StringFromDart(Dart_Handle handle) {
  return DartConverter<String>::FromDart(handle);
}

} // namespace blink

#endif // SKY_ENGINE_BINDINGS2_DART_CONVERTER_H_
