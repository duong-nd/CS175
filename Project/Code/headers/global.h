#ifndef GLOBAL_H
#define GLOBAL_H

#if __GNUG__
#   include <tr1/memory>
#endif
#include "scenegraph.h"


/* For string, vector, iostream, and other standard C++ stuff */
using namespace std;
/* For shared_ptr */
using namespace tr1;

shared_ptr<SgRootNode> g_world;

#endif
