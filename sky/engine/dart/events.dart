// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart.sky;

// BEGIN STUBS -----------------------------------------------------------------

class DispatcherController<T> {
  Dispatcher<T> _dispatcher;
  Dispatcher<T> get dispatcher => _dispatcher;

  void add(T data) {
    // ...
  }
}

typedef bool Filter<T>(T t);
typedef void Handler<T>(T t);

class Dispatcher<T> {
  Dispatcher<T> where(Filter<T> filter) { return this; }
  void listen(Handler<T> handler) { }
}

class ExceptionListException<T> {
  List<T> _exceptions;
  void add(T exception) {
    if (_exceptions == null)
      _exceptions = new List<T>();
    _exceptions.add(exception);
  }
  int get length => _exceptions == null ? 0 : _exceptions.length;
}

// END STUBS -------------------------------------------------------------------

abstract class Event<ReturnType> {
  Event() { init(); }
  void init() { }

  bool get bubbles;

  EventTarget _target;
  EventTarget get target => _target;

  EventTarget _currentTarget;
  EventTarget get currentTarget => _currentTarget;

  bool handled; // precise semantics depend on the event type, but in general, set this when you set result
  ReturnType result;

  bool resultIsCompatible(dynamic candidate) => candidate is ReturnType;

  // TODO(ianh): abstract API for doing things at shadow tree boundaries 
  // TODO(ianh): do events get blocked at scope boundaries, e.g. focus events when both sides are in the scope?
  // TODO(ianh): do events get retargetted, e.g. focus when leaving a custom element?
}

class EventTarget extends NativeFieldWrapperClass2 {
  EventTarget() : _eventsController = new DispatcherController<Event>();

  Dispatcher get events => _eventsController.dispatcher;
  EventTarget parentNode;

  List<EventTarget> getEventDispatchChain() {
    if (this.parentNode == null) {
      return [this];
    } else {
      var result = this.parentNode.getEventDispatchChain();
      result.insert(0, this);
      return result;
    }
  }

  final DispatcherController _eventsController;

  dynamic dispatchEvent(@nonnull Event event, { dynamic defaultResult: null }) { // O(N*M) where N is the length of the chain and M is the average number of listeners per link in the chain
    // note: this will throw an ExceptionListException<ExceptionListException> if any of the listeners threw
    assert(event != null); // event must be non-null
    event.handled = false;
    assert(event.resultIsCompatible(defaultResult));
    event.result = defaultResult;
    event._target = this;
    var chain;
    if (event.bubbles)
      chain = this.getEventDispatchChain();
    else
      chain = [this];
    var exceptions = new ExceptionListException<ExceptionListException>();
    for (var link in chain) {
      try {
        link._dispatchEventLocally(event);
      } on ExceptionListException catch (e) {
        exceptions.add(e);
      }
    }
    if (exceptions.length > 0)
      throw exceptions;
    return event.result;
  }

  void _dispatchEventLocally(@nonnull Event event) {
    event._currentTarget = this;
    _eventsController.add(event);
  }
}
