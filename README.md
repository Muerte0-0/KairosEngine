# KairosEngine
Kairos Engine is a custom game engine built with a focus on modern graphics programming using Vulkan.
This project is currently under active development.

## 🛠️ Building the Engine
### Prerequisites
Before building, make sure you have the following installed:
- C++ Compiler
	- Windows: MSVC (Visual Studio 2022 recommended)
	- Linux: GCC or Clang

- Vulkan SDK
	- Install the latest Vulkan SDK from the official [LunarG website](https://vulkan.lunarg.com)
	- Ensure environment variables are properly set (VULKAN_SDK)

- Build System
	- Premake

- GLFW Dependencies
  - Kairos Engine includes GLFW as a submodule/source dependency.
  - Refer to the [GLFW Docs](https://www.glfw.org/docs/latest/compile.html) to Install any Dependencies that GLFW Requires
 
## 📌 Notes

- Make sure your graphics drivers support Vulkan
- If you encounter missing Vulkan headers or libraries, double-check your SDK installation
- On Linux, ensure X11/Wayland development libraries are installed for GLFW
