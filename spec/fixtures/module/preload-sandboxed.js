(function () {
  if (location.protocol === 'file:') {
    window.test = 'preload'
    window.require = require
    window.process = process
  } else if (location.href !== 'about:blank') {
    let {ipcRenderer} = require('electron')
    addEventListener('DOMContentLoaded', () => {
      ipcRenderer.send('child-loaded', window.opener == null, document.body.innerHTML)
    }, false)
  }
})()
