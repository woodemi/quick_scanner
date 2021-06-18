#include "include/quick_scanner/quick_scanner_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Devices::Enumeration;

namespace {

class QuickScannerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  QuickScannerPlugin();

  virtual ~QuickScannerPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  DeviceWatcher deviceWatcher{ nullptr };

  winrt::event_token deviceWatcherAddedToken;
  void DeviceWatcher_Added(DeviceWatcher sender, DeviceInformation info);

  winrt::event_token deviceWatcherRemovedToken;
  void DeviceWatcher_Removed(DeviceWatcher sender, DeviceInformationUpdate infoUpdate);
};

// static
void QuickScannerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "quick_scanner",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<QuickScannerPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

QuickScannerPlugin::QuickScannerPlugin() {
  deviceWatcher = DeviceInformation::CreateWatcher(DeviceClass::ImageScanner);
  deviceWatcherAddedToken = deviceWatcher.Added({ this, &QuickScannerPlugin::DeviceWatcher_Added });
  deviceWatcherRemovedToken = deviceWatcher.Removed({ this, &QuickScannerPlugin::DeviceWatcher_Removed });
}

QuickScannerPlugin::~QuickScannerPlugin() {
  deviceWatcher.Added(deviceWatcherAddedToken);
  deviceWatcher.Removed(deviceWatcherRemovedToken);
  deviceWatcher = nullptr;
}

void QuickScannerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("startWatch") == 0) {
    deviceWatcher.Start();
    result->Success(nullptr);
  } else if (method_call.method_name().compare("stopWatch") == 0) {
    deviceWatcher.Stop();
    result->Success(nullptr);
  } else {
    result->NotImplemented();
  }
}

void QuickScannerPlugin::DeviceWatcher_Added(DeviceWatcher sender, DeviceInformation info) {
  std::cout << "DeviceWatcher_Added " << winrt::to_string(info.Id()) << std::endl;
}

void QuickScannerPlugin::DeviceWatcher_Removed(DeviceWatcher sender, DeviceInformationUpdate infoUpdate) {
  std::cout << "DeviceWatcher_Removed " << winrt::to_string(infoUpdate.Id()) << std::endl;
}

}  // namespace

void QuickScannerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  QuickScannerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
