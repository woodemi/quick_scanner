# quick_scanner

A cross-platform (Windows/macOS) scanner plugin for Flutter

## Usage

```
QuickScanner.startWatch();

var _scanners = await QuickScanner.getScanners();
var directory = await getApplicationDocumentsDirectory();
var scannedFile = await QuickScanner.scanFile(_scanners.first, directory.path);

QuickScanner.stopWatch();
```

