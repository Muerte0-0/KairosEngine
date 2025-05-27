#include "kepch.h"
#include "Engine/Core/Window.h"

#ifdef KE_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

#ifdef KE_PLATFORM_LINUX
//#include "Platform/Linux/LinuxWindow.h"
#endif

#ifdef KE_PLATFORM_MACOS
//#include "Platform/MacOS/MacOSWindow.h"
#endif

namespace Kairos
{
	Window* Window::Create(const WindowProps& props)
	{
	#ifdef KE_PLATFORM_WINDOWS
		return new WindowsWindow(props);
	#elif KE_PLATFORM_LINUX
		KE_CORE_ASSERT(false, "Linux Platform Not Yet Supported!");
		return nullptr;
	#elif KE_PLATFORM_MACOS
		KE_CORE_ASSERT(false, "MACOS Platform Not Yet Supported!");
		return nullptr;
	#else
		KE_CORE_ASSERT(false, "Unknown Platform!");
		return nullptr;
	#endif
	}
}