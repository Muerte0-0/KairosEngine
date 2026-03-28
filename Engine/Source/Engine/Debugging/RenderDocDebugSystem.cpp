#include "kepch.h"
#include "RenderDocDebugSystem.h"

#if defined(PLATFORM_WINDOWS)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#elif defined(PLATFORM_LINUX)
#	include <dlfcn.h>
#endif

// Value for eRENDERDOC_API_Version_1_4_1 from RenderDoc's header to avoid including it
#ifndef RENDERDOC_API_VERSION_1_4_1
#	define RENDERDOC_API_VERSION_1_4_1 10401
#endif

// Minimal local typedefs and struct to receive function pointers without including renderdoc_app.h
using pTriggerCaptureLocal    = void (*)();
using pStartFrameCaptureLocal = void (*)(void *, void *);
using pEndFrameCaptureLocal   = unsigned int (*)(void *, void *);

struct RENDERDOC_API_1_4_1_MIN
{
	pTriggerCaptureLocal    TriggerCapture;
	void                   *_pad0;        // We don't rely on layout beyond the subset we read via memcpy
	pStartFrameCaptureLocal StartFrameCapture;
	pEndFrameCaptureLocal   EndFrameCapture;
};


namespace Engine
{
	static RenderDocDebugSystem* instance = nullptr;

	RenderDocDebugSystem& RenderDocDebugSystem::Get()
	{
		return *instance;
	}

	bool RenderDocDebugSystem::LoadRenderDocAPI()
	{
		if (renderdocAvailable)
			return true;

		// Try to fetch RENDERDOC_GetAPI from a loaded module without forcing a dependency
		pRENDERDOC_GetAPI getAPI = nullptr;

#if defined(PLATFORM_WINDOWS)
		HMODULE mod = GetModuleHandleA("renderdoc.dll");
		if (!mod)
		{
			// If not already injected/loaded, do not force-load. We can attempt LoadLibraryA as a fallback
			mod = LoadLibraryA("renderdoc.dll");
			if (!mod)
			{
				LOG(LogLevel::Info, "RenderDoc not loaded into process");
				return false;
			}
		}
		getAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(mod, "RENDERDOC_GetAPI"));
#elif defined(__APPLE__) || defined(PLATFORM_LINUX)
		void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
		if (!mod)
		{
			// Try to load if not already loaded; if unavailable, just no-op
			mod = dlopen("librenderdoc.so", RTLD_NOW);
			if (!mod)
			{
				LOG_INFO("RenderDoc", "RenderDoc not loaded into process");
				return false;
			}
		}
		getAPI = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(mod, "RENDERDOC_GetAPI"));
#endif

		if (!getAPI)
		{
			LOG(LogLevel::Warning, "RENDERDOC_GetAPI symbol not found");
			return false;
		}

		// Request API 1.4.1 into a temporary buffer and then extract needed functions
		RENDERDOC_API_1_4_1_MIN apiMin{};
		void                   *apiPtr = nullptr;
		int                     result = getAPI(RENDERDOC_API_VERSION_1_4_1, &apiPtr);
		if (result == 0 || apiPtr == nullptr)
		{
			LOG(LogLevel::Warning, "Failed to acquire RenderDoc API 1.4.1");
			return false;
		}

		// Copy only the subset we care about; layout is stable for these early members
		std::memcpy(&apiMin, apiPtr, sizeof(apiMin));

		fnTriggerCapture    = apiMin.TriggerCapture;
		fnStartFrameCapture = apiMin.StartFrameCapture;
		fnEndFrameCapture   = apiMin.EndFrameCapture;

		renderdocAvailable = (fnTriggerCapture || fnStartFrameCapture || fnEndFrameCapture);

		if (renderdocAvailable)
		{
			LOG(LogLevel::Info, "RenderDoc API loaded");
		}
		else
		{
			LOG(LogLevel::Warning, "RenderDoc API did not provide expected functions");
		}
		
		return renderdocAvailable;
	}

	void RenderDocDebugSystem::TriggerCapture()
	{
		if (!renderdocAvailable && !LoadRenderDocAPI())
			return;
		if (fnTriggerCapture)
		{
			fnTriggerCapture();
			LOG(LogLevel::Info, "Triggered capture");
		}
		else
		{
			LOG(LogLevel::Warning, "TriggerCapture not available");
		}
	}

	void RenderDocDebugSystem::StartFrameCapture(void* device, void* window)
	{
		if (!renderdocAvailable && !LoadRenderDocAPI())
			return;
		if (fnStartFrameCapture)
		{
			fnStartFrameCapture(device, window);
			LOG(LogLevel::Trace, "StartFrameCapture called");
		}
		else
		{
			LOG(LogLevel::Warning, "StartFrameCapture not available");
		}
	}

	bool RenderDocDebugSystem::EndFrameCapture(void* device, void* window)
	{
		if (!renderdocAvailable && !LoadRenderDocAPI())
			return false;
		
		if (fnEndFrameCapture)
		{
			unsigned int ok = fnEndFrameCapture(device, window);
			LOG(LogLevel::Trace, "EndFrameCapture: {}", ok ? "success" : "failure");
			return ok != 0;
		}
		
		LOG(LogLevel::Warning, "EndFrameCapture not available");
		
		return false;
	}
}
