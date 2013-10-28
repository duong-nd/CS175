Computer Science 175
====================
Assignment 5
------------
This README was written using Markdown syntax -- consider using a Markdown viewer like [Dillinger](http://dillinger.io/) for a more pleasureable viewing experience!

### Authors
- Michael Tingley, < michaeltingley@college.harvard.edu >
- Michael Traver, < mtraver@college.harvard.edu >

### Files Submitted
* headers/
  * arcball.h: (as provided)
  * asstcommon.h: (as provided)
  * cvec.h: (as provided)
  * drawer.h: (as provided)
  * frame.h: Our data structure for maintaining individual frames, their corresponding shared node pointers, and the rigid body transforms that each frame owns.
  * geometrymaker.h: (as provided)
  * glsupport.h: (as provided)
  * matrix4.h: (as provided)
  * picker.h: (as provided)
  * ppm.h: (as provided)
  * quat.h: (as provided)
  * rigtform.h: (as provided)
  * scenegraph.h: (as provided)
  * script.h: Our data structure for maintaining the current frame state as well as the possible keyframes in the animation.
  * sgutils.h: (as provided)
  * utils.h: Our helper file. Right now we just use this for string splitting.
* shaders/: (as provided)
  * basic-gl2.vshader: (as provided)
  * diffuse-gl2.fshader: (as provided)
  * pick-gl2.fshader (as provided)
  * solid-gl2.fshader: (as provided)
* AUTHORS: (as provided)
* LICENSE: (as provided)
* Makefile: Slightly modified the Makefile so that it produces an output file called `asst` rather than `asst<assignment number>`.
* README.md: This file, in Markdown format.
* asst.cpp: The C++ file, now responding to frame update requests and with animation support.
* glsupport.cpp: (as provided)
* picker.cpp: (as provided)
* ppm.cpp: (as provided)
* scenegraph.cpp: (as provided)
* script.script: Our saved script file that can be loaded or overwritten. It's stored in our own custom format.

### Platform
We developed on two different platforms. Michael Tingley developed on his personal laptop, running Ubuntu 13.04. Michael Traver developed on his personal laptop, running Mac OS X. We developed using OpenGL2, however, for consistency with each other's code.

### Compiling and Running
Simply run the command `make all; ./asst` to compile and run the code.

### Requirements
We met all problem set requirements. We compared our result against the solution binary and it behaves in the the same way.

### Code Design
The code design choices for this assignment were much more open-ended than previously.

We created the `Frame` class and `Script` class, and utilized the `utils.h` helper header. The header file will collect miscellaneous helper methods, but right now just contains functionality for string splitting.

The classes are each responsible for owning data used in generating the scene and maintaining its state. `Frame` maintains the shared pointer of scene nodes as well as a list of `RigTForms`. If we want to display the scene, we can call `showFrameInScene`, which will cause the frame to load its RBT contents into its shared pointer of nodes (and thus they will be represented in the scene when it is next drawn). It also has operations for interpolating frames, serializing, and deserializing frames.

`Script` is responsible for maintaining the current frame and handling operations associated with it. It directly implements most of the functionality described in Task 1. It has private fields that are the full list of frames and an iterator that points to the current frame. It delegates frame-specific functionality (such as rendering or interpolating frames) to the frame classes themselves. This class is also the main interface point for the file i/o used in this pset to save and load the script file, named `script.script`.

We added methods to `RigTForm` to carry out lerping and slerping for the components of `RigTForm`s. These are straightforward from the lecture notes, with the following exception: if we try to slerp two identical quaternions, we will get a segfault. Thus, we implemented the equality operator for quaternions, and first check if they are equal. If they are, we just return one of them rather than trying to compute the slerp. (In fact, if you think about it, it's clear that in this case this is exactly what slerp would carry out if it were a defined operation.)

The logic for playing the animation was somewhat hairy, since we can't simply index into our list of frames (it is a linked list, after all).

### Testing
Testing for this assignment was much more demanding than previous assignments.

**Task 1.**  REMARKS

### Above and Beyond
We did not implement anything above and beyond the assignment.

























