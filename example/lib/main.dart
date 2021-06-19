import 'package:flutter/material.dart';

import 'package:quick_scanner/quick_scanner.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
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
                    var scanners = await QuickScanner.getScanners();
                    scanners.forEach(print);
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
