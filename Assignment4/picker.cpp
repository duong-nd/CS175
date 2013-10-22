#include <GL/glew.h>

#include "headers/picker.h"

using namespace std;
using namespace std::tr1;

Picker::Picker(const RigTForm& initialRbt, const ShaderState& curSS)
  : drawer_(initialRbt, curSS)
  , idCounter_(0)
  , srgbFrameBuffer_(!g_Gl2Compatible) {}

bool Picker::visit(SgTransformNode& node) {
  nodeStack_.push_back(node.shared_from_this());
  return drawer_.visit(node);
}

bool Picker::postVisit(SgTransformNode& node) {
  nodeStack_.pop_back();
  return drawer_.postVisit(node);
}

bool Picker::visit(SgShapeNode& node) {
  idCounter_++;

  shared_ptr<SgRbtNode> p;
  for (int i = nodeStack_.size() - 1; i >= 0; i--) {
    p = dynamic_pointer_cast<SgRbtNode>(nodeStack_[i]);
    if (p != NULL) break;
  }
  addToMap(idCounter_, p);

  const Cvec3 color = idToColor(idCounter_);
  safe_glUniform3f(drawer_.getCurSS().h_uIdColor, color[0], color[1], color[2]);
  return drawer_.visit(node);
}

bool Picker::postVisit(SgShapeNode& node) {
  // TODO maybe nothing to add?
  return drawer_.postVisit(node);
}

shared_ptr<SgRbtNode> Picker::getRbtNodeAtXY(int x, int y) {
  // TODO
  return shared_ptr<SgRbtNode>(); // return null for now
}

//------------------
// Helper functions
//------------------
//
void Picker::addToMap(int id, shared_ptr<SgRbtNode> node) {
  idToRbtNode_[id] = node;
}

shared_ptr<SgRbtNode> Picker::find(int id) {
  IdToRbtNodeMap::iterator it = idToRbtNode_.find(id);
  if (it != idToRbtNode_.end())
    return it->second;
  else
    return shared_ptr<SgRbtNode>(); // set to null
}

// encode 2^4 = 16 IDs in each of R, G, B channel, for a total of 16^3 number of objects
static const int NBITS = 4, N = 1 << NBITS, MASK = N-1;

Cvec3 Picker::idToColor(int id) {
  assert(id > 0 && id < N * N * N);
  Cvec3 framebufferColor = Cvec3(id & MASK, (id >> NBITS) & MASK, (id >> (NBITS+NBITS)) & MASK);
  framebufferColor = framebufferColor / N + Cvec3(0.5/N);

  if (!srgbFrameBuffer_)
    return framebufferColor;
  else {
    // if GL3 is used, the framebuffer will be in SRGB format, and the color we supply needs to be in linear space
    Cvec3 linearColor;
    for (int i = 0; i < 3; ++i) {
      linearColor[i] = framebufferColor[i] <= 0.04045 ? framebufferColor[i]/12.92 : pow((framebufferColor[i] + 0.055)/1.055, 2.4);
    }
    return linearColor;
  }
}

int Picker::colorToId(const PackedPixel& p) {
  const int UNUSED_BITS = 8 - NBITS;
  int id = p.r >> UNUSED_BITS;
  id |= ((p.g >> UNUSED_BITS) << NBITS);
  id |= ((p.b >> UNUSED_BITS) << (NBITS+NBITS));
  return id;
}
