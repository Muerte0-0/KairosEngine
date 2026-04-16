#include "kepch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace Engine
{
	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& projectPath)
	{
		Ref<Project> project = CreateRef<Project>();
		
		ProjectSerializer serializer(project);
		
		if (serializer.Deserialize(projectPath))
		{
			project->m_ProjectDirectory = projectPath.parent_path();
			s_ActiveProject = project;
			return s_ActiveProject;
		}
		
		return nullptr;
	}
		
	bool Project::Save(const std::filesystem::path& projectPath)
	{
		ProjectSerializer serializer(s_ActiveProject);
		
		if (serializer.Serialize(projectPath))
		{
			s_ActiveProject->m_ProjectDirectory = projectPath.parent_path();
			return true;
		}
		
		return false;
	}
}
