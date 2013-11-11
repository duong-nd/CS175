/*******************************************************************************
 *
 *   Harvard University
 *   CS175 : Computer Graphics
 *   Professor Steven Gortler
 *   Problem Set by Michael Tingley and Michael Traver
 *
 ******************************************************************************/

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include <GL/glew.h>
#ifdef __MAC__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "headers/cvec.h"
#include "headers/matrix4.h"
#include "headers/geometrymaker.h"
#include "headers/geometry.h"
#include "headers/ppm.h"
#include "headers/glsupport.h"
#include "headers/arcball.h"
#include "headers/quat.h"
#include "headers/rigtform.h"

#include "headers/asstcommon.h"
#include "headers/scenegraph.h"
#include "headers/drawer.h"
#include "headers/picker.h"

#include "headers/sgutils.h"
#include "headers/frame.h"
#include "headers/script.h"

#include "headers/mesh.h"

#define ESCAPE_KEY 27
#define SPACE_KEY  32

/* For string, vector, iostream, and other standard C++ stuff */
using namespace std;
/* For shared_ptr */
using namespace tr1;

/** G L O B A L S *************************************************************/
const bool g_Gl2Compatible = true;


/** A minimal of 60 degree field of view */
static const float g_frustMinFov = 60.0;
/** FOV in y direction (updated by updateFrustFovY) */
static float g_frustFovY = g_frustMinFov;

/** Near plane */
static const float g_frustNear = -0.1;
/** Far plane */
static const float g_frustFar = -50.0;
/** Y-coordinate of the ground */
static const float g_groundY = -2.0;
/** Half the ground length */
static const float g_groundSize = 10.0;

static int g_windowWidth = 512;
static int g_windowHeight = 512;
/** Is the mouse button pressed */
static bool g_mouseClickDown = false;
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
/** Coordinates for mouse click event */
static int g_mouseClickX, g_mouseClickY;
static int g_activeShader = 0;

static bool g_picking = false;

static shared_ptr<Material> g_redDiffuseMat,
                            g_blueDiffuseMat,
                            g_bumpFloorMat,
                            g_arcballMat,
                            g_pickingMat,
                            g_lightMat,
                            g_specularMat;

shared_ptr<Material> g_overridingMaterial;

static Script g_script = Script();

/** GEOMETRY */
typedef SgGeometryShapeNode MyShapeNode;

/**
 * Vertex buffer and index buffer associated with the ground and cube geometry
 */
static shared_ptr<Geometry> g_ground, g_cube, g_sphere;
static shared_ptr<SimpleGeometryPN> g_subdivisionSurface;
static shared_ptr<Mesh> g_subdivisionSurfaceMeshOriginal;
static shared_ptr<Mesh> g_subdivisionSurfaceMeshActual;
static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node;
static shared_ptr<SgRbtNode> g_meshNode;
static shared_ptr<SgRbtNode> g_light1Node, g_light2Node;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode;

/** SCENE */
static const int g_numObjects = 2;
static int g_currentViewIndex = 0;
static shared_ptr<SgRbtNode> g_currentView; /* set to g_skyNode in initScene() */

static double g_arcballScreenRadius = 1.0;
static double g_arcballScale = 1.0;

/**
 * Global constant representing the number of objects in the world (including
 * the sky).
 */
static const int g_numberOfViews = g_numObjects + 1;

/**
 * Frame that we're manipulating the current object with respect to. This is:
 * - If we're manipulating a cube and the eye is the sky, this should be the
 *   cube-sky frame.
 * - If we're manipulating cube i and eye is cube j, this should be the
 *   cube i-cube j frame.
 * - If we're manipulating the sky camera and eye is the sky, we have two
 *   viable frames, and pressing 'm' switches between them:
 *   - World-sky frame (like orbiting around the world)
 *   - Sky-sky frame (like moving your head)
 *
 * Initialize it to use world-sky in initScene() after g_SkyNode is initialized
 */
static RigTForm g_aFrame;

/**
 * controls whether we use the world-sky or sky-sky frame
 * when modifying the sky while the using the sky camera
 */
static const int WORLD_SKY = 0;
static const int SKY_SKY = 1;
static int g_skyViewChoice = WORLD_SKY;

/**
 * The name of the file to save and load the script to and from.
 */
static const string DEFAULT_SCRIPT_FILENAME = "script.script";

/** Whether or not the animation is currently playing. */
static bool g_animationPlaying = false;

/** 2 seconds between g_script */
static int g_msBetweenKeyFrames = 2000;
/** Frames to render per second during animation */
static int g_animateFramesPerSecond = 60;
/** Hack so that we know when we need to update the frame for the animation. */
static int g_lastAnimatedFrame = -1;

/** The filename for the file containing the subdivision surface. */
static const string g_subdivisionSurfaceFilename = "cube.mesh";

/** METHOD PROTOTYPES *********************************************************/
static void enablePickingMode();
static void disablePickingMode();
static void animateTimerCallback(int ms);
static SimpleGeometryPN readGeometryFromQuadFile(const string filename);
static void smoothShade(Mesh mesh);
void animateSubdivisionSurfaceCallback(int ms);
static shared_ptr<Mesh> applySubdivision(shared_ptr<Mesh> actualMesh);
static shared_ptr<Mesh> applyFaceSubdivisions(shared_ptr<Mesh> actualMesh);
static Cvec3 getFaceSubdivisionVertex(Mesh::Face f);
static shared_ptr<Mesh> applyEdgeSubdivisions(shared_ptr<Mesh> actualMesh);
static Cvec3 getEdgeSubdivisionVertex(Mesh::Edge e, shared_ptr<Mesh> actualMesh);

/** METHODS *******************************************************************/

static void initGround() {
  int ibLen, vbLen;
  getPlaneVbIbLen(vbLen, ibLen);

  /* Temporary storage for cube Geometry */
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makePlane(g_groundSize * 2, vtx.begin(), idx.begin());
  g_ground.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  /* Temporary storage for cube Geometry */
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeCube(1, vtx.begin(), idx.begin());
  g_cube.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void updateMeshVertices(shared_ptr<Mesh> meshActual, shared_ptr<Mesh> meshOriginal, float t) {
  for (int i = 0; i < meshOriginal->getNumVertices(); i++) {
    Cvec3 v = meshOriginal->getVertex(i).getPosition();
    meshActual->getVertex(i).setPosition(v + v * 0.75 * (sin(i + t / 100)));
  }
}

static void updateMeshNormals(shared_ptr<Mesh> mesh) {
  for (int i = 0; i < mesh->getNumVertices(); i++) {
    mesh->getVertex(i).setNormal(Cvec3());
  }
  for (int i = 0; i < mesh->getNumVertices(); i++) {
    Cvec3 vecSum = Cvec3();
    Mesh::Vertex currentVertex = mesh->getVertex(i);

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

static vector<VertexPN> getGeometryVertices(shared_ptr<Mesh> mesh) {
  vector<VertexPN> vs;
  for (int i = 0; i < mesh->getNumFaces(); i++) {
    Mesh::Face f = mesh->getFace(i);

    for (int j = 1; j < f.getNumVertices() - 1; j++) {
      vs.push_back(VertexPN(f.getVertex(0).getPosition(), f.getVertex(0).getNormal()));
      vs.push_back(VertexPN(f.getVertex(j).getPosition(), f.getVertex(j).getNormal()));
      vs.push_back(VertexPN(f.getVertex(j+1).getPosition(), f.getVertex(j+1).getNormal()));
    }
  }

  return vs;
}

/**
 * Applies Catmull-Clark subdivisions to the provided actual mesh using
 * levelsOfSubdivision subdivisions of the originalMesh.
 */
static shared_ptr<Mesh> applySubdivisions(
    shared_ptr<Mesh> actualMesh,
    shared_ptr<Mesh> originalMesh,
    int levelsOfSubdivision) {
  g_subdivisionSurfaceMeshActual.reset(new Mesh(*g_subdivisionSurfaceMeshOriginal));
  for (int i = 0; i < levelsOfSubdivision; i++) {
    applySubdivision(g_subdivisionSurfaceMeshActual);
  }
}

/**
 * Destructively applies a single Catmull-Clark subdivision to the provided mesh.
 *
 * These must be done in the order:
 *   1. All faces
 *   2. All edges
 *   3. All vertices
 */
static shared_ptr<Mesh> applySubdivision(shared_ptr<Mesh> actualMesh) {
  applyFaceSubdivisions(actualMesh);
  applyEdgeSubdivisions(actualMesh);
  applyVertexSubdivisions(actualMesh);
}

/**
 * Computes and applies the new face subdivisions to the provided mesh.
 */
static shared_ptr<Mesh> applyFaceSubdivisions(shared_ptr<Mesh> actualMesh) {
  for (int i = 0; i < actualMesh->getNumFaces(); i++) {
    Mesh::Face f = actualMesh->getFace(i);
    actualMesh->setNewFaceVertex(f, getFaceSubdivisionVertex(f));
  }
}

/**
 * face_vertex = 1/(# near face vertices) * (sum of near face vertices)
 */
static Cvec3 getFaceSubdivisionVertex(Mesh::Face f) {
  int numNearFaceVertices = f.getNumVertices();
  Cvec3 sumOfNearFaceVertices = Cvec3();
  for (int i = 0; i < numNearFaceVertices; i++) {
    sumOfNearFaceVertices += f.getVertex(i).getPosition();
  }
  return sumOfNearFaceVertices * (1.0 / numNearFaceVertices);
}

/**
 * Computes and applies the new edge subdivisions to the provided mesh.
 */
static shared_ptr<Mesh> applyEdgeSubdivisions(shared_ptr<Mesh> actualMesh) {
  for (int i = 0; i < actualMesh->getNumEdges(); i++) {
    Mesh::Edge e = actualMesh->getEdge(i);
    actualMesh->setNewEdgeVertex(e, getEdgeSubdivisionVertex(e, actualMesh));
  }
}

/**
 * edge_vertex = 1/4 * (end vertex 1 + end vertex 2 + edge face vertex 1 + edge face vertex 2)
 */
static Cvec3 getEdgeSubdivisionVertex(Mesh::Edge e, shared_ptr<Mesh> actualMesh) {
  return (
    e.getVertex(0).getPosition() +
    e.getVertex(1).getPosition() +
    actualMesh->getNewFaceVertex(e.getFace(0)) +
    actualMesh->getNewFaceVertex(e.getFace(1))
  ) * 0.25;
}

/**
 * Computes and applies the new vertex subdivisions to the provided mesh.
 */
static shared_ptr<Mesh> applyVertexSubdivisions(shared_ptr<Mesh> actualMesh) {

}

/**
 * vertex_vertex =
 *   (current vertex vertex) * (# near vertex vertices - 2) / (# near vertex vertices) +
 *   1/((# near vertex vertices)^2) * (sum of near vertex vertices) +
 *   1/((# near vertex vertices)^2) * (sum of near face vertices)
 */
static Cvec3 getVertexSubdivisionVertex(???) {

}

static void initSubdivisionSurface() {
  g_subdivisionSurfaceMeshOriginal.reset(new Mesh());
  g_subdivisionSurfaceMeshOriginal->load(g_subdivisionSurfaceFilename.c_str());
  g_subdivisionSurfaceMeshActual.reset(new Mesh(*g_subdivisionSurfaceMeshOriginal));

  updateMeshNormals(g_subdivisionSurfaceMeshActual);
  vector<VertexPN> verticies = getGeometryVertices(g_subdivisionSurfaceMeshActual);

  g_subdivisionSurface.reset(new SimpleGeometryPN());
  g_subdivisionSurface->upload(&verticies[0], verticies.size());

  animateSubdivisionSurfaceCallback(0);
}

static void initSphere() {
  const int slices = 25;
  const int stacks = 25;

  int ibLen, vbLen;
  getSphereVbIbLen(slices, stacks, vbLen, ibLen);

  /* Temporary storage for sphere Geometry */
  vector<VertexPNTBX> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeSphere(1, slices, stacks, vtx.begin(), idx.begin());
  g_sphere.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vtx.size(), idx.size()));
}

/** Takes a projection matrix and sends it to the the shaders */
inline void sendProjectionMatrix(Uniforms& uniforms, const Matrix4& projMatrix) {
  uniforms.put("uProjMatrix", projMatrix);
}

/** Update g_frustFovY from g_frustMinFov, g_windowWidth, and g_windowHeight */
static void updateFrustFovY() {
  if (g_windowWidth >= g_windowHeight)
    g_frustFovY = g_frustMinFov;
  else {
    const double RAD_PER_DEG = 0.5 * CS175_PI/180;
    g_frustFovY = atan2(sin(g_frustMinFov * RAD_PER_DEG) * g_windowHeight / g_windowWidth, cos(g_frustMinFov * RAD_PER_DEG)) / RAD_PER_DEG;
  }
}

static Matrix4 makeProjectionMatrix() {
  return Matrix4::makeProjection(
           g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
           g_frustNear, g_frustFar);
}

/**
 * - If we're manipulating a cube and the eye is the sky, this should be the
 *   cube-sky frame.
 * - If we're manipulating cube i and eye is cube j, this should be the
 *   cube i-cube j frame.
 * - If we're manipulating the sky camera and eye is the sky, we have two
 *   viable frames, and pressing 'm' switches between them:
 *   - World-sky frame (like orbiting around the world)
 *   - Sky-sky frame (like moving your head)
 */
static void setWrtFrame() {
  if (g_currentPickedRbtNode == g_skyNode) { /* manipulating sky */
    if (g_currentView == g_skyNode) { /* view is sky */
      if (g_skyViewChoice == WORLD_SKY) {
        g_aFrame = linFact(g_skyNode->getRbt()); /* world-sky */
      } else {
        g_aFrame = g_skyNode->getRbt(); /* sky-sky */
      }
    }
  } else {
    if (g_currentView == g_skyNode) { /* view is sky */
      g_aFrame = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) *
        transFact(getPathAccumRbt(g_world, g_currentPickedRbtNode)) * linFact(getPathAccumRbt(g_world, g_skyNode));
    } else { /* view is cube */
      g_aFrame = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) * getPathAccumRbt(g_world, g_currentPickedRbtNode);
    }
  }
}

static RigTForm getEyeRBT() {
  return getPathAccumRbt(g_world, g_currentView);
}

static bool nonEgoCubeManipulation() {
  /* manipulating cube, and view not from that cube */
  return g_currentPickedRbtNode != g_skyNode && g_currentView != g_currentPickedRbtNode;
}

static bool useArcball() {
  return (g_currentPickedRbtNode == g_skyNode && g_skyViewChoice == WORLD_SKY) || nonEgoCubeManipulation();
}

static bool worldSkyManipulation() {
  /* manipulating sky camera, while eye is sky camera, and while in world-sky mode */
  return g_currentPickedRbtNode == g_skyNode && g_currentView == g_skyNode && g_skyViewChoice == WORLD_SKY;
}

static void drawStuff(bool picking) {
  Uniforms uniforms;

  /* need to call this here so that the arcball moves when we change the object we're manipulating */
  setWrtFrame();

  /* Build & send proj. matrix to vshader */
  const Matrix4 projmat = makeProjectionMatrix();
  sendProjectionMatrix(uniforms, projmat);

  /* Set the camera view */
  RigTForm eyeRbt = getEyeRBT();
  const RigTForm invEyeRbt = inv(eyeRbt);


  /* Get light positions in world coordinates */
  const Cvec3 light1 = getPathAccumRbt(g_world, g_light1Node).getTranslation();
  const Cvec3 light2 = getPathAccumRbt(g_world, g_light2Node).getTranslation();

  /* Get light positions in eye coordinates, and hand them to uniforms */
  uniforms.put("uLight", Cvec3(invEyeRbt * Cvec4(light1, 1)));
  uniforms.put("uLight2", Cvec3(invEyeRbt * Cvec4(light2, 1)));

  if (!picking) {
    Drawer drawer(invEyeRbt, uniforms);
    g_world->accept(drawer);

    RigTForm sphereTarget;
    if (g_currentPickedRbtNode == g_skyNode) {
      if (g_skyViewChoice == WORLD_SKY) {
        sphereTarget = inv(RigTForm());
      } else {
        sphereTarget = eyeRbt;
      }
    } else {
      sphereTarget = getPathAccumRbt(g_world, g_currentPickedRbtNode);
    }

    /* don't update g_arcballScale if we're translating in the z direction */
    if (!g_mouseMClickButton && !(g_mouseLClickButton && g_mouseRClickButton) && useArcball()) {
      g_arcballScale = getScreenToEyeScale(
        (inv(eyeRbt) * sphereTarget).getTranslation()[2],
        g_frustFovY,
        g_windowHeight
      );
    }

    const Matrix4 scale = Matrix4::makeScale(g_arcballScale * g_arcballScreenRadius);
    Matrix4 MVM = rigTFormToMatrix(invEyeRbt * sphereTarget) * scale;
    Matrix4 NMVM = normalMatrix(MVM);
    sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));
    g_arcballMat->draw(*g_sphere, uniforms);
  } else {
    Picker picker(invEyeRbt, uniforms);

    g_overridingMaterial = g_pickingMat;
    g_world->accept(picker);
    g_overridingMaterial.reset();

    glFlush();
    g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_mouseClickX, g_mouseClickY);
    if (g_currentPickedRbtNode == g_groundNode || g_currentPickedRbtNode == NULL)
      g_currentPickedRbtNode = g_skyNode;
  }
}

static void display() {
  /* Clear framebuffer color & depth */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawStuff(g_picking);

  /* Show the back buffer (where we rendered stuff) */
  if (!g_picking) {
    glutSwapBuffers();
  }

  checkGlErrors();
}

static void pick() {
  /* We need to set the clear color to black, for pick
   * rendering, so let's save the clear color */
  GLdouble clearColor[4];
  glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawStuff(true);

  /* Uncomment below and comment out the glutPostRedisplay in mouse(...)
   * call back to see result of the pick rendering pass */
  // glutSwapBuffers();

  /* Now set back the clear color */
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  checkGlErrors();
}

static void reshape(const int w, const int h) {
  g_windowWidth = w;
  g_windowHeight = h;
  glViewport(0, 0, w, h);
  cerr << "Size of window is now " << w << "x" << h << endl;

  g_arcballScreenRadius = 0.25 * min(g_windowWidth, g_windowHeight);

  updateFrustFovY();
  glutPostRedisplay();
}

/**
 * Return a RigTForm representing an arcball rotation from the point where the mouse was clicked to its current location.
 *
 * @param  x Curent x coord of the mouse, in OpenGL coordinates (NOT raw GLUT coords)
 * @param  y Curent y coord of the mouse, in OpenGL coordinates (NOT raw GLUT coords)
 * @return   A RigTForm representing the arcball rotation
 */
static RigTForm getArcballRotation(const int x, const int y) {
  const RigTForm eyeRbt = getEyeRBT();
  // const RigTForm object = g_currentPickedRbtNode->getRbt();
  const RigTForm object = getPathAccumRbt(g_world, g_currentPickedRbtNode);

  const bool world_sky_manipulation = worldSkyManipulation();

  Cvec2 sphereOnScreenCoords;
  if (world_sky_manipulation) {
    /* use the screen center */
    sphereOnScreenCoords = Cvec2((g_windowWidth - 1) / 2.0, (g_windowHeight - 1) / 2.0);
  } else {
    sphereOnScreenCoords = getScreenSpaceCoord(
      (inv(eyeRbt) * object).getTranslation(),
      makeProjectionMatrix(),
      g_frustNear,
      g_frustFovY,
      g_windowWidth,
      g_windowHeight
    );
  }

  const Cvec3 sphere_center = Cvec3(sphereOnScreenCoords, 0);
  const Cvec3 p1 = Cvec3(g_mouseClickX, g_mouseClickY, 0) - sphere_center;
  const Cvec3 p2 = Cvec3(x, y, 0) - sphere_center;

  const Cvec3 v1 = normalize(Cvec3(p1[0], p1[1],
    sqrt(max(0.0, pow(g_arcballScreenRadius, 2) - pow(p1[0], 2) - pow(p1[1], 2)))));
  const Cvec3 v2 = normalize(Cvec3(p2[0], p2[1],
    sqrt(max(0.0, pow(g_arcballScreenRadius, 2) - pow(p2[0], 2) - pow(p2[1], 2)))));

  /* If we're manipulating the sky camera, the eye is the sky camera, and
   * we're in world-sky mode, negate the rotation so it behaves intuituvely. */
  if (world_sky_manipulation) {
    /* reversing dot product to accomplish the negation described above */
    return RigTForm(Quat(0, v1 * -1.0) * Quat(0, v2));
  } else {
    return RigTForm(Quat(0, v2) * Quat(0, v1 * -1.0));
  }
}

static void motion(const int x, const int y) {
  /* don't allow the sky frame to be manipulated if we're in a cube view */
  if (g_currentView != g_skyNode && g_currentPickedRbtNode == g_skyNode) return;

  const double curr_x = x;
  const double curr_y = g_windowHeight - y - 1;
  const double raw_dx = curr_x - g_mouseClickX;
  const double raw_dy = curr_y - g_mouseClickY;

  /* invert dx and/or dy depending on the situation */
  double dx_t, dx_r, dy_t, dy_r;
  if (nonEgoCubeManipulation()) {
    dx_t = raw_dx; dx_r = raw_dx;
    dy_t = raw_dy; dy_r = raw_dy;
  } else if (worldSkyManipulation()) {
    dx_t = -raw_dx; dx_r = -raw_dx;
    dy_t = -raw_dy; dy_r = -raw_dy;
  } else {
    dx_t = raw_dx; dx_r = -raw_dx;
    dy_t = raw_dy; dy_r = -raw_dy;
  }

  /* Use arcball is either of the following conditions are true:
   *
   * 1. manipulating sky camera w.r.t world-sky frame
   * 2. manipulating cube, and view not from that cube
   *
   * Otherwise use standard dx and dy rotation.
   */
  const bool use_arcball = useArcball();

  /* use g_arcballScale to scale translation, unless we're not using arcball */
  double translateFactor;
  if (use_arcball) {
    translateFactor = g_arcballScale;
  } else {
    translateFactor = 0.01;
  }

  /* Setting the auxiliary frame here because it needs to be updated whenever a
   * translation occurs; this also covers all other cases for which it needs to
   * be updated, including view and object manipulation changes. */
  setWrtFrame();

  RigTForm m;
  /* Left button down? */
  if (g_mouseLClickButton && !g_mouseRClickButton) {
    if (use_arcball) {
      m = getArcballRotation(curr_x, curr_y);
    } else {
      m = RigTForm(Quat::makeXRotation(-dy_r) * Quat::makeYRotation(dx_r));
    }
  }
  /* Right button down? */
  else if (g_mouseRClickButton && !g_mouseLClickButton) {
    m = RigTForm(Cvec3(dx_t, dy_t, 0) * translateFactor);
  }
  /* Middle or (left and right) button down? */
  else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {
    m = RigTForm(Cvec3(0, 0, -dy_t) * translateFactor);
  }

  /* apply the transformation */
  if (g_mouseClickDown) {
    m = g_aFrame * m * inv(g_aFrame);

    g_currentPickedRbtNode->setRbt(m * g_currentPickedRbtNode->getRbt());
  }

  g_mouseClickX = curr_x;
  g_mouseClickY = curr_y;

  /* Always redraw if we changed the scene */
  glutPostRedisplay();
}

static void mouse(const int button, const int state, const int x, const int y) {
  g_mouseClickX = x;
  /* Conversion from GLUT window-coordinate-system to OpenGL window-coordinate-system */
  g_mouseClickY = g_windowHeight - y - 1;

  g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
  g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
  g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

  g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
  g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
  g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

  g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;

  if (g_picking && g_mouseLClickButton && !g_mouseRClickButton) {
    pick();
    disablePickingMode();
  }
  glutPostRedisplay();
}

static void cycleSkyAChoice() {
  /* Only allow this to be toggled if we're manipulating the sky while using the sky camera */
  if (g_currentPickedRbtNode == g_skyNode && g_currentView == g_skyNode) {
    g_skyViewChoice = (g_skyViewChoice + 1) % 2;
    if (g_skyViewChoice == WORLD_SKY) {
      cout << "Setting aux frame to world-sky" << endl;
    } else {
      cout << "Setting aux frame to sky-sky" << endl;
    }
  } else {
    cout << "Unable to change sky manipulation mode while in this view." << endl;
  }
}

/**
 * Toggles the frame to be used with the sky camera.
 */
static void toggleEyeMode() {
  g_currentViewIndex = (g_currentViewIndex + 1) % g_numberOfViews;
  if (g_currentViewIndex == 0) {
    cout << "Using sky view" << endl;
  } else {
    cout << "Using object " << g_currentViewIndex << " view" << endl;
  }

  /* actually set the current view */
  switch (g_currentViewIndex) {
    case 0:
      g_currentView = g_skyNode;
      break;
    case 1:
      g_currentView = g_robot1Node;
      break;
    case 2:
      g_currentView = g_robot2Node;
      break;
  }
}

static void enablePickingMode() {
  g_picking = true;
  cout << "Picking mode is on" << endl;
}

static void disablePickingMode() {
  g_picking = false;
  cout << "Picking mode is off" << endl;
}

/**
 * Toggles the animation on and off.
 */
static void toggleAnimation() {
  if (!g_animationPlaying) {
    if (g_script.getNumberOfKeyframes() < 4) {
      cout << "[Warning] Need at least 4 keyframes defined to play the " <<
        "animation. Had " << g_script.getNumberOfKeyframes() << "." << endl;
      return;
    }

    /* Start playing the animation if it's currently off. */
    g_animationPlaying = true;
    g_script.goToBeginning();
    animateTimerCallback(0);
  } else {
    /* Stop playing the animation if it was being played. */
    g_animationPlaying = false;
  }
}

/**
 * Given t in the range [0, n], perform interpolation and draw the scene for the
 * particular t.
 *
 * Sets g_animationPlaying to false when the animation is over.
 */
void interpolateAndDisplay(float t) {
  int firstFrame = floor(t);
  if (firstFrame != g_lastAnimatedFrame) {
    g_lastAnimatedFrame = firstFrame;
    g_script.advanceCurrentFrame();
  }

  if (!g_script.canAnimate()) {
    g_animationPlaying = false;
  } else {
    const float alpha = t - floor(t);
    g_script.interpolate(alpha);
    glutPostRedisplay();
  }
}

/**
 * Interpret "ms" as milliseconds into the animation.
 */
static void animateTimerCallback(int ms) {
  float t = (float)ms/(float)g_msBetweenKeyFrames;
  /* This function will set g_animationPlaying to false when we're done. */
  interpolateAndDisplay(t);

  if (g_animationPlaying) {
    glutTimerFunc(
      1000 / g_animateFramesPerSecond,
      animateTimerCallback,
      ms + 1000 / g_animateFramesPerSecond
    );
  }
}

/**
 * Animates the subdivision surface by flexing the vertices.
 */
void animateSubdivisionSurface(float t) {
  updateMeshVertices(g_subdivisionSurfaceMeshActual, g_subdivisionSurfaceMeshOriginal, t);
  updateMeshNormals(g_subdivisionSurfaceMeshActual);
  vector<VertexPN> verticies = getGeometryVertices(g_subdivisionSurfaceMeshActual);
  g_subdivisionSurface->upload(&verticies[0], verticies.size());
  glutPostRedisplay();
}

/**
 * The GLUT timer callback used to control the subdivision surface animation.
 */
void animateSubdivisionSurfaceCallback(int ms) {
  animateSubdivisionSurface((float)ms);

  glutTimerFunc(
    10,
    animateSubdivisionSurfaceCallback,
    ms + 10
  );
}


static void keyboard(const unsigned char key, const int x, const int y) {
  switch (key) {
    case ESCAPE_KEY:
      exit(0);
    case 'h':
      cout << " ============== H E L P ==============\n\n"
      << "h\t\thelp menu\n"
      << "s\t\tsave screenshot\n"
      << "f\t\tToggle flat shading on/off.\n"
      << "o\t\tCycle object to edit\n"
      << "v\t\tCycle view\n"
      << "drag left mouse to rotate\n" << endl;
      break;
    case 's':
      glFlush();
      writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
      break;
    case 'v':
      toggleEyeMode();
      break;
    case 'm':
      cycleSkyAChoice();
      break;
    case 'p':
      enablePickingMode();
      break;
    case SPACE_KEY:
      g_script.showCurrentFrameInScene();
      break;
    case 'u':
      g_script.replaceCurrentFrameFromScene(g_world);
      break;
    case '>':
      g_script.advanceCurrentFrame();
      break;
    case '<':
      g_script.regressCurrentFrame();
      break;
    case 'd':
      g_script.deleteCurrentFrame();
      break;
    case 'n':
      g_script.createNewFrameFromSceneAfterCurrentFrame(g_world);
      break;
    case 'i':
      g_script = Script::loadScriptFromFile(DEFAULT_SCRIPT_FILENAME, g_world);
      break;
    case 'w':
      g_script.writeScriptToFile(DEFAULT_SCRIPT_FILENAME);
      break;
    case 'q':
      getEyeRBT().getRotation().raisedTo(0.6);
      break;
    case 'y':
      toggleAnimation();
      break;
    case '+':
      g_msBetweenKeyFrames = max(100, g_msBetweenKeyFrames - 100);
      cout << g_msBetweenKeyFrames << "ms between keyframes" << endl;
      break;
    case '-':
      g_msBetweenKeyFrames = min(10000, g_msBetweenKeyFrames + 100);
      cout << g_msBetweenKeyFrames << "ms between keyframes" << endl;
      break;
  }
  glutPostRedisplay();
}

static void initGlutState(int argc, char * argv[]) {
  /* Initialize Glut based on cmd-line args */
  glutInit(&argc, argv);
  /* RGBA pixel channels and double buffering */
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  /* Create a window */
  glutInitWindowSize(g_windowWidth, g_windowHeight);
  /* Title the window */
  glutCreateWindow("Assignment 6.5");

  /* Display rendering callback */
  glutDisplayFunc(display);
  /* Window reshape callback */
  glutReshapeFunc(reshape);
  /* Mouse movement callback */
  glutMotionFunc(motion);
  /* Mouse click callback */
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
}

static void initGLState() {
  glClearColor(128./255., 200./255., 255./255., 0.);
  glClearDepth(0.);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);
  glReadBuffer(GL_BACK);
  if (!g_Gl2Compatible)
    glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initMaterials() {
  /* Create some prototype materials */
  Material diffuse("./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader");
  Material solid("./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader");
  Material specular("./shaders/basic-gl3.vshader", "./shaders/specular-gl3.fshader");

  /* copy diffuse prototype and set red color */
  g_redDiffuseMat.reset(new Material(diffuse));
  g_redDiffuseMat->getUniforms().put("uColor", Cvec3f(1, 0, 0));

  /* copy diffuse prototype and set blue color */
  g_blueDiffuseMat.reset(new Material(diffuse));
  g_blueDiffuseMat->getUniforms().put("uColor", Cvec3f(0, 0, 1));

  /* normal mapping material */
  g_bumpFloorMat.reset(new Material("./shaders/normal-gl3.vshader", "./shaders/normal-gl3.fshader"));
  g_bumpFloorMat->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("Fieldstone.ppm", true)));
  g_bumpFloorMat->getUniforms().put("uTexNormal", shared_ptr<ImageTexture>(new ImageTexture("FieldstoneNormal.ppm", false)));

  /* copy solid prototype, and set to wireframed rendering */
  g_arcballMat.reset(new Material(solid));
  g_arcballMat->getUniforms().put("uColor", Cvec3f(0.27f, 0.82f, 0.35f));
  g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);

  /* copy solid prototype, and set to color white */
  g_lightMat.reset(new Material(solid));
  g_lightMat->getUniforms().put("uColor", Cvec3f(1, 1, 1));

  g_specularMat.reset(new Material(specular));
  g_specularMat->getUniforms().put("uColor", Cvec3f(1, 0, 0.72));

  /* pick shader */
  g_pickingMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"));
};

static void initGeometry() {
  initGround();
  initCubes();
  initSubdivisionSurface();
  initSphere();
}

static void constructRobot(shared_ptr<SgTransformNode> base, shared_ptr<Material> material) {

  const double ARM_LEN = 0.7,
               ARM_THICK = 0.25,
               TORSO_LEN = 1.5,
               TORSO_THICK = 0.25,
               TORSO_WIDTH = 1,
               HEAD_RADIUS = 0.35;
  const int NUM_JOINTS = 10,
            NUM_SHAPES = 10;

  struct JointDesc {
    int parent;
    float x, y, z;
  };

  JointDesc jointDesc[NUM_JOINTS] = {
    {-1}, // torso
    {0,  TORSO_WIDTH/2, TORSO_LEN/2, 0}, // upper right arm
    {1,  ARM_LEN, 0, 0}, // lower right arm
    {0,  -TORSO_WIDTH/2, TORSO_LEN/2, 0}, // upper left arm
    {3, -ARM_LEN, 0, 0}, // lower left arm
    {0, 0, TORSO_LEN/2, 0}, // noggin
    {0, TORSO_WIDTH/2-ARM_THICK/2, -TORSO_LEN/2, 0}, // upper right leg
    {6, 0, -ARM_LEN, 0}, // lower right leg
    {0, -(TORSO_WIDTH/2-ARM_THICK/2), -TORSO_LEN/2, 0}, // upper left leg
    {8, 0, -ARM_LEN, 0}, // lower left leg
  };

  struct ShapeDesc {
    int parentJointId;
    float x, y, z, sx, sy, sz;
    shared_ptr<Geometry> geometry;
  };

  ShapeDesc shapeDesc[NUM_SHAPES] = {
    {0, 0,         0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, g_cube}, // torso

    {1,  ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // upper right arm
    {2,  ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK * 0.7, ARM_THICK, g_cube}, // lower right arm
    {3, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // upper left arm
    {4, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK * 0.7, ARM_THICK, g_cube}, // lower left arm

    {5, 0,  HEAD_RADIUS, 0, HEAD_RADIUS, HEAD_RADIUS, HEAD_RADIUS, g_sphere}, // noggin

    {6, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, g_cube}, // upper right leg
    {7, 0, -ARM_LEN/2, 0, ARM_THICK * 0.7, ARM_LEN, ARM_THICK, g_cube}, // lower right leg
    {8, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, g_cube}, // upper left leg
    {9, 0, -ARM_LEN/2, 0, ARM_THICK * 0.7, ARM_LEN, ARM_THICK, g_cube}, // lower left leg
  };

  shared_ptr<SgTransformNode> jointNodes[NUM_JOINTS];

  for (int i = 0; i < NUM_JOINTS; ++i) {
    if (jointDesc[i].parent == -1)
      jointNodes[i] = base;
    else {
      jointNodes[i].reset(new SgRbtNode(RigTForm(Cvec3(jointDesc[i].x, jointDesc[i].y, jointDesc[i].z))));
      jointNodes[jointDesc[i].parent]->addChild(jointNodes[i]);
    }
  }
  for (int i = 0; i < NUM_SHAPES; ++i) {
    shared_ptr<MyShapeNode> shape(
      new MyShapeNode(
        shapeDesc[i].geometry,
        material,
        Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
        Cvec3(0, 0, 0),
        Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz))
      );
    jointNodes[shapeDesc[i].parentJointId]->addChild(shape);
  }
}

static void initScene() {
  g_world.reset(new SgRootNode());

  g_skyNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, 0.25, 4.0))));
  g_aFrame = linFact(g_skyNode->getRbt());
  g_currentPickedRbtNode = g_skyNode;
  g_currentView = g_skyNode;

  g_groundNode.reset(new SgRbtNode());
  g_groundNode->addChild(shared_ptr<MyShapeNode>(
                          new MyShapeNode(g_ground, g_bumpFloorMat, Cvec3(0, g_groundY, 0))));

  /* define two light positions in world space */
  const Cvec3 light1(4.5, 3.0, 6.5), light2(-4.5, 1.5, -6.5);

  g_light1Node.reset(new SgRbtNode(RigTForm(light1)));
  g_light1Node->addChild(shared_ptr<MyShapeNode>(
                           new MyShapeNode(g_sphere, g_lightMat, Cvec3(0, 0, 0))));

  g_light2Node.reset(new SgRbtNode(RigTForm(light2)));
  g_light2Node->addChild(shared_ptr<MyShapeNode>(
                           new MyShapeNode(g_sphere, g_lightMat, Cvec3(0, 0, 0))));

  g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
  g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));

  constructRobot(g_robot1Node, g_redDiffuseMat); // a Red robot
  constructRobot(g_robot2Node, g_blueDiffuseMat); // a Blue robot

  g_meshNode.reset(new SgRbtNode(RigTForm()));
  g_meshNode->addChild(shared_ptr<MyShapeNode>(
                           new MyShapeNode(g_subdivisionSurface, g_specularMat, Cvec3(0, 0, 0))));

  g_world->addChild(g_skyNode);
  g_world->addChild(g_groundNode);
  g_world->addChild(g_light1Node);
  g_world->addChild(g_light2Node);
  g_world->addChild(g_robot1Node);
  g_world->addChild(g_robot2Node);
  g_world->addChild(g_meshNode);
}

int main(int argc, char * argv[]) {
  try {
    initGlutState(argc,argv);

    /* Load the OpenGL extensions */
    glewInit();

    cout << (g_Gl2Compatible ? "Will use OpenGL 2.x / GLSL 1.0" : "Will use OpenGL 3.x / GLSL 1.3") << endl;
    if ((!g_Gl2Compatible) && !GLEW_VERSION_3_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.3");
    else if (g_Gl2Compatible && !GLEW_VERSION_2_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.0");

    initGLState();
    initMaterials();
    initGeometry();
    initScene();

    glutMainLoop();
    return 0;
  }
  catch (const runtime_error& e) {
    cout << "Exception caught: " << e.what() << endl;
    return -1;
  }
}
