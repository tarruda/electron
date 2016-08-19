#ifndef ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_
#define ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"
#include "content/public/renderer/render_frame_observer.h"

namespace atom {

class AtomSandboxedRendererClient : public content::ContentRendererClient {
 public:
  AtomSandboxedRendererClient();
  virtual ~AtomSandboxedRendererClient();

  DISALLOW_COPY_AND_ASSIGN(AtomSandboxedRendererClient);
};

}  // namespace atom

#endif  // ATOM_RENDERER_ATOM_SANDBOXED_RENDERER_CLIENT_H_
