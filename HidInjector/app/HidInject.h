#pragma once

#include "common.h"

UCHAR VKeyToKeyboardUsage(UCHAR vk);
UCHAR ScanCodeToKeyboardUsage(UCHAR code);
UCHAR UnicodeToKeyboardUsage(WCHAR ch);

// No usage is defined for this VKEY
#define USAGE_NONE 0

// A usage may be defined, but we haven't bothered to find it yet.  
#define USAGE_TODO 0

BOOL SetKeybaordUsage(HIDINJECTOR_INPUT_REPORT *Rep, UCHAR Usage);

BOOL ClearKeyboardUsage(HIDINJECTOR_INPUT_REPORT *Rep, UCHAR Usage);
