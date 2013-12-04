#ifndef WATERY_HELL
#define WATERY_HELL
/**
 * This is the header for water.
 */

#include <string>
#include <vector>

using namespace std;

/** GLOBALS *******************************************************************/
/** The filename for the file containing the subdivision surface. */
static const string g_waterMeshFilename = "watery_hell.mesh";
static Mesh g_waterOriginal;


/** FUNCTION PROTOTYPES *******************************************************/
static void updateMeshNormals(Mesh &mesh);

/** FUNCTIONS *****************************************************************/

/**
 * Initializes the water
 */
static void initWater() {
  g_waterOriginal = Mesh();
  g_waterOriginal.load(g_waterMeshFilename.c_str());
  updateMeshNormals(g_waterOriginal);

  g_subdivisionSurfaceMeshActual = Mesh(g_waterOriginal);

  vector<VertexPN> verticies = getGeometryVertices(g_subdivisionSurfaceMeshActual);

  g_subdivisionSurface.reset(new SimpleGeometryPN());
  g_subdivisionSurface->upload(&verticies[0], verticies.size());

  animateSubdivisionSurfaceCallback(0);
}

/**
 * Updates mesh normals based on surrounding vertices
 */
static void updateMeshNormals(Mesh &mesh) {
  /* reset normals */
  for (int i = 0; i < mesh.getNumVertices(); i++) {
    mesh.getVertex(i).setNormal(Cvec3());
  }

  for (int i = 0; i < mesh.getNumVertices(); i++) {
    Cvec3 vecSum = Cvec3();
    Mesh::Vertex currentVertex = mesh.getVertex(i);

    Mesh::VertexIterator it(currentVertex.getIterator()), it0(it);
    do {
      vecSum += it.getFace().getNormal();
    } while (++it != it0);

    if (dot(vecSum, vecSum) > CS175_EPS2) {
      vecSum.normalize();
    }
    currentVertex.setNormal(vecSum);
  }
}

#endif
WHATCHA GONNA DO ABOUT IT MOTHERDUCKTYPER?
