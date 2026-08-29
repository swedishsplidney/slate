```text
                                                 ppppppppppppppp           
                ZZZZZZ                  ZZZ     pppppppppppppppp           
                    ZZ                  ZZZ    pppppppppppppppp            
       ZZZZZZZZ     ZZ       ZZZZZZY ZZZZZZZZZZppw      pppppp             
       ZZ    OOO    ZZ      ZZ    ZZ,   ZZZ   ppf  pppp  pppp|             
       ZZZZZZZ      ZZ       ZZZZZZZ3   ZZZ  ppp         Oppp              
             ZZZ    ZZ     ZZZ    ZZ3   ZZZ  ppp   ppppppppp               
       ZZ    ZZZ    ZZZ    ZZZ   XZZ3   ZZZ ppppp  pppp  pp                
        ZZZZZZQ      XZZZZZ WZZZZ ZZ3     Z    ppp      ppp                
                                          :ppppppppppppppp                      
                                                          _          
                                            ___ _ _  __ _(_)_ _  ___ 
                                           / -_) ' \/ _` | | ' \/ -_)
                                           \___|_||_\__, |_|_||_\___|
                                                    |___/  
```

# slate engine

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

* hierarchy panel
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



