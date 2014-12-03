// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

define("mojo/services/public/js/shell", [
  "mojo/public/interfaces/application/service_provider.mojom",
  "mojo/services/public/js/service_provider",
], function(spInterfaceModule, spModule) {

  function Shell(appShell) {
    this.appShell_ = appShell;
    this.applications_ = new Map();
  }

  Shell.prototype.connectToApplication = function(url) {
    var application = this.applications_.get(url);
    if (application)
      return application;

    var proxy = new spInterfaceModule.ServiceProvider.proxyClass;
    this.appShell_.connectToApplication(url, proxy);
    application = new spModule.ServiceProvider(proxy);
    this.applications_.set(url, application);
    return application;
  };

  Shell.prototype.connectToService = function (url, service, client) {
    return this.connectToApplication(url).connectToService(service, client);
  };

  Shell.prototype.close = function() {
    this.applications_.forEach(function(application, url) {
      application.close();
    });
    this.applications_.clear();
  };

  var exports = {};
  exports.Shell = Shell;
  return exports;
});