#ifndef SGUTILS_H
#define SGUTILS_H

#include <vector>

#include "scenegraph.h"

struct RbtNodesScanner : public SgNodeVisitor {
  typedef std::vector<std::tr1::shared_ptr<SgRbtNode> > SgRbtNodes;

  SgRbtNodes& nodes_;

  RbtNodesScanner(SgRbtNodes& nodes) : nodes_(nodes) {}

  virtual bool visit(SgTransformNode& node) {
    using namespace std;
    using namespace tr1;
    shared_ptr<SgRbtNode> rbtPtr = dynamic_pointer_cast<SgRbtNode>(node.shared_from_this());
    if (rbtPtr)
      nodes_.push_back(rbtPtr);
    return true;
  }
};

inline void dumpSgRbtNodes(std::tr1::shared_ptr<SgNode> root, std::vector<std::tr1::shared_ptr<SgRbtNode> >& rbtNodes) {
  RbtNodesScanner scanner(rbtNodes);
  root->accept(scanner);
}


#endif