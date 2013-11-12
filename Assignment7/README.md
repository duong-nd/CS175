Computer Science 175
====================
Assignment 7
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
  * frame.h: (as provided)
  * geometry.h: (as provided)
  * geometrymaker.h: (as provided)
  * glsupport.h: (as provided)
  * material.h: (as provided)
  * matrix4.h: (as provided)
  * mesh.h: (as provided)
  * picker.h: (as provided)
  * ppm.h: (as provided)
  * quat.h: (as provided)
  * renderstates.h: (as provided)
  * rigtform.h: (as provided)
  * scenegraph.h: (as provided)
  * script.h: (as provided)
  * sgutils.h: (as provided)
  * texture.h: (as provided)
  * uniforms.h: (as provided)
  * utils.h: (as provided)
* shaders/: (as provided)
  * basic-gl2.vshader: (as provided)
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
* asst.cpp: The main C++ file, now with subdivision surfacing.
* cube.mesh: (as provided)
* geometry.cpp: (as provided)
* glsupport.cpp: (as provided)
* material.cpp: (as provided)
* mesh.interface: (as provided)
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
We maintain two meshes in global variables. One mesh is the original mesh, and the other mesh is our temporary mesh, which gets updated with subdividing, oscillating vertices, etc.

** Task 1. ** We created a new method, `initSubdivisionSurface`. This is called like the other init methods, but simply reads in the provided mesh file into a global variable. It then parses through the vertices of the quads and turns them into triangles. We construct the triangles using the first point and every other two sequential points. For instance, if we had a pentagon, we would break this into triangles using vertex groupings: {1, 2, 3}, {1, 3, 4}, and {1, 4, 5}. We assign them appropriate normals, based on whether we're using specular shading or not, and upload them into the geometry, which is also a global variable.

We also added a new `SgRbtNode` to the scene graph, and a shape node child constructed from the geometry in order to have the cube rendered in the world.

** Task 2. ** Smooth shading was accomplished by taking an average of all of the face vertices surrounding a given vertex. We do this using a `vertexIterator`. All of this is done before uploading the vertices to the geometry.

** Task 3. ** We did this in the same manner as the animation assignment. We computed the new vertex values by taking the original mesh template, and adding a small sinusoidal scaling factor to each vertex, which had offsets so that they were visually out of phase. The exact sinusoidal offset can be seen in the code. Every time the cube is drawn, we upload the new vertex values to the geometry.

** Task 4. ** Subdivision was fairly straightforward. We made a large number of functions that handled the recursive nature of the tasks at hand. We have a 'toplevel' function that handles the subdivision of mesh, called `applySubdivision`. This calls the three sub-routines, called `applyFaceSubdivisions`, `applyEdgeSubdivisions`, and `applyVertexSubdivisions`. Each one of these iterates through the appropriate pieces of the mesh and iteratively call, respectively, `getFaceSubdivisionVertex`, `getEdgeSubdivisionVertex`, and `getVertexSubdivisionVertex`. These methods are responsible for directly carrying out the math needed to update the appropriate face vertex, edge vertex, or vertex vertex. The math can be seen in more detail in the code.

Implementing the keyboard callback was very simple. To turn on or off smooth shading, we simply either upload the face normals or the average face normals to the geometry when rendering the subdivision surface. In order to change the number of subdivision surfaces, we change the number of times that we recursively divide the surface. To change the speed, we simply change the amount of 'time' that we jump forward after each iteration of the timer loop.

### Testing ###
Testing for this assignment was very straightforward.

We tested spcular shading by turning off and on smooth shading and observing changes in the rendering surface. Smooth should not be able to see corners.

We tested oscillating vertices by observation. They should oscillate at different speeds and out-of-phase. We tried changing the speed to very slow and very fast to ensure that everything still worked correctly.

To test subdivision surfaces, we tried changing the number of subdivisions and ensuring that our solution looked like the solution binary's. We tried changing the number of subdivision surfaces to a very high amount to ensure that everything slowed down a lot. Indeed it did :) We also made sure that you coldn't subdivide less than zero.

We also tried pickign different objects, and rotating and translating the subdivision surface to ensure that everything behaved as expected and that the animation was not stopped prematurely.

### Above and Beyond ###
We did not implement anything above and beyond the assignment.
