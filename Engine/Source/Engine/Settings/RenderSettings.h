#pragma once

#include "Engine/Settings/SettingsRegistry.h"

namespace Engine
{
	class RenderSettings : public ISettings
	{
	public:
		bool HDR = true;
		float Exposure = 1.0f;

		const char* GetFileName() const override { return "Render"; }
		const char* GetTypeName() const override { return "RenderSettings"; }

		SettingsMetadata GetMetadata() const override
		{
			return { SettingsCategories::Engine, "Rendering", 0, false };
		}

		void Serialize(YAML::Emitter& out) const override;
		void Deserialize(const YAML::Node& node) override;
		void DrawUI() override;
	};
}
