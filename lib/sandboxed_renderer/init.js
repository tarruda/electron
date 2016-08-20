const events = require('events')

let ipcRenderer = new events.EventEmitter()

require('../renderer/api/ipc-renderer-setup')(ipcRenderer, binding)

binding.onMessage = function(...args) {
  console.log('SJKLASKDJSALJ');
  ipcRenderer.emit.apply(ipcRenderer, args)
}

IPC = ipcRenderer
