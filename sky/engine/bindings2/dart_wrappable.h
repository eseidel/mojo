// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
#define SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_

#include "base/logging.h"
#include "base/template_util.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/bindings2/dart_converter.h"
#include "sky/engine/bindings2/dart_state.h"

namespace blink {
struct DartWrapperInfo;

// DartWrappable is a base class that you can inherit from in order to be
// exposed to Dart code as an interface.
class DartWrappable {
 public:
  enum DartNativeFields {
    kWrapperInfoIndex,
    kPeerIndex,
    kNumberOfNativeFields,
  };

  DartWrappable() : dart_wrapper_(nullptr) {}

  virtual const DartWrapperInfo& GetDartWrapperInfo() const;

  Dart_Handle Wrap(DartState* dart_state);
  Dart_WeakPersistentHandle dart_wrapper() const { return dart_wrapper_; }

  void set_dart_wrapper(Dart_WeakPersistentHandle dart_wrapper) {
    DCHECK(!dart_wrapper_ || !dart_wrapper);
    dart_wrapper_ = dart_wrapper;
  }

 protected:
  virtual ~DartWrappable();

 private:
  Dart_WeakPersistentHandle dart_wrapper_;
};

#define DEFINE_WRAPPERTYPEINFO()

template <typename T>
struct DartConverter<
    T*,
    typename base::enable_if<
        base::is_convertible<T*, DartWrappable*>::value>::type> {
  static Dart_Handle ToDart(DartWrappable* val) {
    if (Dart_WeakPersistentHandle wrapper = val->dart_wrapper())
      return Dart_HandleFromWeakPersistent(wrapper);
    return val->Wrap(DartState::Current());
  }

  static void SetReturnValue(Dart_NativeArguments args,
                             DartWrappable* val,
                             bool auto_scope = true) {
    if (Dart_WeakPersistentHandle wrapper = val->dart_wrapper())
      Dart_SetWeakHandleReturnValue(args, wrapper);
    else
      Dart_SetReturnValue(args, val->Wrap(DartState::Current()));
  }

  static T* FromDart(Dart_Handle handle) {
    intptr_t* peer = 0;
    Dart_Handle result =
        Dart_GetNativeInstanceField(handle, DartWrappable::kPeerIndex, peer);
    // TODO(abarth): Add error checking.
    DCHECK(!Dart_IsError(result));
    return static_cast<T*>(reinterpret_cast<DartWrappable*>(peer));
  }

  static T* FromArguments(Dart_NativeArguments args,
                          int index,
                          Dart_Handle& exception,
                          bool auto_scope = true) {
    intptr_t native_fields[DartWrappable::kNumberOfNativeFields];
    Dart_Handle result = Dart_GetNativeFieldsOfArgument(
        args, index, DartWrappable::kNumberOfNativeFields, native_fields);
    // TODO(abarth): Add error checking.
    DCHECK(!Dart_IsError(result));
    return static_cast<T*>(reinterpret_cast<DartWrappable*>(
        native_fields[DartWrappable::kPeerIndex]));
  }

  static T* FromArgumentsWithNullCheck(Dart_NativeArguments args,
                                       int index,
                                       Dart_Handle& exception,
                                       bool auto_scope = true) {
    intptr_t native_fields[DartWrappable::kNumberOfNativeFields];
    Dart_Handle result = Dart_GetNativeFieldsOfArgument(
        args, index, DartWrappable::kNumberOfNativeFields, native_fields);
    if (Dart_IsNull(result))
      return nullptr;
    // TODO(abarth): Add error checking.
    DCHECK(!Dart_IsError(result));
    return static_cast<T*>(reinterpret_cast<DartWrappable*>(
        native_fields[DartWrappable::kPeerIndex]));
  }
};

template <typename T>
struct DartConverter<RefPtr<T>> {
  static Dart_Handle ToDart(RefPtr<T> val) {
    return DartConverter<T*>::ToDart(val.get());
  }
};

template <typename T>
static T* GetReceiver(Dart_NativeArguments args) {
  intptr_t receiver;
  Dart_Handle result = Dart_GetNativeReceiver(args, &receiver);
  DCHECK(!Dart_IsError(result));
  return static_cast<T*>(reinterpret_cast<DartWrappable*>(receiver));
}

}  // namespace blink

#endif  // SKY_ENGINE_BINDINGS2_DART_WRAPPABLE_H_
