# slate engine

![C++](https://img.shields.io/badge/c%2B%2B-%2300599C.svg?style=for-the-badge&logo=cplusplus&logoColor=white)
![Vulkan API](https://img.shields.io/badge/Vulkan-%23AC162C.svg?style=for-the-badge&logo=vulkan&logoColor=white&logoSize=auto)
![SDL3](https://img.shields.io/badge/SDL3-%2314385C.svg?style=for-the-badge&logo=sdl&logoColor=white)
![GLM](https://img.shields.io/badge/GLM-Mathematics-%23FFB13B.svg?style=for-the-badge)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-%23FCC624.svg?style=for-the-badge&logo=linux&logoColor=black)
![windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![macos](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=apple&logoColor=white)

slate engine is a cross-platform, open source, 3d, fully featured game engine built using c++ and vulkan/sdl3.

---

# features:

* obj importing
* fully customizeable .json material system (with support for importing .mtl files)
* transform and rotation gizmos
* custom ui inspector and hierarchy panel to modify object properties

---

# technical info:

* **indexed rendering:** for good memory efficiency while mapping triangles
* **backface culling:** because why would you render more than you need to
* **double-buffering:** basically just vsync, protects from screen tears
* **pbr material system:** realistic cook-torrance pbr material system with a semi-accurate refraction approximator
* **obj / mtl importing:** to import all the 3d models and their materials
* **automatic earcut triangulation:** so it can handle the most complex of n-gons
* **fully custom persistent ui system:** designed from scratch for efficiency and functionality

---

# future features:

* scene saving
* physics
* scriptable objects

---

# the stack:

* JetBrains CLion / Neovim
* c++
* CMake
* vulkan
* sdl3

---

# ai disclosure:

no generative ai or LLMs were used to write or debug any of this code

---

created by **swedishsplidney** for hack club stardance 2026