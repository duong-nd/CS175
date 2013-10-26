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

#include "scenegraph.h"
#include "sgutils.h"
#include "frame.h"

using namespace std;
using namespace tr1;

class Script {
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

public:
  /**
   * Returns the number of stored keyframes.
   */
  int getNumberOfKeyframes() {
    return frames.size();
  }

  /**
   * Renders the current frame in the scene.
   */
  void showCurrentFrameInScene() {
    if (defined()) {
      iter->showFrameInScene();
    }
    DEBUG();
  }

  /**
   * Replace CurrentFrame with the current frame from the scene.
   */
  void replaceCurrentFrameFromScene(shared_ptr<SgRootNode> rootNode) {
    if (defined()) {
      Frame frame = Frame(rootNode);
      *iter = frame;
      iter->showFrameInScene();
    } else {
      createNewFrameFromSceneAfterCurrentFrame(rootNode);
    }

    DEBUG();
  }

  /**
   * Steps the current frame forward.
   */
  void advanceCurrentFrame() {
    if (defined()) {
      iter++;
      if (iter == frames.end()) {
        iter--;
      } else {
        iter->showFrameInScene();
      }
    }

    DEBUG();
  }

  /**
   * Steps the current frame backwards.
   */
  void regressCurrentFrame() {
    if (defined()) {
      if (iter != frames.begin()) {
        iter--;
        iter->showFrameInScene();
      }
    }

    DEBUG();
  }

  /**
   * Deletes the current frame.
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
   */
  void deleteCurrentFrame() {
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
      iter->showFrameInScene();
    }

    DEBUG();
  }

  /**
   * Creates a new frame (after the current frame) from the scene.
   *
   * If the current key frame is defined, create a new key frame immediately
   * after current key frame. Otherwise just create a new key frame. Copy
   * scene graph RBT data to the new key frame. Set the current key frame to
   * the newly created key frame.
   */
  void createNewFrameFromSceneAfterCurrentFrame(shared_ptr<SgRootNode> rootNode) {
    Frame frame = Frame(rootNode);

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

    DEBUG();
  }

  /**
   * Interpolates between the frame with index frameNumber and frameNumber + 1,
   * as if we were alpha % between these two frames. Will fail if you try to
   * interpolate on a frame before the first frame, on the last frame, or with
   * an alpha not in [0, 1]
   * @param rootNode    The root node of the scene, so that we can display to
   *                    the scene's current nodes.
   * @param frameNumber First of the pair of frames to interpolate between. This
   *                    should be between 1 and frames.size() - 2.
   * @param alpha       The "weight" of the second frame relative to the first.
   *                    0 means the first frame is shown, 0 means the second
   *                    frame is shown. This should be between 0 and 1.
   */
  void interpolate(
      shared_ptr<SgRootNode> rootNode,
      int frameNumber,
      float alpha) {

  }

  /**
   * Loads a script from the default file.
   */
  void loadScriptFromFile(string filename, shared_ptr<SgRootNode> rootNode) {
    ifstream file;
    file.open(filename.c_str());
    vector<string> serialized = vector<string>();
    string currentString;
    while (file >> currentString) {
      serialized.push_back(currentString);
    }

    list<Frame> newFrames = list<Frame>();
    for (int i = 0; i < serialized.size(); i++) {
      Frame::deserialize(serialized[i]);
    }
  }

  /**
   * Writes a script to the default file.
   */
  void writeScriptToFile(string filename) {
    ofstream file;
    file.open(filename.c_str());

    string serialized = "";
    for (list<Frame>::iterator it = frames.begin(); it != frames.end(); ++it) {
      serialized += it->serialize() + "\n";
    }

    file << serialized << "\n";
    file.close();
  }
};

#endif
