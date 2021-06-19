
import 'dart:async';

import 'package:flutter/services.dart';

class QuickScanner {
  static const MethodChannel _channel =
      const MethodChannel('quick_scanner');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Future<void> startWatch() => _channel.invokeMethod('startWatch');

  static Future<void> stopWatch() => _channel.invokeMethod('stopWatch');

  static Future<List<String>> getScanners() async {
    List list = await _channel.invokeMethod('getScanners');
    return list.cast();
  }
}
