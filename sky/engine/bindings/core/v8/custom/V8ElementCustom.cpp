/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sky/engine/config.h"
#include "bindings/core/v8/V8Element.h"

#include "bindings/core/v8/V8AnimationEffect.h"
#include "bindings/core/v8/V8AnimationPlayer.h"
#include "gen/sky/platform/RuntimeEnabledFeatures.h"
#include "sky/engine/bindings2/exception_state.h"
#include "sky/engine/bindings/core/v8/V8Binding.h"
#include "sky/engine/bindings/core/v8/V8BindingMacros.h"
#include "sky/engine/core/animation/ElementAnimation.h"
#include "sky/engine/core/dom/Element.h"
#include "sky/engine/wtf/GetPtr.h"

namespace blink {

////////////////////////////////////////////////////////////////////////////////
// Overload resolution for animate()
// FIXME: needs support for union types http://crbug.com/240176
////////////////////////////////////////////////////////////////////////////////

// AnimationPlayer animate(AnimationEffect? effect);
void animate1Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    Element* impl = V8Element::toNative(info.Holder());
    TONATIVE_VOID(AnimationEffect*, effect, V8AnimationEffect::toNativeWithTypeCheck(info.GetIsolate(), info[0]));
    v8SetReturnValueFast(info, WTF::getPtr(ElementAnimation::animate(*impl, effect)), impl);
}

// AnimationPlayer animate(AnimationEffect? effect, double timing);
void animate3Method(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    Element* impl = V8Element::toNative(info.Holder());
    TONATIVE_VOID(AnimationEffect*, effect, V8AnimationEffect::toNativeWithTypeCheck(info.GetIsolate(), info[0]));
    TONATIVE_VOID(double, duration, static_cast<double>(info[1]->NumberValue()));
    v8SetReturnValueFast(info, WTF::getPtr(ElementAnimation::animate(*impl, effect, duration)), impl);
}

void V8Element::animateMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate* isolate = info.GetIsolate();
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "animate", "Element");
    // AnimationPlayer animate(
    //     AnimationEffect? effect,
    //     optional double timing);
    switch (info.Length()) {
    case 1:
        // AnimationPlayer animate(AnimationEffect? effect);
        if (info[0]->IsNull()) {
            animate1Method(info);
            return;
        }
        // AnimationPlayer animate(AnimationEffect effect);
        if (V8AnimationEffect::hasInstance(info[0], isolate)) {
            animate1Method(info);
            return;
        }
        break;
    case 2:
        // As above, null resolved to AnimationEffect
        // AnimationPlayer animate(AnimationEffect? effect, double timing);
        if (info[0]->IsNull()) {
            animate3Method(info);
            return;
        }
        // AnimationPlayer animate(AnimationEffect effect, double timing);
        if (V8AnimationEffect::hasInstance(info[0], isolate)) {
            animate3Method(info);
            return;
        }
        break;
    default:
        setArityTypeError(exceptionState, "[1]", info.Length());
        exceptionState.ThrowIfNeeded();
        return;
        break;
    }
    exceptionState.ThrowTypeError("No function was found that matched the signature provided.");
    exceptionState.ThrowIfNeeded();
}

} // namespace blink
