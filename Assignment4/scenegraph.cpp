#include <algorithm>

#include "headers/scenegraph.h"

using namespace std;
using namespace std::tr1;

bool SgTransformNode::accept(SgNodeVisitor& visitor) {
  if (!visitor.visit(*this))
    return false;
  for (int i = 0, n = children_.size(); i < n; ++i) {
    if (!children_[i]->accept(visitor))
      return false;
  }
  return visitor.postVisit(*this);
}

void SgTransformNode::addChild(shared_ptr<SgNode> child) {
  children_.push_back(child);
}

void SgTransformNode::removeChild(shared_ptr<SgNode> child) {
  children_.erase(find(children_.begin(), children_.end(), child));
}

bool SgShapeNode::accept(SgNodeVisitor& visitor) {
  if (!visitor.visit(*this))
    return false;
  return visitor.postVisit(*this);
}

class RbtAccumVisitor : public SgNodeVisitor {
protected:
  vector<RigTForm> rbtStack_;
  SgTransformNode& target_;
  bool found_;
public:
  RbtAccumVisitor(SgTransformNode& target): target_(target), found_(false) {}

  /**
   * Gets the last accumulated matrix from the saved stack in the visitor.
   * @param  offsetFromDestination  Allows you to back up from the destination
   *                                node.
   * @return                        The accumulated RBT.
   */
  const RigTForm getAccumulatedRbt(int offsetFromDestination = 0) {
    RigTForm accumulation = RigTForm();

    for (int i = 0; i < offsetFromDestination; i++) { rbtStack_.pop_back(); }
    for (int i = rbtStack_.size() - 1; i > 0; i--) {
      accumulation = rbtStack_[i] * accumulation;
    }

    return accumulation;
  }

  virtual bool visit(SgTransformNode& node) {
    if (node == target_) return false;
    rbtStack_.push_back(node.getRbt());
    return true;
  }

  virtual bool postVisit(SgTransformNode& node) {
    return true;
  }
};

RigTForm getPathAccumRbt(
  shared_ptr<SgTransformNode> source,
  shared_ptr<SgTransformNode> destination,
  int offsetFromDestination) {

  RbtAccumVisitor accum(*destination);
  source->accept(accum);
  return accum.getAccumulatedRbt(offsetFromDestination);
}
