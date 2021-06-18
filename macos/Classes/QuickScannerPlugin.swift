import Cocoa
import FlutterMacOS
import ImageCaptureCore

public class QuickScannerPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "quick_scanner", binaryMessenger: registrar.messenger)
    let instance = QuickScannerPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  private var deviceBrowser: ICDeviceBrowser!
    
  override public init() {
    super.init()
    deviceBrowser = ICDeviceBrowser()
    deviceBrowser.delegate = self
    let mask = ICDeviceTypeMask(rawValue: ICDeviceTypeMask.scanner.rawValue | ICDeviceLocationTypeMask.local.rawValue)
    deviceBrowser.browsedDeviceTypeMask = mask!
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "getPlatformVersion":
      result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
    case "startWatch":
      deviceBrowser.start()
      result(nil)
    case "stopWatch":
      deviceBrowser.stop()
      result(nil)
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}

extension QuickScannerPlugin: ICDeviceBrowserDelegate {
  public func deviceBrowser(_ browser: ICDeviceBrowser, didAdd device: ICDevice, moreComing: Bool) {
    print("deviceBrowser:\(browser) didAdd:\(device.name) moreComing:\(moreComing)")
  }
    
  public func deviceBrowser(_ browser: ICDeviceBrowser, didRemove device: ICDevice, moreGoing: Bool) {
    print("deviceBrowser:\(browser) didRemove:\(device.name) moreGoing:\(moreGoing)")
  }
}
