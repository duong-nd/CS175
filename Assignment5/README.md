Computer Science 175
====================
Assignment 4
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasureable viewing experience!

### Authors
- Michael Tingley, <michaeltingley@college.harvard.edu>
- Michael Traver, <mtraver@college.harvard.edu>

### Files Submitted
* headers/
 * arcball.h: (as provided)
 * asstcommon.h: (as provided)
 * cvec.h: (as provided)
 * drawer.h: (as provided)
 * geometrymaker.h: (as provided)
 * glsupport.h: (as provided)
 * matrix4.h: (as provided)
 * picker.h: (as provided)
 * ppm.h: (as provided)
 * quat.h: (as provided)
 * rigtform.h: (as provided)
 * scenegraph.h: (as provided)
* shaders/: (as provided)
 * basic-gl2.vshader: (as provided)
 * diffuse-gl2.fshader: (as provided)
 * pick-gl2.fshader (as provided)
 * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`
* README.md: This file, in Markdown format.
* asst.cpp: The C++ file, now using scenegraphs.
* glsupport.cpp: (as provided)
* picker.cpp: Implemented methods to visit nodes and enode IDs in their colors.
* ppm.cpp: (as provided)
* scenegraph.cpp: Implemented methods to visit nodes and gather the accumulated rigid body transforms as necessary.

### Platform
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OS X. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements
We met all problem set requirements. We compared our result against the solution binary and it behaves in the the same way.

### Code Design
We updated all of the existing code to use `scenegraph`s.

We implemented the *Visitor Pattern* in `picker.cpp` and `scenegraph.cpp`. The visitor pattern in `picker` is responsible for visiting each node and encoding in their color values a unique ID. This allows us to select a node based on the node that the mouse is hovering above. The visitor pattern in `scenegraph` is used to allow us to accumulate rigid body transformations. This is done so that we can articulate child joints whenmodifying a parent joint.

Whenever the user presses `p`, we enter picking mode, and, as per the solution binary, do not hide the arcball. On the next mouse click, we do a final rerender of the scene and determine the ID of the joint over which the mouse was hovering. We then use this to perform transformations.

When a transformation is made, we use the accumulated RBT when applying the transformation. In this way, all children joints of the joint being articulated also get articulated in an identical way with respect to the joint that was originally being articulated.

We also modified the specification of the eye RBT using `getPathAccumRbt` so that the eye can assume any transform node as the eye frame. However, as per the pset spec, we limited this to just the sky node and the two robot nodes.

We built a complete robot in `constructRobot`, following the existing code architecture, with the following components:
* Head
* Left and right upper arm
* Left/right lower arm
* Left/right upper leg
* Left/right lower leg

### Testing
**Tasks 1 and 4.**  These tasks were tested visually, by ensuring that the results were as expected and that the robot was constructed as intended. We manipulated the environment after completing these steps to ensure that everything behaved as expected.

**Task 2.**  To test this task, we turned buffer swapping on. This allowed us to see the colors being encoded into our environment. We ensured that the different objects were encoded in different colors. We added print statements to check that different IDs were being selected when different components were being selected.

**Task 3.**  Testing this part was fairly straightforward. We turned on picking mode and tried picking a bunch of different parts. We ensured that the arcball moved to the beginning of each joint when each part was selected.

We tried moving around using the different components in the main scene. We tried manipulating each hierarchical component of the red and blue robots in order: manipulate (transform and rotate) the body, then the upper arm, then the lower arm. Each one of these operations should have behaved hierarchically: the body should move move and rotate the arm components, in addition to itself, the upper arm should move and rotate the lower arm in addition to itself, and the lower arm should rotate and translate itself (we had not implemented the other parts at that time).

We tried switching views and manipulating the encironment to ensure that nothing had changed.

### Above and Beyond
We did not implement anything above and beyond the assignment.
