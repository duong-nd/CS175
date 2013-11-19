Computer Science 175
====================
Assignment 8
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasurable viewing experience!

### Authors ###
- Michael Tingley, < michaeltingley@college.harvard.edu >
- Michael Traver, < mtraver@college.harvard.edu >

### Files Submitted ###
* headers/
  * arcball.h: (as provided)
  * asstcommon.h: (as provided)
  * bunny.h: Our custom-baked bunny header file that contains most of the interesting bunny calculations used in this assignment.
  * cvec.h: (as provided)
  * debug.h: Introduced to print Cvec3s and RigTForms. This isn't used anywhere in production code, but we left it here since it may prove useful in the final project.
  * drawer.h: (as provided)
  * frame.h: (as provided)
  * geometry.h: (as provided)
  * geometrymaker.h: (as provided)
  * global.h: Contains the global definition for the world. We introduced this so that we could use `g_world` in the bunny header file.
  * glsupport.h: (as provided)
  * material.h: (as provided)
  * matrix4.h: (as provided)
  * mesh.h: (as provided)
  * picker.h: (as provided)
  * ppm.h: (as provided)
  * quat.h: (as provided)
  * renderstates.h: (as provided)
  * rigtform.h: Expaned to also allow for multiplication with Cvec3s.
  * scenegraph.h: (as provided)
  * script.h: (as provided)
  * sgutils.h: (as provided)
  * texture.h: (as provided)
  * uniforms.h: (as provided)
  * utils.h: (as provided)
* shaders/
  * basic-gl2.vshader: (as provided)
  * bunny-gl2.fshader: (as provided)
  * bunny-shell-gl2.fshader: (as provided)
  * bunny-shell-gl2.vshader: (as provided)
  * diffuse-gl2.fshader: (as provided)
  * normal-gl2.fshader: (as provided)
  * normal-gl2.vshader: (as provided)
  * pick-gl2.fshader (as provided)
  * solid-gl2.fshader: (as provided)
  * specular-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* Fieldstone.ppm: (as provided)
* FieldstoneNormal.ppm: (as provided)
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`.
* README.md: This file, in Markdown format.
* asst.cpp: The main C++ file, now with a furry bunny.
* bunny.mesh: (as provided)
* cube.mesh: (as provided)
* geometry.cpp: (as provided)
* glsupport.cpp: (as provided)
* material.cpp: (as provided)
* mesh.interface: (as provided)
* picker.cpp: (as provided)
* ppm.cpp: (as provided)
* renderstates.cpp: (as provided)
* scenegraph.cpp: (as provided)
* shell.ppm: (as provided)
* texture.cpp: (as provided)

### Platform ###
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OS X. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running ###
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements ###
We met all problem set requirements. We compared our result against the solution binary and it behaves in the the same way.

### Code Design ###
Most of the interesting code was implemented in `bunny.h`. We did a few initialization things in `asst.cpp`, including:
* Initializing the bunny geometry and vertices
* Initializing the bunny physics
* Initializing the bunny simulation
* Initializing the bunny scene graph node
* Initializing the bunny material

The rest was done in `bunny.h`. This file contains all of our configurable global constants related to the bunny simulation. The design is centered around these functions, so I'll describe the program design in terms of what these functions do.

** getAtRestTipPosition. **  Computes the value of __s__ as defined in the problem set.

** initializeBunnyPhysics. **  Sets the initialization values for `g_tipPos` and `g_tipVelocity`. Nothing too special going on here.

** computeHairVertex. **  Computes the hair tip vertex for the <i>i</i>th layer for a given bunny surface vertex.

** getBunnyShellGeometryVertices. **  Returns a vector of the `VertexPNX`s used for the <i>layer</i>th shell in the bunny.

** updateHairCalculation. **  Performs the animation simulation for the tip associated with the provided vertex in the bunny shell.

** updateHairs. **  Repeatedly calls `updateHairCalculation` in order to update all of the hair tips on for the bunny.

** hairsSimulationCallback. **  Main animation loop for the bunny simulation. Calls `updateHairs` a number `g_numStepsPerFrame` of times per frame in order to simulate the physics acting on the hairs.

** prepareBunnyForRendering. **  Called before the bunny is rendered. From the simulated tip computations, this computes the actual vertex locations of each shell and uploads them into the appropriate entry in `g_bunnyShellGeometries`.


### Testing ###
Testing for this assignment was straightforward.

** Bunny. **  Is there a bunny being rendered? Check.

** Bunny transforms. **  Can the bunny be picked, rotated, and translated correctly, and is it smooth? Check.

** Furry. **  Does the bunny have fur? Check.

** Curvy fur. **  Does the bunny have curvy fur when stationary? Check.

** Animated fur. **  Does the bunny's fur move in a realistic fashion when subjected to the following interactions:
* Bunny is slowly rotated? Check.
* Bunny is vigorously rotated? Check.
* Bunny is slowly translated? Check.
* Bunny is vigorously translated? Check.
* Bunny is gently caressed? Unfortunately, we were unable to check this one.

** Normals. **  It's important to check that the fur normals are displayed correctly. It's difficult to test this programmatically. The only way that we could test this was to compare the visual reflection properties of the bunny in our implementation to those in the official solution. They appear to be the same, very very shiny.

** Responds to keyboard. **  We ensured that the bunny did the following:
* When the right arrow key is pressed, the fur grows longer.
* When the left arrow key is pressed, the fur grows shorter.
* When the up arrow key is pressed, the bunny becomes hairier.
* When the down arrow key is pressed, the bunny becomes less hairy.

### Above and Beyond ###
We did not implement anything above and beyond the assignment.
