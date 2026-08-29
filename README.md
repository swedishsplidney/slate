# Slate Engine

<p align="center">
  <a href="https://">
    <img src="assets/slate engine logo darkbg.svg" width="400" alt="slate engine logo">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/c%2B%2B_20-%2300599C.svg?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++20">
  <img src="https://img.shields.io/badge/Vulkan-%23AC162C.svg?style=for-the-badge&logo=vulkan&logoColor=white" alt="Vulkan">
  <img src="https://img.shields.io/badge/SDL3-%2314385C.svg?style=for-the-badge&logo=sdl&logoColor=white" alt="SDL3">
  <img src="https://img.shields.io/badge/GLM-%23FFB13B.svg?style=for-the-badge" alt="GLM">
  <img src="https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake">
  <img src="https://img.shields.io/badge/GPL--3.0-red?style=for-the-badge" alt="GPLv3">
  <br>
  <img src="https://img.shields.io/badge/Linux-%23FCC624.svg?style=for-the-badge&logo=linux&logoColor=black" alt="Linux">
  <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows">
  <img src="https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=apple&logoColor=white" alt="macOS">
</p>

**Slate Engine** is an open source, cross-platform, fully featured game engine to create 2d and 3d games from a single interface. It is currently very, very early in development, but i'm actively working on it!

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

# building from source:

no premade binaries are available yet, but you can easily build it from source:

### prerequisites:

* CMake 3.22 or higher
* C++20 compatible compiler
* Vulkan SDK
* SDL3 dev libraries
* GLM

### build and run:

to build it, just run the following commands in your terminal of choice:

```bash
git clone https://github.com/swedishsplidney/slate

cd slate

mkdir build && cd build

cmake ..

cmake --build .
```

then you can run it:

```bash
./slate
```

---

# built using:

* C++ 20
* Vulkan 1.3
* SDL3
* CMake
* JetBrains CLion / Neovim

---

# ai disclosure:

no generative ai or LLMs were used to write or debug any of this code

---

# license:

slate engine is open source, licensed under the GPLv3 license

see the `LICENSE` file for details

---

created by **swedishsplidney**