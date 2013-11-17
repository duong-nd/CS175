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
 * Computes the position of a hair vertex on a bunny shell.
 *
 * For each vertex with position p, compute the longest hair position s
 * Compute n = (s - p) / g_numShells
 * Compute our specific vertex position with p + n * layer
 *
 * @param  p The vertex position on the bunny.
 * @param  N The normal at the above vertex position.
 * @param  i The layer of the bunny that we're computing.
 * @return   Returns the position of the hair vertex.
 */
static Cvec3 computeHairVertexPosition(Cvec3 p, Cvec3 N, int i) {
  return p + (N * g_furHeight / g_numShells) * i;
}

/**
 * Returns the vertices for the layer-th layer of the bunny shell.
 */
static vector<VertexPNX> getBunnyShellGeometryVertices(Mesh &mesh, int layer) {
  vector<VertexPNX> vs;
  /* For each face: */
  for (int i = 0; i < mesh.getNumFaces(); i++) {
    Mesh::Face f = mesh.getFace(i);

    Cvec3 normals[3];
    /* For each vertex of each face: */
    for (int j = 1; j < f.getNumVertices() - 1; j++) {
      normals[0] = f.getNormal();
      normals[1] = f.getNormal();
      normals[2] = f.getNormal();

      Cvec3 vertexShellPosition1 = computeHairVertexPosition(f.getVertex(0).getPosition(), normals[0], layer);
      Cvec3 vertexShellPosition2 = computeHairVertexPosition(f.getVertex(j).getPosition(), normals[1], layer);
      Cvec3 vertexShellPosition3 = computeHairVertexPosition(f.getVertex(j+1).getPosition(), normals[2], layer);

      vs.push_back(VertexPNX(vertexShellPosition1, normals[1], Cvec2(0, 0)));
      vs.push_back(VertexPNX(vertexShellPosition2, normals[2], Cvec2(g_hairyness, 0)));
      vs.push_back(VertexPNX(vertexShellPosition3, normals[3], Cvec2(0, g_hairyness)));
    }
  }

  return vs;
}
