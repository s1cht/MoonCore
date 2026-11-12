#pragma once
#include "MoonCore/Types.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MCAN
#define NOCLIPBOARD
#define far
#define FAR
#define near
#define NEAR
#undef TEXT

#include <windows.h>
#include <windowsx.h>
#include <Objbase.h>
#include <Shlwapi.h>
#include <winbase.h>
#include <KnownFolders.h>
#include <Shlobj.h>

#pragma comment(lib, "OneCore.lib")

#undef far
#undef FAR
#undef near
#undef NEAR
#undef Yield
#undef CreateDirectory
#undef RemoveDirectory
#undef SetCurrentDirectory
#undef GetCurrentDirectory
#undef IsMaximized
#undef IsMinimized
#undef FormatMessage
#undef TEXT

#define TEXT(x) TEXT_HELPER(x)