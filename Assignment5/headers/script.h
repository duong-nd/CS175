#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include "scenegraph.h"
#include "sgutils.h"
#include "frame.h"

using namespace std;
using namespace tr1;

class Script {
private:

  const string DEFAULT_SCRIPT_FILENAME = "script.script";

  class CurrentFrame {
  private:
    list<Frame> frames = list<Frame>();
    list<Frame>::iterator iter = frames.begin();

    /**
     * Returns whether the current frame is defined (i.e., if there are frames)
     * to be displayed.
     */
    bool defined() {
      return (iter != frames.end());
    }

  public:
    /**
     * Returns the current frame to be rendered.
     */
    Frame getFrame() {
      return iter*;
    }

    /**
     * Replaces the current frame with the given frame if the current frame is
     * defined. Otherwise creates a `newFrame`.
     *
     * Returns the Frame that is the CurrentFrame.
     */
    Frame update(Frame frame) {
      if (defined()) {
        *iter = frame;
      } else {
        newFrame(frame);
      }

      return *iter;
    }

    /**
     * If the current key frame is defined, create a new key frame immediately
     * after current key frame. Otherwise just create a new key frame. Copy
     * scene graph RBT data to the new key frame. Set the current key frame to
     * the newly created key frame.
     *
     * Returns the Frame that is the CurrentFrame.
     */
    Frame newFrame(Frame frame) {
      if (defined()) {
        iter++;
        /* Insert inserts before current iterator position. */
        frames.insert(iter, frame);
        iter--;
      } else {
        frames.push_back(frame);
        iter = frames.begin();
      }

      return *iter;
    }

    /**
     * Deletes the frame that the CurrentFrame currently represents.
     *
     * If the current key frame is defined, delete the current key frame and do
     * the following:
     *   - If the list of frames is empty after the deletion, set the current
     *     key frame to undefined.
     *   - Otherwise:
     *     - If the deleted frame is not the first frame, set the current frame
     *       to the frame immediately before the deleted frame.
     *     - Else set the current frame to the frame immediately after the
     *       deleted frame.
     *
     * Returns the Frame that is the CurrentFrame.
     */
    Frame deleteFrame() {
      if (defined()) {
        list<Frame>::iterator temp = iter;

        if (iter == frames.begin()) {
          /* Deleting the first frame, so result is set to the next frame. */
          temp++;
        } else {
          /* Deleting a non-first frame, so result is set to the prev. frame. */
          temp--;
        }
        frames.erase(iter);
        iter = temp;
      }

      return *iter;
    }

  } currentFrame = CurrentFrame();

public:
  /**
   * Renders the current frame.
   */
  void showCurrentFrameInScene() {

  }

  /**
   * Replace CurrentFrame with the current frame from the scene.
   */
  void replaceCurrentFrameFromScene() {

  }

  /**
   * Steps the current frame forward.
   */
  void advanceCurrentFrame() {

  }

  /**
   * Steps the current frame backwards.
   */
  void regressCurrentFrame() {

  }

  /**
   * Deletes the current frame.
   */
  void deleteCurrentFrame() {

  }

  /**
   * Creates a new frame (after the current frame) from the scene.
   */
  void createNewFrameFromSceneAfterCurrentFrame() {

  }

  /**
   * Loads a script from the default file.
   */
  void loadScriptFromFile() {

  }

  /**
   * Writes a script to the default file.
   */
  void writeScriptToFile() {

  }
};

#endif
