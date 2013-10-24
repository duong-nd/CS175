#include <vector>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include "scenegraph.h"
#include "sgutils.h"

class Frame {
private:
    std::vector<std::tr1::shared_ptr<SgNode> > nodes;
    std::vector<RigTForm> rbts;

public:
    Frame(shared_ptr<SgNode> root) {
        dumpSgRbtNodes(root, nodes);
    }
}