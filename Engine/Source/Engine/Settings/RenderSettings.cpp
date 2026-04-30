#include "kepch.h"
#include "RenderSettings.h"

#include "imgui.h"

namespace Engine
{
	void RenderSettings::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "HDR" << YAML::Value << HDR;
		out << YAML::Key << "Exposure" << YAML::Value << Exposure;
	}

	void RenderSettings::Deserialize(const YAML::Node& node)
	{
		HDR = node["HDR"] ? node["HDR"].as<bool>() : HDR;
		Exposure = node["Exposure"] ? node["Exposure"].as<float>() : Exposure;
	}

	void RenderSettings::DrawUI()
	{
		ImGui::Checkbox("HDR", &HDR);
		ImGui::DragFloat("Exposure", &Exposure, 0.1f, 0.0f, 10.0f);
	}
}
