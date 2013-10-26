#ifndef FRAME_H
#define FRAME_H

#include <vector>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include "scenegraph.h"
#include "sgutils.h"

using namespace std;
using namespace tr1;

class Frame {
private:
  vector< shared_ptr<SgRbtNode> > nodesInScene;
  vector<RigTForm> frameRBTs = vector<RigTForm>();

public:
  Frame(shared_ptr<SgNode> root) {
    /* Dumps the current scene into nodes. */
    dumpSgRbtNodes(root, nodesInScene);
    /* Iterates through the nodes and stores their corresponding RBTs into rbts. */
    for (int i = 0; i < nodesInScene.size(); i++) {
      frameRBTs.push_back(nodesInScene[i]->getRbt());
    }
  }

  void showFrameInScene() {
    for (int i = 0; i < nodesInScene.size(); i++) {
      nodesInScene[i]->setRbt(frameRBTs[i]);
    }
  }

  /**
   * Interpolates between the first frame and second frame, with the second
   * frame having a relative "weight" of alpha. Returns the interpolated frame.
   */
  static Frame interpolate(Frame firstFrame, Frame secondFrame, float alpha) {

  }

  /**
   * File-writable serialization string representing this frame.
   */
  string serialize() {
    string outString = "";
    for (int i = 0; i < frameRBTs.size(); i++) {
      outString += frameRBTs[i].serialize() + " ";
    }
    return outString;
  }

  /**
   * Reads and returns a frame from the serialized version.
   */
  static void deserialize(string serialized) {
    // TODO
  }
};

#endif
