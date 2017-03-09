# `sandbox` Option

> Create a browser window with renderer that can run inside chromium OS sandbox.

One of chromium key security features is an OS-provided sandbox that it uses to
confine all blink rendering/javascript code. This sandbox uses OS-specific
features to ensure the renderer process can only do one of the following:

- Consume CPU
- Consume Memory
- Talk to the main process via IPC pipes.

That means a renderer process in chromium can only affect operating system by
delegating tasks to the main process via IPC.
[Here's](https://www.chromium.org/developers/design-documents/sandbox) more
information about the sandbox.

Since one of electron main features is the ability to run node.js in the
renderer process(making it easier to develop desktop applications using only web
technologies), the sandbox has to disabled by electron. One of the reasons is
that most node.js APIs require system access. `require()` for example, is not
possible without file system permissions, which are unavailable in a sandboxed
environment.

Usually this is not a problem for desktop applications since the code is always
trusted, but it makes electron less secure than chromium for displaying
untrusted web content. To fill this gap(to a certain extent), the `sandbox` flag
will force electron to spawn a classic chromium renderer that is compatible with
the sandbox.

A sandboxed renderer doesn't have a node.js environment running and doesn't
expose javascript APIs to client code. The only exception is the preload script,
which has access to a subset of electron renderer API.

Another difference is that sandboxed renderers don't modify any of the default
javascript APIs. Consequently, some APIs such as `window.open` will work as they
do in chromium(no `BrowserWindowProxy`).

## Example

Create a sandboxed window, simply pass `sandbox: true` to `webPreferences`:

```js
let win
app.on('ready', () => {
  win = new BrowserWindow({
    webPreferences: {
      sandbox: true
    }
  })
  w.loadURL('http://google.com')
})
```

This alone won't enable the OS-level sandbox. If this feature is desired, the
`--enable-sandbox` command-line argument must be passed to electron, which will
force `sandbox: true` to all BrowserWindow instances.

```js
let win
app.on('ready', () => {
  // no need to pass `sandbox: true` since `--enable-sandbox` was enabled.
  win = new BrowserWindow()
  w.loadURL('http://google.com')
})
```

Note that is is not enough to call
`app.commandLine.appendSwitch('--enable-sandbox')`, as electron/node startup
code runs after it is possible to make changes to chromium sandbox settings. The
switch must be passed to electron command-line:

```
electron --enable-sandbox app.js
```

It is not possible to have the OS-level sandbox active only for some renderers,
if `--enable-sandbox` is enabled, normal electron windows cannot be created.

If you need mix sandboxed and non-sandboxed renderers in one application, simply
omit the `--enable-sandbox` argument. Without this argument, windows created
with `sandbox: true` will still have node.js disabled and communicate only via
IPC, which by itself is already a gain from security POV.

## Preload

The app can make customizations to sandboxed renderers using a preload script.
Here's an example:

```js
let win
app.on('ready', () => {
  win = new BrowserWindow({
    webPreferences: {
      sandbox: true,
      preload: 'preload.js'
    }
  })
  w.loadURL('http://google.com')
})
```

and preload.js:

```js
// This file is loaded whenever a javascript context is created. It runs in a
// scope that is not available to normal javascript loaded from web pages, so it
// can can access a subset of electron renderer APIs. We must be careful to not
// leak any objects into the global object!
const {ipcRenderer, remote} = require('electron')

// read some plugin
// any API/module from
let buf = remote.require('fs').readFileSync('allowed-popup-urls.json')
const allowedUrls = JSON.parse(buf.toString('utf8'))

const defaultWindowOpen = window.open

function customWindowOpen (url, ...args) {
  if (allowedUrls.indexOf(url) === -1) {
    ipcRenderer.sendSync('blocked-popup-notification', location.origin, url)
    return null
  }
  return defaultWindowOpen(url, ...args)
}

window.open = customWindowOpen
```

Important things to notice in the above file:

- Even though the sandboxed renderer doesn't have node.js running, it still has
  access to a limited node-like environment:`Buffer`, `process` and `require`
  are available.
- The preload can indirectly access all APIs from the main process through the
  `remote` and `ipcRenderer` modules.
- The preload must be contained in a single script, but it is possible to have
  complex preload code composed with multiple modules by using browserify to
  create the bundle. If this is desired, you should pass certain flags to ensure
  browserify won't include unnecessary code:

      browserify preload/index.js -x electron --insert-global-vars=__filename,__dirname -o preload.js

  Explanation:

  `-x electron` will stop browserify from trying to include the `electron`
  module(falling back to the `require` function provided to the preload), and
  `--insert-global-vars=__filename,__dirname` will make sure only
  `__filename`/`__dirname` are included(`Buffer` and `process` are also provided
  to preload)

## Status

Please use the `sandbox` option with care, as it still is an experimental
feature. We are still not sure of the security implications of exposing some
electron renderer APIs to preload, but here are some things to consider before
running untrusted content:

- A preload script can accidentaly leak privileged APIs to untrusted code.
- Some bug in V8 engine may allow malicious code to access the renderer preload
  APIs, effectively granting full access to the system through the `remote`
  module.

Since running untrusted code in electron is still uncharted territory, the APIs
exposed to preload script are more unstable than the rest of electron APIs, and
may change without notice.
