#pragma once

namespace Kairos
{
	class Panel
	{
	public:
		virtual ~Panel() = default;
		virtual void OnImGuiRender() = 0;
	};
}
