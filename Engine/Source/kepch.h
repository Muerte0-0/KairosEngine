// kepch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#pragma once

#include "Engine/Core/PlatformDetection.h"

#ifdef KE_PLATFORM_WINDOWS
#ifndef NOMINMAX
	// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <utility>
#include <limits>
#include <algorithm>
#include <functional>
#include <optional>

#include <string>
#include <array>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Engine/Core/Base.h"

#include "Engine/Core/Log.h"

#include "Engine/Debug/Instrumentor.h"

#ifdef KE_PLATFORM_WINDOWS
#include <Windows.h>
#endif