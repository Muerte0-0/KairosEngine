// kepch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.
#pragma once

#include "Engine/Core/PlatformDetection.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <utility>
#include <assert.h>
#include <limits>
#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
#include <thread>

#include <string>
#include <array>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#include "Engine/Core/Base.h"