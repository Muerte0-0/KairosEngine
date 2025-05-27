-- Kairos Engine Dependencies --

-- Add Vulkan SDK detection
VULKAN_SDK = os.getenv("VULKAN_SDK")

Dependencies = "%{wks.location}/Engine/Dependencies"

IncludeDir = {}
IncludeDir["Vulkan"] = "%{Dependencies}/Vulkan/include"
IncludeDir["GLFW"] = "%{Dependencies}/GLFW/include"
IncludeDir["Volk"] = "%{Dependencies}/volk"
IncludeDir["ImGui"] = "%{Dependencies}/ImGui"
IncludeDir["Slang"] = "%{Dependencies}/Slang/include"
IncludeDir["Glad"] = "%{Dependencies}/Glad/include"
IncludeDir["Glm"] = "%{Dependencies}/glm"
IncludeDir["stb_image"] = "%{Dependencies}/stb_image"
IncludeDir["spdlog"] = "%{Dependencies}/spdlog/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] =  "%{Dependencies}/Vulkan/lib/vulkan-1.lib"
Library["GLFW"] =  "%{Dependencies}/GLFW/lib/glfw3.lib"
Library["Slang"] =  "%{Dependencies}/Slang/lib/slang.lib"
Library["Slang-RT"] =  "%{Dependencies}/Slang/lib/slang-rt.lib"

-- Shader C Debub Libraries
Library["ShaderC_Combined_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_combinedd.lib"

-- Shader C Release Libraries
Library["ShaderC_Combined_Release"] = "%{Dependencies}/Vulkan/lib/shaderc_combined.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"