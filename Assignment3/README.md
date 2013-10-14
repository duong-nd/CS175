Computer Science 175
====================
Assignment 3
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasureable viewing experience!

### Authors
- Michael Tingley <michaeltingley@college.harvard.edu>
- Michael Traver <mtraver@college.harvard.edu>

### Files Submitted
* headers/
 * arcball.h: (as provided)
 * cvec.h: (as provided)
 * geometrymaker.h: (as provided)
 * glsupport.h: (as provided)
 * matrix4.h: (as provided)
 * ppm.h: (as provided)
 * quat.h: (as provided)
 * rigtform.h: Filled in the interface for Rigid Body Transforms so that they support the essentially same interface as Matrix4s.
* shaders/
 * basic-gl2.vshader: (as provided)
 * diffuse-gl2.fshader: (as provided)
 * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`
* README.md: This file, in Markdown format.
* asst2.cpp: The C++ file, now using RigidTForms and with arcball support.
* cvec.h: (as provided)
* glsupport.cpp: (as provided)
* ppm.cpp: (as provided)

### Platform
We developed on two differnet platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OSX. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements
We met most of the problem set requirements. In particular, we completed all of part 1 and all of part 3. For part 2, the arcball renders correctly and with the correct orientation, and we compute the screen coordinates for where the ball is being handled and create vectors for them. When the user rotates using the arcball in this manner, the rotation is path-invariant. However, the rotation seems to behave strangely: when rotating the ball, it doesn't rotate in the intuitive manner that we would expect, and instead rotates at an angle. This is probably because of some bad math or because at some point we set the wrong respect frame to rotate around.

### Code Design
We completed the specification for the Rigid Body Transforms in `RigTForm.h`. We updated all of the existing code to use `RigTForm`s instead of using `Matrix4`s.

`drawStuff` handles all of the main logic for drawing the arcball. In `drawStuff`, we set the arcball scale in a global called `g_arcballScale`. We also set the center of the arcball to be the current object being manipulated, with its corresponding frame. We render the arcball. It's also worth pointing out now that we call `glutPostRedisplay` in the `mouse` function in order to reset the arcball radius when the mouse is released.

To handle rotation, we do vector algebra in the `motion` function. We compute `v_1` and `v_2` according to the formulae provided in class using coordinates from `getScreenSpaceCoord`. This allows us to correctly determine and apply the new transform to the object being manipulated.

### Testing
To test Part 1, we simply ran the code with the `RigidTForm`s in it.

Testing Part 3 was also easy. We translated an object, and made sure that the object followed the cursor closely as intended. We also moved the object toward and away from the camera, and ensured that the radius changed screen size during movement, but reset after translation was complete.

For testing Part 2, we ensured that the arcball followed orientation of the object that it was surrounding. We tried rotating the object to ensure that path invariance was achieved. If our implementation was better, we also would have tried rotating the object to determine if the actual rotation was carried out at approximately twice the speed of the mouse movement.

### Above and Beyond
We did not implement anything above and beyond the assignment.
