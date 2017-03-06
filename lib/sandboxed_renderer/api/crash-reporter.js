const {remote} = require('electron')
const binding = process.atomBinding('crash_reporter')

class CrashReporter {
  start (options) {
    let [productName, companyName, submitURL, crashesDirectory, uploadToServer,
      ignoreSystemCrashHandler, extra] = remote.crashReporter.getBindingStartArgs(options)

    remote.crashReporter.ensureInternalCrashService(submitURL, productName, crashesDirectory)

    binding.start(productName, companyName, submitURL, crashesDirectory,
      uploadToServer, ignoreSystemCrashHandler, extra)
  }
}

module.exports = new CrashReporter()
