const events = require('events')

ipcRenderer = new events.EventEmitter()

require('../renderer/api/ipc-renderer-setup')(ipcRenderer, binding)

IPC = ipcRenderer
