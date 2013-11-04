Computer Science 175
====================
Assignment 6
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasureable viewing experience!

### Authors ###
- Michael Tingley, < michaeltingley@college.harvard.edu >
- Michael Traver, < mtraver@college.harvard.edu >

### Files Submitted ###
* headers/
  * arcball.h: (as provided)
  * asstcommon.h: (as provided)
  * cvec.h: (as provided)
  * drawer.h: (as provided)
  * frame.h: Tweaks to make Catmull-Rom interpolation work correctly.
  * geometry.h: (as provided)
  * geometrymaker.h: (as provided)
  * glsupport.h: (as provided)
  * matrix4.h: (as provided)
  * picker.h: (as provided)
  * ppm.h: (as provided)
  * quat.h: (as provided)
  * renderstates.h: (as provided)
  * rigtform.h: Modified to make Catmull-Rom interpolation work.
  * scenegraph.h: (as provided)
  * script.h: Tweaks to make BezCatmull-Rom interpolation work.
  * sgutils.h: (as provided)
  * texture.h: (as provided)
  * uniforms.h: (as provided)
  * utils.h: (as provided)
* shaders/: (as provided)
  * basic-gl2.vshader: (as provided)
  * diffuse-gl2.fshader: (as provided)
  * normal-gl2.fshader: Modified to work with texture-mapped surfaces.
  * normal-gl2.vshader: (as provided)
  * pick-gl2.fshader (as provided)
  * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* Fieldstone.ppm
* FieldstoneNormal.ppm
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`.
* README.md: This file, in Markdown format.
* asst.cpp: The C++ file, now responding to frame update requests and with animation support.
* geometry.cpp: (as provided)
* glsupport.cpp: (as provided)
* material.h: (as provided)
* picker.cpp: (as provided)
* ppm.cpp: (as provided)
* renderstates.cpp: (as provided)
* scenegraph.cpp: (as provided)
* texture.cpp: (as provided)

### Platform ###
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OS X. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running ###
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements ###
We met all problem set requirements. We compared our result against the solution binary and it behaves in the the same way.

### Code Design ###
** Assignment 6. ** This required a few tweaks to how we were doing interpolation in `script.h` and `frame.h`. We now pass in four consecutive frames. In `frame.h`, we use the calculations (as explained in the textbook) necessary to achieve smooth Catmull-Rom interpolation. We also created a new function `controlPoint` in `RigTForm.h` to calculate the control points used at each stage in Catmull-Rom interpolation.

** Assignment 6.5. ** Code migration was straightforward and will not be discussed.

< TODO: TRAVER'S PART >

Writing the GLSL was easy once we figured out what we needed to do. We made a few small tweaks to the `normal-gl2.fshader` to use the appropriate normal, as explained in the problem set. It involves basic manipulation of the normal texture values, and the code can be seen for specifics.

### Testing ###
Testing for this assignment was very straightforward.

** Assignment 6. **  To test this, we simply confirmed that the animation was smooth. We compared with the solution binary to ensure that the smoothness was holisitically 'similar'.

** Assignment 6.5. **  To test that the lights were moveable, we simply checked visually that they had spheres around them, and that we could move them as we could with all the other objects.

To test that the GLSL was correct, we moved the camera and lights around. We checked that the shinyness changed as expected whenlights moved closer and farther away, and when we changed our orientation with respect to the lights. We also moved the robots around to make sure that we didn't mess anything else up.

### Above and Beyond ###
We did not implement anything above and beyond the assignment.
