// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_

#include "flutter/flow/embedded_views.h"
#include "flutter/fml/platform/darwin/scoped_nsobject.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterBinaryMessenger.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterChannels.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPlatformViews.h"

// A UIView that is used as the parent for embedded UIViews.
//
// This view has 2 roles:
// 1. Delay or prevent touch events from arriving the embedded view.
// 2. Dispatching all events that are hittested to the embedded view to the FlutterView.
@interface FlutterTouchInterceptingView : UIView
- (instancetype)initWithEmbeddedView:(UIView*)embeddedView flutterView:(UIView*)flutterView;

// Stop delaying any active touch sequence (and let it arrive the embedded view).
- (void)releaseGesture;
@end

namespace shell {

class IOSSurface;

struct FlutterPlatformViewLayer {
  FlutterPlatformViewLayer(UIView* overlay_view,
                           std::unique_ptr<IOSSurface> ios_surface,
                           std::unique_ptr<Surface> surface)
      : overlay_view([overlay_view retain]),
        ios_surface(std::move(ios_surface)),
        surface(std::move(surface)){};

  fml::scoped_nsobject<UIView> overlay_view;
  std::unique_ptr<IOSSurface> ios_surface;
  std::unique_ptr<Surface> surface;
};

class FlutterPlatformViewsController {
 public:
  FlutterPlatformViewsController() = default;

  void SetFlutterView(UIView* flutter_view);

  void RegisterViewFactory(NSObject<FlutterPlatformViewFactory>* factory, NSString* factoryId);

  SkCanvas* CompositeEmbeddedView(int view_id,
                                  const flow::EmbeddedViewParams& params,
                                  IOSSurface& surface);

  bool Present();

  void OnMethodCall(FlutterMethodCall* call, FlutterResult& result);

 private:
  fml::scoped_nsobject<FlutterMethodChannel> channel_;
  fml::scoped_nsobject<UIView> flutter_view_;
  std::map<std::string, fml::scoped_nsobject<NSObject<FlutterPlatformViewFactory>>> factories_;
  std::map<int64_t, fml::scoped_nsobject<FlutterTouchInterceptingView>> views_;
  std::map<int64_t, std::unique_ptr<FlutterPlatformViewLayer>> overlays_;

  // A vector of embedded view IDs according to their composition order.
  // The last ID in this vector belond to the that is composited on top of all others.
  std::vector<int64_t> composition_order_;

  // The latest composition order that was presented in Present().
  std::vector<int64_t> active_composition_order_;

  std::vector<std::unique_ptr<SurfaceFrame>> composition_frames_;

  void OnCreate(FlutterMethodCall* call, FlutterResult& result);
  void OnDispose(FlutterMethodCall* call, FlutterResult& result);
  void OnAcceptGesture(FlutterMethodCall* call, FlutterResult& result);

  void EnsureOverlayInitialized(int64_t overlay_id);

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterPlatformViewsController);
};

}  // namespace shell

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_IOS_FRAMEWORK_SOURCE_FLUTTERPLATFORMVIEWS_INTERNAL_H_
