# KairosEngine
Kairos Engine is a custom game engine built with a focus on modern graphics programming using Vulkan.
This project is currently under active development.

## 🚧 Status
Early development. Things may break. That’s part of the adventure.

## 📌 Notes

- Make sure your graphics drivers support Vulkan
- If you encounter missing Vulkan headers or libraries, double-check your SDK installation
- On Linux, ensure X11/Wayland development libraries are installed for GLFW

## ⚙️ Generating Project Files
Before building, you must generate the project files using the provided scripts.

Navigate to the Scripts folder in Terminal and Run:

### 🪟 Windows
	.\GenerateProjectFiles.bat

### 🐧 Linux
	chmod +x GenerateProjectFiles.sh
	./GenerateProjectFiles.sh

## 🛠️ Building the Engine
### Prerequisites
Before building, make sure you have the following installed:
- C++ Compiler
	- Windows: MSVC (Visual Studio 2022 recommended)
	- Linux: GCC or Clang

- Vulkan SDK
	- Install the latest Vulkan SDK from the official [LunarG website](https://vulkan.lunarg.com).
	- Ensure environment variables are properly set (VULKAN_SDK).

- GLFW Dependencies
  - Kairos Engine includes GLFW as a submodule/source dependency and Builds it along the Engine.
  - Refer to the [GLFW Docs](https://www.glfw.org/docs/latest/compile.html) to Install any Dependencies that GLFW Requires.
