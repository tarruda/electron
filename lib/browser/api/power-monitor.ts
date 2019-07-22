'use strict'

import { createLazyInstance } from '../utils'

const { EventEmitter } = require('events')
const { createPowerMonitor, PowerMonitor } = process.electronBinding('power_monitor')
const { ipcMain } = require('electron')

// PowerMonitor is an EventEmitter.
Object.setPrototypeOf(PowerMonitor.prototype, EventEmitter.prototype)

const powerMonitor = createLazyInstance(createPowerMonitor, PowerMonitor, true)

// On Linux we need to call blockShutdown() to subscribe to shutdown event.
if (process.platform === 'linux') {
  powerMonitor.on('newListener', (event:string) => {
    if (event === 'shutdown' && powerMonitor.listenerCount('shutdown') === 0) {
      powerMonitor.blockShutdown()
    }
  })

  powerMonitor.on('removeListener', (event: string) => {
    if (event === 'shutdown' && powerMonitor.listenerCount('shutdown') === 0) {
      powerMonitor.unblockShutdown()
    }
  })
}

if (process.platform === 'win32') {
  // On Windows we need to handle shutdown event coming from renderers. See
  // shutdown_blocker_win.h for more details.
  // To prevent any race conditions where multiple renderers receive
  // QUERYENDSESSION and forward the message to us, we debounce by using `once`
  // to subscribe and reattach the handler 1 second later
  ipcMain.once('ELECTRON_BROWSER_QUERYENDSESSION', function remoteQueryEndSessionHandler (event) {
    powerMonitor.emit('shutdown', event)
    setTimeout(() => {
      ipcMain.once('ELECTRON_BROWSER_QUERYENDSESSION', remoteQueryEndSessionHandler)
    }, 1000)
  })
}

module.exports = powerMonitor
