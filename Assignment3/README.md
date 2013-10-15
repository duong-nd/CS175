Computer Science 175
====================
Assignment 3
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasureable viewing experience!

### Authors
- Michael Tingley, <michaeltingley@college.harvard.edu>
- Michael Traver, <mtraver@college.harvard.edu>

### Files Submitted
* headers/
 * arcball.h: (as provided)
 * cvec.h: (as provided)
 * geometrymaker.h: (as provided)
 * glsupport.h: (as provided)
 * matrix4.h: (as provided)
 * ppm.h: (as provided)
 * quat.h: (as provided)
 * rigtform.h: Filled in the interface for rigid body transforms so that they support the essentially same interface as Matrix4s.
* shaders/
 * basic-gl2.vshader: (as provided)
 * diffuse-gl2.fshader: (as provided)
 * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`
* README.md: This file, in Markdown format.
* asst.cpp: The C++ file, now using RigidTForms and with arcball support.
* cvec.h: (as provided)
* glsupport.cpp: (as provided)
* ppm.cpp: (as provided)

### Platform
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OS X. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements
We met all problem set requirements. We compared our result against the solution binary and it behaves in the the same way.

### Code Design
We completed the specification for the rigid body transforms in `RigTForm.h`. We updated all of the existing code to use `RigTForm`s instead of using `Matrix4`s.

`drawStuff` handles all of the main logic for drawing the arcball. In `drawStuff`, we set the arcball scale in a global called `g_arcballScale`. We also set the center of the arcball to be the current object being manipulated, with its corresponding frame. We render the arcball. It's also worth pointing out that we call `glutPostRedisplay` in the `mouse` function in order to reset the arcball radius when the mouse is released.

Arcball rotation is implemented in the `getArcballRotation` function. The function returns the appropriate `RigTForm` for an arcball rotation from the point where the mouse was clicked to the current mouse location. `getArcballRotation` is called in the `motion` function in the cases for which we use arcball.

### Testing
Part 1: We simply ran the code with the `RigidTForm`s in it, and ensured that its behavior didn't change.

Part 2: We tested to make sure that path invariance was achieved, and that our program's behavior matched the solution binary.

Part 3: We translated an object, and made sure that the object followed the cursor closely as intended. We also moved the object toward and away from the camera, and ensured that the radius of the arcball changed screen size during movement, but reset after translation was complete.

### Above and Beyond
We did not implement anything above and beyond the assignment.
