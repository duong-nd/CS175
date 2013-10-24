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
  vector< shared_ptr<SgRbtNode> > nodes;
  vector<RigTForm> rbts = vector<RigTForm>();

public:
  Frame(shared_ptr<SgNode> root) {
    /* Dumps the current scene into nodes. */
    dumpSgRbtNodes(root, nodes);
    /* Iterates through the nodes and stores their corresponding RBTs into rbts. */
    for (int i = 0; i < nodes.size(); i++) {
      rbts.push_back(nodes[i]->getRbt());
    }
  }

  void showFrameInScene() {
    for (int i = 0; i < nodes.size(); i++) {
      nodes[i]->setRbt(rbts[i]);
    }
  }
};

#endif
