#include "include/quick_scanner/quick_scanner_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Scanners.h>
#include <winrt/Windows.Storage.h>

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
using namespace Windows::Devices::Scanners;
using namespace Windows::Storage;

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

  std::vector<std::string> scanners_{};

  winrt::fire_and_forget ScanFileAsync(std::string device_id, std::string directory,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result
  );
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
  } else if (method_call.method_name().compare("getScanners") == 0) {
    flutter::EncodableList list{};
    for (auto scanner : scanners_) {
      list.push_back(flutter::EncodableValue(scanner));
    }
    result->Success(list);
  } else if (method_call.method_name().compare("scanFile") == 0) {
    auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
    auto device_id = std::get<std::string>(args[flutter::EncodableValue("deviceId")]);
    auto directory = std::get<std::string>(args[flutter::EncodableValue("directory")]);
    ScanFileAsync(device_id, directory, std::move(result));
    //result->Success(nullptr);
  } else {
    result->NotImplemented();
  }
}

void QuickScannerPlugin::DeviceWatcher_Added(DeviceWatcher sender, DeviceInformation info) {
  std::cout << "DeviceWatcher_Added " << winrt::to_string(info.Id()) << std::endl;

  auto device_id = winrt::to_string(info.Id());
  auto it = std::find(scanners_.begin(), scanners_.end(), device_id);
  if (it == scanners_.end()) {
    scanners_.push_back(device_id);
  }
}

void QuickScannerPlugin::DeviceWatcher_Removed(DeviceWatcher sender, DeviceInformationUpdate infoUpdate) {
  std::cout << "DeviceWatcher_Removed " << winrt::to_string(infoUpdate.Id()) << std::endl;

  auto device_id = winrt::to_string(infoUpdate.Id());
  auto it = std::find(scanners_.begin(), scanners_.end(), device_id);
  if (it != scanners_.end()) {
    scanners_.erase(it);
  }
}

winrt::fire_and_forget QuickScannerPlugin::ScanFileAsync(std::string device_id, std::string directory,
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  auto scanner = co_await ImageScanner::FromIdAsync(winrt::to_hstring(device_id));

  if (scanner.IsScanSourceSupported(ImageScannerScanSource::Feeder)) {
    // TODO
  }

  auto flatbedConfiguration = scanner.FlatbedConfiguration();
  auto supportGrayscale = flatbedConfiguration.IsColorModeSupported(ImageScannerColorMode::Grayscale);
  auto supportColor = flatbedConfiguration.IsColorModeSupported(ImageScannerColorMode::Color);
  if (!supportGrayscale && !supportColor) {
    flatbedConfiguration.ColorMode(ImageScannerColorMode::Monochrome);
  } else {
    // TODO other mode
    flatbedConfiguration.ColorMode(ImageScannerColorMode::Color);
  }

  try {
    auto storageFolder = co_await StorageFolder::GetFolderFromPathAsync(winrt::to_hstring(directory));
    auto scanResult = co_await scanner.ScanFilesToFolderAsync(ImageScannerScanSource::Flatbed, storageFolder);
    auto path = scanResult.ScannedFiles().First().Current().Path();
    result->Success(flutter::EncodableValue(winrt::to_string(path)));
  } catch (winrt::hresult_error const& ex) {
    result->Error(std::to_string(ex.code()), winrt::to_string(ex.message()));
  }
}

}  // namespace

void QuickScannerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  QuickScannerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
