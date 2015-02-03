/*
 * Copyright (C) 2009, 2011 Google Inc. All rights reserved.
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
#include "bindings/core/v8/V8Window.h"

#include "bindings/core/v8/V8Node.h"
#include "sky/engine/bindings/core/v8/BindingSecurity.h"
#include "sky/engine/bindings2/exception_messages.h"
#include "sky/engine/bindings2/exception_state.h"
#include "sky/engine/bindings2/scheduled_action.h"
#include "sky/engine/bindings/core/v8/ScriptController.h"
#include "sky/engine/bindings/core/v8/ScriptSourceCode.h"
#include "sky/engine/bindings/core/v8/SerializedScriptValue.h"
#include "sky/engine/bindings/core/v8/V8Binding.h"
#include "sky/engine/bindings/core/v8/V8GCForContextDispose.h"
#include "sky/engine/bindings/core/v8/V8HiddenValue.h"
#include "sky/engine/core/dom/Element.h"
#include "sky/engine/core/dom/ExceptionCode.h"
#include "sky/engine/core/dom/Node.h"
#include "sky/engine/core/frame/DOMTimer.h"
#include "sky/engine/core/frame/DOMWindowTimers.h"
#include "sky/engine/core/frame/FrameView.h"
#include "sky/engine/core/frame/LocalDOMWindow.h"
#include "sky/engine/core/frame/LocalFrame.h"
#include "sky/engine/core/frame/Settings.h"
#include "sky/engine/platform/PlatformScreen.h"
#include "sky/engine/wtf/ArrayBuffer.h"
#include "sky/engine/wtf/Assertions.h"
#include "sky/engine/wtf/OwnPtr.h"

namespace blink {

// FIXME: There is a lot of duplication with SetTimeoutOrInterval() in V8WorkerGlobalScopeCustom.cpp.
// We should refactor this.
static void windowSetTimeoutImpl(const v8::FunctionCallbackInfo<v8::Value>& info, bool singleShot, ExceptionState& exceptionState)
{
}

// FIXME(fqian): returning string is cheating, and we should
// fix this by calling toString function on the receiver.
// However, V8 implements toString in JavaScript, which requires
// switching context of receiver. I consider it is dangerous.
void V8Window::toStringMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> domWrapper = V8Window::findInstanceInPrototypeChain(info.This(), info.GetIsolate());
    if (domWrapper.IsEmpty()) {
        v8SetReturnValue(info, info.This()->ObjectProtoToString());
        return;
    }
    v8SetReturnValue(info, domWrapper->ObjectProtoToString());
}

void V8Window::namedPropertyGetterCustom(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
}

void V8Window::setTimeoutMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "setTimeout", "Window");
    windowSetTimeoutImpl(info, true, exceptionState);
    exceptionState.ThrowIfNeeded();
}


void V8Window::setIntervalMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ExecutionContext, "setInterval", "Window");
    windowSetTimeoutImpl(info, false, exceptionState);
    exceptionState.ThrowIfNeeded();
}

bool V8Window::namedSecurityCheckCustom(v8::Local<v8::Object> host, v8::Local<v8::Value> key, v8::AccessType type, v8::Local<v8::Value>)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Handle<v8::Object> window = V8Window::findInstanceInPrototypeChain(host, isolate);
    if (window.IsEmpty())
        return false; // the frame is gone.

    LocalDOMWindow* targetWindow = V8Window::toNative(window);

    ASSERT(targetWindow);

    LocalFrame* target = targetWindow->frame();
    if (!target)
        return false;

    return BindingSecurity::shouldAllowAccessToFrame(isolate, target, DoNotReportSecurityError);
}

bool V8Window::indexedSecurityCheckCustom(v8::Local<v8::Object> host, uint32_t index, v8::AccessType type, v8::Local<v8::Value>)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Handle<v8::Object> window = V8Window::findInstanceInPrototypeChain(host, isolate);
    if (window.IsEmpty())
        return false;

    LocalDOMWindow* targetWindow = V8Window::toNative(window);

    ASSERT(targetWindow);

    LocalFrame* target = targetWindow->frame();
    if (!target)
        return false;

    return BindingSecurity::shouldAllowAccessToFrame(isolate, target, DoNotReportSecurityError);
}

v8::Handle<v8::Value> toV8(LocalDOMWindow* window, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    // Notice that we explicitly ignore creationContext because the LocalDOMWindow is its own creationContext.

    if (!window)
        return v8::Null(isolate);
    // Initializes environment of a frame, and return the global object
    // of the frame.
    LocalFrame* frame = window->frame();
    if (!frame)
        return v8Undefined();

    v8::Handle<v8::Context> context = toV8Context(frame, DOMWrapperWorld::current(isolate));
    if (context.IsEmpty())
        return v8Undefined();

    v8::Handle<v8::Object> global = context->Global();
    ASSERT(!global.IsEmpty());
    return global;
}

} // namespace blink
