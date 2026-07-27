# slate engine

slate engine is a cross-platform, open source, 3d, fully featured game engine built using c++ and vulkan/sdl3.

as of right now, slate engine is purely vulkan based, however, directx and metal support might be coming in the future! :0

---

# technical info:

* **indexed rendering:** for good memory efficiency while mapping triangles
* **backface culling:** because why would you render more than you need to
* **double-buffering:** basically just vsync, protects from screen tears
* **pbr material system:** realistic cook-torrance pbr material system
* **obj / mtl importing:** to import all the 3d models and their materials
* **automatic earcut triangulation:** so it can handle the most complex of n-gons

---

# future features:

* custom persistent ui system
* generic game engine stuff

---

# the stack:

* JetBrains CLion
* c++
* CMake
* vulkan
* sdl3

---

# ai disclosure:

no generative ai or LLMs were used to write or debug any of this code

---

created by **swedishsplidney** for hack club stardance 2026