# Spectrum Analyzer

**Seven Phases Spectrum Analyzer** is a real-time spectrum analysis VST plugin. The idea was to create an old-fashioned tool reminiscent of classic hardware analyzers of the earlier centuries. Unlike most of other analyzers available these days, this plugin is not FFT-based but utilizes a filter-bank algorithms that mould its unique pros and cons.

See [plugin page](http://kvraudio.com/product/spectrum-analyzer-by-seven-phases#kvrphd) at [**KVR Audio**](http://www.kvraudio.com/).

Looking for release downloads? Find them [here](https://github.com/seven-phases/spectrum-analyzer/releases).

## Build Instructions

Required tools:

* **WDK 7.1**: https://microsoft.com/download/details.aspx?id=11800
* **VST SDK**: http://www.steinberg.net/en/company/developers.html

Building:

1. Install WDK and VST SDK.
2. Specify their paths in `build/wdk71/paths.cmd` file (run `build-release.cmd` to create this file for you). For example:
```
set WDKROOT=E:\WDK
set VSTSDKROOT=C:\Foo\Steinberg\VST3SDK
```
3. Run `build/wdk71/build-release.cmd` script.
4. On a successful build (run the script via `cmd` console to see errors and warnings) the compiled binaries will appear in the `release` directory (`x86` and `x64` respectively).

## More info
Work in progress...
