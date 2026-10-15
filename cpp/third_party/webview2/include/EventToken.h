#pragma once
// mingw-w64 lacks this Windows SDK header, which WebView2.h includes.
typedef struct EventRegistrationToken {
    __int64 value;
} EventRegistrationToken;
