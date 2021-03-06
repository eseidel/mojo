// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/kiosk_wm/kiosk_wm.h"

#include "services/window_manager/basic_focus_rules.h"

namespace mojo {
namespace kiosk_wm {

KioskWM::KioskWM()
    : window_manager_app_(new window_manager::WindowManagerApp(this, this)),
      root_(nullptr),
      content_(nullptr),
      navigator_host_factory_(this),
      weak_factory_(this) {
  exposed_services_impl_.AddService(&navigator_host_factory_);
}

KioskWM::~KioskWM() {
}

base::WeakPtr<KioskWM> KioskWM::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void KioskWM::Initialize(mojo::ApplicationImpl* app) {
  window_manager_app_->Initialize(app);

  // Format: --args-for="app_url default_url"
  if (app->args().size() > 1)
    default_url_ = app->args()[1];
}

bool KioskWM::ConfigureIncomingConnection(
    mojo::ApplicationConnection* connection) {
  window_manager_app_->ConfigureIncomingConnection(connection);
  return true;
}

bool KioskWM::ConfigureOutgoingConnection(
    mojo::ApplicationConnection* connection) {
  window_manager_app_->ConfigureOutgoingConnection(connection);
  return true;
}

void KioskWM::OnEmbed(
    mojo::View* root,
    mojo::InterfaceRequest<mojo::ServiceProvider> services,
    mojo::ServiceProviderPtr exposed_services) {
  // KioskWM does not support being embedded more than once.
  CHECK(!root_);

  root_ = root;
  root_->AddObserver(this);

  // Resize to match the Nexus 5 aspect ratio:
  window_manager_app_->SetViewportSize(gfx::Size(320, 640));

  content_ = root->view_manager()->CreateView();
  content_->SetBounds(root_->bounds());
  root_->AddChild(content_);
  content_->SetVisible(true);

  window_manager_app_->InitFocus(
      make_scoped_ptr(new window_manager::BasicFocusRules(root_)));

  // Now that we're ready, either load a pending url or the default url.
  if (!pending_url_.empty())
    ReplaceContentWithURL(pending_url_);
  else if (!default_url_.empty())
    ReplaceContentWithURL(default_url_);
}

void KioskWM::Embed(
    const mojo::String& url,
    mojo::InterfaceRequest<mojo::ServiceProvider> ignored_incoming_services,
    mojo::ServiceProviderPtr ignored_exposed_services) {
  // We can get Embed calls before we've actually been
  // embedded into the root view and content_ is created.
  // Just save the last url, we'll embed it when we're ready.
  if (!content_) {
    pending_url_ = url;
    return;
  }

  mojo::ServiceProviderPtr exposed_services;
  exposed_services_impl_.Bind(GetProxy(&exposed_services));
  content_->Embed(url, nullptr, exposed_services.Pass());
}

void KioskWM::OnViewManagerDisconnected(
    mojo::ViewManager* view_manager) {
  root_ = nullptr;
}

void KioskWM::OnViewDestroyed(mojo::View* view) {
  view->RemoveObserver(this);
}

void KioskWM::OnViewBoundsChanged(mojo::View* view,
                                              const mojo::Rect& old_bounds,
                                              const mojo::Rect& new_bounds) {
  content_->SetBounds(new_bounds);
}

// Convenience method:
void KioskWM::ReplaceContentWithURL(const mojo::String& url) {
  Embed(url, nullptr, nullptr);
}

}  // namespace kiosk_wm
}  // namespace mojo
