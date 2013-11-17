#include "debug.h"

static const int    g_numShells = 24;
static       double g_furHeight = 0.21;
static       double g_hairyness = 0.7;

static const Cvec3  g_gravity(0, -0.5, 0);
static       double g_timeStep = 0.02;
static       double g_numStepsPerFrame = 10;
static       double g_damping = 0.96;
static       double g_stiffness = 4;

/* The hair tip position in world-space coordinates */
static std::vector<Cvec3> g_tipPos;
/* The hair tip velocity in world-space coordinates */
static std::vector<Cvec3> g_tipVelocity;

/**
 * Computes the vertex on a bunny shell.
 *
 * For each vertex with position p, compute the longest hair position s
 * Compute n = (s - p) / g_numShells
 * Compute our specific vertex position with p + n * layer
 *
 * @param       v The vertex on the bunny itself.
 * @param       i The layer of the bunny that we're computing.
 * @param vertNum The number of the vertex we're currently doing. 0, 1, or 2.
 */
static VertexPNX computeHairVertex(Mesh::Vertex v, int i, int vertNum) {
  return VertexPNX(
    v.getPosition() + (v.getNormal() * g_furHeight / g_numShells) * i,
    v.getNormal(),
    Cvec2(vertNum == 1 ? g_hairyness : 0, vertNum == 2 ? g_hairyness : 0)
  );
}

/**
 * Returns the vertices for the layer-th layer of the bunny shell.
 */
static vector<VertexPNX> getBunnyShellGeometryVertices(Mesh &mesh, int layer) {
  vector<VertexPNX> vs;
  /* For each face: */
  for (int i = 0; i < mesh.getNumFaces(); i++) {
    Mesh::Face f = mesh.getFace(i);
    /* For each vertex of each face: */
    for (int j = 1; j < f.getNumVertices() - 1; j++) {
      vs.push_back(computeHairVertex(f.getVertex(  0), layer, 0));
      vs.push_back(computeHairVertex(f.getVertex(  j), layer, 1));
      vs.push_back(computeHairVertex(f.getVertex(j+1), layer, 2));
    }
  }

  return vs;
}
