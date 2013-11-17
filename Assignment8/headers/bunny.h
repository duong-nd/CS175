#include "debug.h"

/* Some constant shits */
static const int    g_numShells = 24;
static       double g_furHeight = 0.21;
static       double g_hairyness = 0.7;
static const Cvec3  g_gravity(0, -0.5, 0);
static       double g_timeStep = 0.02;
static       double g_numStepsPerFrame = 10;
static       double g_damping = 0.96;
static       double g_stiffness = 4;

/* Used for physical simulation */
static int g_simulationsPerSecond = 60;

/* The hair tip position in world-space coordinates */
static std::vector<Cvec3> g_tipPos;
/* The hair tip velocity in world-space coordinates */
static std::vector<Cvec3> g_tipVelocity;

/**
 * Given a vertex on the bunny, returns the at-rest position of the hair tip.
 * @param  v The vertex on the bunny.
 * @return   The position of the tip of the hair.
 */
Cvec3 getAtRestTipPosition(Mesh::Vertex v) {
  return v.getNormal() * g_furHeight;
}

/**
 * Sets the tip positions to be the tips of the bunny's hair positions. Sets the
 * velocities to be initialized to zero.
 * @param mesh The bunny mesh.
 */
void initializeBunnyPhysics(Mesh &mesh) {
  for (int i = 0; i < mesh.getNumVertices(); i++) {
    g_tipPos.push_back(getAtRestTipPosition(mesh.getVertex(i)));
    g_tipVelocity.push_back(Cvec3());
  }
}

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
    v.getPosition() + (getAtRestTipPosition(v) / g_numShells) * i,
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

static void updateHairCalculation(...) {

}

/**
 * Updates the hair calculations for the bunny based on the physics simulation
 * descriptions provided in the assignment.
 */
static void updateHairCalculations(Mesh &mesh) {
  for (int i = 0; i < .getNumVertices(); i++) {

  }
}

/**
 * Performs dynamics simulation g_simulationsPerSecond times per second
 */
static void hairsSimulationCallback(int _) {
  /* Update the hair dynamics. HACK: Ideally, we'd be passing in g_bunnyMesh to
     this function, but that's hard since it's a fucking callback. */
  updateHairCalculations(g_bunnyMesh);
  /* Schedule this to get called again */
  glutTimerFunc(1000 / g_simulationsPerSecond, hairsSimulationCallback, 0);
  /* Force visual refresh */
  glutPostRedisplay();
}
