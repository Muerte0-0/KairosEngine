#pragma once
#include "Project.h"

namespace Engine
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(Ref<Project> project);
		
		bool Serialize(const std::filesystem::path& filepath) const;
		bool Deserialize(const std::filesystem::path& filepath);
		
	private:
		Ref<Project> m_Project;
	};
}
