#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif
#include <iostream>
#include <fstream>
#include <sstream>

#include "scenegraph.h"
#include "sgutils.h"
#include "frame.h"

using namespace std;
using namespace tr1;

class Script {
private:
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
     * Updates the scene to show the new current frame.
     */
    void showCurrentFrameInScene() {
      if (defined()) {
        iter->showFrameInScene();
      }
    }

    /**
     * Replaces the current frame with the given frame if the current frame is
     * defined. Otherwise creates a `newFrame`.
     *
     * Updates the scene to show the new current frame.
     */
    void update(Frame frame) {
      if (defined()) {
        *iter = frame;
        iter->showFrameInScene();
      } else {
        newFrame(frame);
      }
    }

    /**
     * If the current key frame is defined, create a new key frame immediately
     * after current key frame. Otherwise just create a new key frame. Copy
     * scene graph RBT data to the new key frame. Set the current key frame to
     * the newly created key frame.
     *
     * Updates the scene to show the new current frame.
     */
    void newFrame(Frame frame) {
      if (defined()) {
        iter++;
        /* Insert inserts before current iterator position. */
        frames.insert(iter, frame);
        iter--;
      } else {
        frames.push_back(frame);
        iter = frames.begin();
      }

      iter->showFrameInScene();
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
     * Updates the scene to show the new current frame.
     */
    void deleteFrame() {
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

      if (defined()) {
        iter->showFrameInScene();
      }
    }

    /**
     * Steps the current frame forward if possible.
     *
     * Updates the scene to show the new current frame.
     */
    void stepForward() {
      if (defined()) {
        iter++;
        if (iter == frames.end()) {
          iter--;
        } else {
          iter->showFrameInScene();
        }
      }
    }

    /**
     * Steps the current frame backward if possible.
     *
     * Updates the scene to show the new current frame.
     */
    void stepBackward() {
      if (defined()) {
        if (iter != frames.begin()) {
          iter--;
          iter->showFrameInScene();
        }
      }
    }

    /**
     * Serializes frames and returns the string to be written to a file.
     */
    string serializeFrames() {
      stringstream s;
      for (list<Frame>::iterator it = frames.begin(); it != frames.end(); ++it) {
        s << it->serialize() << "\n";
      }
      return s.str();
    }

    /**
     * Deserializes the frames represented by a string, loads them, and sets the
     * current frame to be the first one.
     */
    void deserializeFrames(vector<string> serialized) {
      list<Frame> newFrames = list<Frame>();
      for (int i = 0; i < serialized.size(); i++) {
        Frame::deserialize(serialized[i]);
      }
    }

    void DEBUG() {
      for (list<Frame>::iterator it = frames.begin(); it != frames.end(); ++it) {
        cout << "  ";
        if (it == iter) cout << "[[";
        cout << &it;
        if (it == iter) cout << "]]";
        cout << "  ";
      }
      cout << endl;
    }

  } currentFrame = CurrentFrame();

public:
  /**
   * Renders the current frame in the scene.
   */
  void showCurrentFrameInScene() {
    currentFrame.showCurrentFrameInScene();
    currentFrame.DEBUG();
  }

  /**
   * Replace CurrentFrame with the current frame from the scene.
   */
  void replaceCurrentFrameFromScene(shared_ptr<SgRootNode> rootNode) {
    currentFrame.update(Frame(rootNode));
    currentFrame.DEBUG();
  }

  /**
   * Steps the current frame forward.
   */
  void advanceCurrentFrame() {
    currentFrame.stepForward();
    currentFrame.DEBUG();
  }

  /**
   * Steps the current frame backwards.
   */
  void regressCurrentFrame() {
    currentFrame.stepBackward();
    currentFrame.DEBUG();
  }

  /**
   * Deletes the current frame.
   */
  void deleteCurrentFrame() {
    currentFrame.deleteFrame();
    currentFrame.DEBUG();
  }

  /**
   * Creates a new frame (after the current frame) from the scene.
   */
  void createNewFrameFromSceneAfterCurrentFrame(shared_ptr<SgRootNode> rootNode) {
    currentFrame.newFrame(Frame(rootNode));
    currentFrame.DEBUG();
  }

  /**
   * Loads a script from the default file.
   */
  void loadScriptFromFile(string filename, shared_ptr<SgRootNode> rootNode) {
    ifstream file;
    file.open(filename.c_str());
    vector<string> serializedFrames = vector<string>();
    string currentString;
    while (file >> currentString) {
      serializedFrames.push_back(currentString);
    }
    currentFrame.deserializeFrames(serializedFrames);
  }

  /**
   * Writes a script to the default file.
   */
  void writeScriptToFile(string filename) {
    ofstream file;
    file.open(filename.c_str());
    file << currentFrame.serializeFrames() << "\n";
    file.close();
  }
};

#endif
