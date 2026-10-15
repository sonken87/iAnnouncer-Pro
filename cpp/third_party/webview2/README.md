# WebView2 SDK (vendored)

From the NuGet package `Microsoft.Web.WebView2` 1.0.4258.31 (BSD-style license, see
`LICENSE.txt`): `include/WebView2.h` and `x64/WebView2Loader.dll`.

`include/EventToken.h` is a compatibility shim: mingw-w64 does not ship this Windows
SDK header, which `WebView2.h` includes.
