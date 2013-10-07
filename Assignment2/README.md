Computer Science 175
====================
Assignment 2
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasurable viewing experience!

### Authors
- Michael Tingley <michaeltingley@college.harvard.edu>
- Michael Traver <mtraver@college.harvard.edu>

### Files Submitted
* shaders/
 * basic-gl2.vshader: (as provided)
 * diffuse-gl2.fshader: (as provided)
 * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: (as provided)
* README.md: This file, in Markdown format.
* asst2.cpp: The C++ file used for CPU handling of this assignment.
* asst2.sln: (as provided)
* asst2.vcxproj: (as provided)
* asst2.vcxproj.filters: (as provided)
* cvec.h: (as provided)
* geometrymaker.h: (as provided)
* glsupport.cpp: (as provided)
* glsupport.h: (as provided)
* matrix4.h: The header file containing the logic for matrix manipulations. This was edited to provided support for `transFact` and `linFact`.
* ppm.cpp: (as provided)
* ppm.h: (as provided)

### Platform
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OSX. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst2` to compile and run the code.

### Requirements
We met all problem set requirements. We compared our result against the solution binaries and it behaves the same. This includes the somewhat inconsistent behavior in the solution binary with regards to how the world is translated when in the different view modes of the eye. When in the 'pivot' view mode, the world translates in one direction when the mouse is moved up (for example), and when in the 'orbit' view mode, the world translates in the opposite direction when the mouse is moved up. This is consistent with the solution binary.

### Code Design
We filled in the bodies of `transFact` and `linFact` in `matrix4.h`. We have to maintain a record of which object we are currently viewing from(`g_currentViewIndex`), which object we're currently transforming (`g_objectBeingManipulated`), and which view mode that we are in for the eye (`g_skyAMatrixChoice`). We also maintain a `matrix4` frame that is which object we are currently transforming with respect to (`g_aFrame`). Since these are states that need to be known throughout the program, we decided to make these global variables.

To create the second cube, we simply modified `init_cubes` to add a new cube geometry.

Much of the user interaction to this program was done by calling functions in the `keyboard` callback. When we receive a `v` keypress, we call the `toggleEyeMode` function, which is responsible for setting a new `g_currentViewIndex`. When we receive an `o` keypress, we call `cycleManipulation`, which is responsible for setting a new `g_objectBeingManipulated`. When we receive an `m` keypress, we call `cycleSkyAMatrix`, which toggles our view mode for when our eye is in the sky.

The transformations were all carried out by modifying `motion`. The main modifications were to correctly set what the cubes were being transformed with respect to and then do the correct transformation, as per the `doMtoOwrtA` algorithm outlined in class. We first call `setWrtFrame`, which correctly updates `g_aFrame` according to whatever the user had set (these are stored in global variables). This uses `transFact` and `linFact` to extract the correct translation and rotation components to ensure that the correct axis is being rotated or translated upon. Then, we set invert modifiers as necessary if we're transforming the object that we're viewing from so that translation and rotation behaves as per the spec. Finally, we actually carry out the `doMtoOwrtA` algorithm, and update object states.

### Testing
Testing is straightforward from the assignment specification.
* Step 1
 * There should be two cubes.
 * These cubes should not be overlapping.
 * The second cube should be a different color from the red cube.
* Step 2
 * Tests for this section are done as a part of tests for subsequent sections.
* Step 3
 * The view should initially start as the sky, looking onward (longingly) at the two (lonely) cubes.
 * Press the `v` key. We should now be looking from the red cube.
 * Press the `v` key. We should now be looking from the cube that we made.
 * Press the `v` key. We should now be looking from the sky again.
* Step 4
 * Test the eye's behavior when the eye is in the sky:
  * Test the 'orbit' (world-sky) view mode behavior:
   * Ensure that orbiting is done as is done in the binary solution.
   * Ensure that translation is done as is done in the binary solution.
  * Test the 'pivot' (sky-sky) view mode behavior:
   * Press the `m` key to switch to the 'pivot' view mode.
   * Ensure that pivoting is done as is done in the binary solution.
   * Ensure that translation is done as is done in the binary solution.
 * Test transforming the red cube when the eye is in the sky:
  * Press the `o` key to switch to transforming the first cube.
  * Rotate the cube. It should rotate intuitively as if you were moving it with your hand.
  * Translate the cube. It should translate intuitively as if you were sliding it with your hand.
  * Rotate it again. It should rotate along its own axis, not the world or sky's axis.
  * Translate it again. It should translate along your view's axis, not its axis or the world's axis.
 * Test transforming the second cube when the eye is in the sky:
  * Press the `o` key to switch to transforming the second cube.
  * Repeat the above tests, but for the second cube.
 * Test the eye in the sky one last time:
  * Press the `o` key to switch to transforming the sky.
  * Repeat the above eye-sky tests to ensure that nothing has changed as a result of having transformed the cubes.
 * Test transformations from the first cube:
  * Press `v` to switch to viewing from the first cube. Try translating and rotating the sky. Nothing should happen, since this action is not allowed. Switch back to the sky by pressing `v` to ensure that nothing has changed.
  * Switch back to the eye-in-the-cube. Try rotating the cube. It should rotate as per the 'pivot' view mode when we were the eye in the sky.
  * Try translating the cube. It should translate as per the 'pivot' view mode when we were the eye in the sky.
  * Press `m` to switch to 'orbit' mode. This mode is not supported for cubes. Try translating and rotating the cube and ensure that it behaves as per the 'pivot' view mode and not the 'orbit' view mode.
 * Test transformations from the second cube:
  * Press the `v` key to switch to viewing from the second cube.
  * Repeat the tests that we just did for the first cube, now as the second cube.

### Above and Beyond
We did not implement anything above and beyond the assignment.
