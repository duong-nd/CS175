Computer Science 175
====================
Assignment 1
------------
### Authors
- Michael Tingley <michaeltingley@college.harvard.edu>
- Michael Traver <mtraver@college.harvard.edu>

### Files Submitted
* shaders/
 * asst1-sq-gl2.fshader: The fragment shader for the squirrel/smiley square
 * asst1-sq-gl2.vshader: The vertex shader for the squirrel/smiley square
 * asst1-tr-gl2.fshader: The fragment shader for the color/logo triangle
 * asst1-tr-gl2.vshader: The vertex shader for the color/logo triangle
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: (as provided)
* README.md: This file, in Markdown format
* asst1.cpp: The C++ file used for CPU handling of this assignment.
* glsupport.cpp: (as provided)
* glsupport.h: (as provided)
* ppm.cpp: (as provided)
* reachup.ppm: (as provided)
* shield.ppm: (as provided)
* smiley.ppm: (as provided)

### Platform
We developed on two differnet platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OSX. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst1` to compile and run the code.

### Requirements
We met all problem set requirements (as far as we know). We compared our result against the solution binaries and it behaves the same, except for a few nominal differences, such as the image moving different amounts when `i`, `j`, `k`, or `l` was pressed.

### Code Design
Most of the code design was already laid out for us. We made minor tweaks to the vertex and fragment files to implement the requirements of the problem set. We follow the style used for updating vertex colors. When designing the triangle, we tried to copy the idea of a `ShaderState`, and created one to manage the shader state of the triangle. From there, it was simple to modify the shader state and pass that information along to the GPU in uniform variables. For getting the colors, we simply had to forward on color information from the vertex phase to the fragment phase by passing along the varying variable that maintained the color information.

### Testing
Testing is straightforward from the assignment specification.
* Resize the window on one axis. Make sure that the image remains centered and the same size.
* Resize the window on another axis. The image should increase in size but maintain its aspect ratio.
* Keep increasing along this axis until the frame is larger in this dimension than in the other dimension. The image shold stop increasing in size and maintain its aspect ratio.
* Press `i`, `j`, `k`, and `l` to test that the triangle moves as expected.
* Resize the frame and move the triangle again. Make sure that the distance that the triangle moves is proportional to its aspect ratio.
* Right click and drag horizontally along the frame at various points during the above testing. Only the rectangle should change; the triangle should remain the same.
* Furthermore, it should be possible to right-click-drag and press one of the directional keys simultaneously to change the features of the two shapes at the same time.

### Above and Beyond
We did not implement anything above and beyond the assignment.
