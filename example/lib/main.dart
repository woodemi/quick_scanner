import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';

import 'package:quick_scanner/quick_scanner.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  List<String> _scanners = [];

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Column(
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  child: Text('startWatch'),
                  onPressed: () async {
                    QuickScanner.startWatch();
                  },
                ),
                ElevatedButton(
                  child: Text('stopWatch'),
                  onPressed: () async {
                    QuickScanner.stopWatch();
                  },
                ),
              ],
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  child: Text('getScanners'),
                  onPressed: () async {
                    var list = await QuickScanner.getScanners();
                    _scanners.addAll(list);
                    _scanners.forEach(print);
                  },
                ),
              ],
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  child: Text('scan'),
                  onPressed: () async {
                    var directory = await getApplicationDocumentsDirectory();
                    var scannedFile = await QuickScanner.scanFile(_scanners.first, directory.path);
                    print('scannedFile $scannedFile');
                  },
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}
