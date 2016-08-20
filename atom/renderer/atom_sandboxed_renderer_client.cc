// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/renderer/atom_sandboxed_renderer_client.h"

#include <string>
#include <vector>

#include "atom/renderer/atom_render_view_observer.h"
#include "atom/renderer/api/atom_api_renderer_ipc.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "ipc/ipc_message_macros.h"
#include "atom/common/api/api_messages.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebKit.h"
#include "atom_natives.h"

namespace atom {

namespace {

class AtomSandboxedRenderFrameObserver : public content::RenderFrameObserver {
 public:
  AtomSandboxedRenderFrameObserver(content::RenderFrame* frame,
                                   AtomSandboxedRendererClient* renderer_client)
      : content::RenderFrameObserver(frame),
        render_frame_(frame),
        renderer_client_(renderer_client) {}

  // content::RenderFrameObserver:
  void DidCreateScriptContext(v8::Handle<v8::Context> context,
                              int extension_group,
                              int world_id) override {
    renderer_client_->DidCreateScriptContext(context, render_frame_);
  }

 private:
  content::RenderFrame* render_frame_;
  AtomSandboxedRendererClient* renderer_client_;

  DISALLOW_COPY_AND_ASSIGN(AtomSandboxedRenderFrameObserver);
};

}  // namespace


AtomSandboxedRendererClient::AtomSandboxedRendererClient() {
}

AtomSandboxedRendererClient::~AtomSandboxedRendererClient() {
}

void AtomSandboxedRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  new AtomSandboxedRenderFrameObserver(render_frame, this);
}

void AtomSandboxedRendererClient::RenderViewCreated(
    content::RenderView* render_view) {
  new AtomRenderViewObserver(render_view, nullptr);
}

void AtomSandboxedRendererClient::DidCreateScriptContext(
    v8::Handle<v8::Context> context, content::RenderFrame* render_frame) {
  auto isolate = context->GetIsolate();
  v8::HandleScope handle_scope(isolate);
  v8::Context::Scope context_scope(context);
  // Wrap the bundle into a function that receives the native IPC object, to
  // avoid polluting the global namespace.
  std::string preload_bundle_native(node::preload_bundle_native,
      node::preload_bundle_native + sizeof(node::preload_bundle_native));
  std::stringstream ss;
  ss << "(function(binding, preloadEntry) {\n";
  ss << "  let ipcRenderer\n";
  ss << preload_bundle_native << "\n";
  ss << "  return ipcRenderer\n";
  ss << "})";
  std::string preload_wrapper = ss.str();
  // Compile the preload wrapper function
  auto script = v8::Script::Compile(v8::String::NewFromUtf8(
        isolate,
        preload_wrapper.c_str(),
        v8::String::kNormalString,
        preload_wrapper.length()));
  // Execute to get a reference to the wrapper function.
  auto result = script->Run(context).ToLocalChecked();
  auto func = v8::Handle<v8::Function>::Cast(result);
  // Create and initialize the IPC object
  auto ipc_binding = v8::Object::New(isolate);
  api::Initialize(ipc_binding, v8::Null(isolate), context, nullptr);
  v8::Local<v8::Value> args[] = {ipc_binding};
  // Execute it, passing the IPC object as argument.
  auto ipc = func->Call(context, v8::Null(isolate), 1, args).ToLocalChecked();
  // Privately store the ipc object.
  auto ipc_key = v8::String::NewFromUtf8(isolate, "ipc",
      v8::NewStringType::kNormal).ToLocalChecked();
  auto private_ipc_key = v8::Private::ForApi(isolate, ipc_key);
  context->Global()->SetPrivate(context, private_ipc_key, ipc);
}

}  // namespace atom
