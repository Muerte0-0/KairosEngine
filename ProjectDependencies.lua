-- Kairos Engine Dependencies --

-- Vulkan SDK detection
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Check For Vulkan SDK
function CheckVulkanSDK()
    if not VULKAN_SDK then
        print("\n==================================================")
        print("			VULKAN SDK NOT FOUND!")
        print("==================================================")
        print("To fix this:")
        print("1. Download Vulkan SDK from: https://vulkan.lunarg.com/")
        print("2. Install it")
        print("3. Set the VULKAN_SDK environment variable:")
        
        if os.target() == "windows" then
            print("   - On Windows: setx VULKAN_SDK \"C:\\VulkanSDK\\[version]\"")
            print("   - Or set it in System Properties -> Environment Variables")
        elseif os.target() == "linux" then
            print("   - On Linux: export VULKAN_SDK=/path/to/vulkan/sdk")
            print("   - Add to ~/.bashrc or ~/.zshrc")
        elseif os.target() == "macosx" then
            print("   - On macOS: export VULKAN_SDK=/path/to/vulkan/sdk")
            print("   - Add to ~/.zshrc or ~/.bash_profile")
        end
        
        print("4. Restart your terminal/IDE")
        print("==================================================\n")
        return false
    end
    
    -- Check if directory exists
    if not os.isdir(VULKAN_SDK) then
        print("VULKAN_SDK points to non-existent directory: " .. VULKAN_SDK)
        return false
    end
    
    return true
end

-- Perform the check
if not CheckVulkanSDK() then
    error("Build aborted due to missing Vulkan SDK dependencies.")
end

print("\n======================================================================")
print("			VULKAN SDK FOUND!")
print("\n	VULKAN SDK Directory: " .. VULKAN_SDK)
print("======================================================================")

ThirdParty = "%{wks.location}/Vendor"

IncludeDir = {}
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/include"
IncludeDir["GLFW"] =  "%{ThirdParty}/GLFW/include"
IncludeDir["ImGui"] = "%{ThirdParty}/ocornut/ImGui"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"