#pragma once

namespace Engine
{
	enum class TextureFormat
	{
		Undefined = 0,

		// Color formats (Linear)
		RGBA8_UNorm,
		BGRA8_UNorm,

		// Color formats (sRGB)
		RGBA8_SRGB,
		BGRA8_SRGB,

		RGBA16_Float,
		RGBA32_Float,
		 
		// Depth formats
		Depth24Stencil8,
		Depth32_Float,

		// Optional extras (nice to have)
		R8_UNorm,
		RG8_UNorm
	};
	
	enum class SampleCountBits
	{
		s1 = 1,
		s2 = 2,
		s4 = 4,
		s8 = 8,
		s16 = 16
	};
}

