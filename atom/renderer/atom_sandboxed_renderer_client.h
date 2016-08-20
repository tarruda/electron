#ifndef ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_
#define ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"
#include "content/public/renderer/render_frame_observer.h"
#include "base/values.h"

namespace atom {

class AtomSandboxedRendererClient : public content::ContentRendererClient {
 public:
  AtomSandboxedRendererClient();
  virtual ~AtomSandboxedRendererClient();

  void DidCreateScriptContext(
      v8::Handle<v8::Context> context, content::RenderFrame* render_frame);
  void EmitIPCEvent(v8::Handle<v8::Context> context,
                    const base::string16& channel,
                    const base::ListValue& args);

  // content::ContentRendererClient:
  void RenderFrameCreated(content::RenderFrame*) override;
  void RenderViewCreated(content::RenderView*) override;

  DISALLOW_COPY_AND_ASSIGN(AtomSandboxedRendererClient);
};

}  // namespace atom

#endif  // ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_
