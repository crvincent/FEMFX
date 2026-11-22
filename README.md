AMD FEMFX
v0.1.0

## Licensing

FEMFX, its bundled components, and the Google Highway submodule are governed
by separate licenses:

| Component | License | Location |
| --- | --- | --- |
| FEMFX | MIT | `LICENSE.txt` |
| Sony Vectormath and other bundled third-party components | See component notice | `NOTICES.txt` |
| Google Highway submodule | Apache-2.0 or BSD-3-Clause, at the user's choice | `highway/LICENSE` |

Linking FEMFX into an application does not remove the redistribution
requirements of any component included in the resulting build. Preserve
`LICENSE.txt`, `NOTICES.txt`, the Highway license, and any component-specific
headers in source or binary distributions as required by the applicable
licenses.

FEMFX is a multithreaded CPU library for deformable material physics, using 
the Finite Element Method (FEM).  Solid objects are represented as a mesh of 
tetrahedral elements, and each element has material parameters that control 
stiffness, how volume changes with deformation, and stress limits where 
fracture or plastic (permanent) deformation occur.  The model supports a 
wide range of materials and interactions between materials.  We intend for 
these features to complement rather than replace traditional rigid body 
physics.  The system is designed with the following considerations:

 * Fidelity: realistic-looking wood, metal, plastic, even glass, because 
   they bend and break according to stress as real materials do.
 * Deformation effects: non-rigid use cases such as soft-body objects, 
   bending or warping objects.   It is not just a visual effect, but 
   materials will resist or push back on other objects.
 * Changing material on the fly: you can change the settings to make the 
   same object behave very differently, e.g., turn gelatinous or melt
 * Interesting physics interactions for gameplay or puzzles

The library uses extensive multithreading to utilize multicore CPUs and benefit 
from the trend of increasing CPU core counts.

System requirements
* AMD Ryzen™ 7 2700X Processor or equivalent
* Windows® 10
* Visual Studio 2017 or Visual Studio 2019