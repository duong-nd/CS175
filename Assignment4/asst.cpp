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
#include "headers/ppm.h"
#include "headers/glsupport.h"
#include "headers/arcball.h"
#include "headers/quat.h"
#include "headers/rigtform.h"

#include "headers/asstcommon.h"
#include "headers/scenegraph.h"
#include "headers/drawer.h"
#include "headers/picker.h"

#define ESCAPE_KEY 27

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
static const int PICKING_SHADER = 2;
static const int g_numShaders = 3;
static const char * const g_shaderFiles[g_numShaders][2] = {
  {"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = {
  {"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/pick-gl2.fshader"}
};
/** Our global shader states */
static vector<shared_ptr<ShaderState> > g_shaderStates;

/** GEOMETRY */

/** Macro used to obtain relative offset of a field within a struct */
#define FIELD_OFFSET(StructType, field) &(((StructType *)0)->field)

/** A vertex with floating point position and normal */
struct VertexPN {
  Cvec3f p, n;

  VertexPN() {}
  VertexPN(float x, float y, float z,
           float nx, float ny, float nz)
    : p(x,y,z), n(nx, ny, nz)
  {}

  /**
   * Define copy constructor and assignment operator from GenericVertex so we
   * can use make* functions from geometrymaker.h.
   */
  VertexPN(const GenericVertex& v) {
    *this = v;
  }

  VertexPN& operator = (const GenericVertex& v) {
    p = v.pos;
    n = v.normal;
    return *this;
  }
};

struct Geometry {
  GlBufferObject vbo, ibo;
  int vboLen, iboLen;

  Geometry(VertexPN *vtx, unsigned short *idx, int vboLen, int iboLen) {
    this->vboLen = vboLen;
    this->iboLen = iboLen;

    /* Now create the VBO and IBO */
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vboLen, vtx, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * iboLen, idx, GL_STATIC_DRAW);
  }

  void draw(const ShaderState& curSS) {
    /* Enable the attributes used by our shader */
    safe_glEnableVertexAttribArray(curSS.h_aPosition);
    safe_glEnableVertexAttribArray(curSS.h_aNormal);

    /* Bind vbo */
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    safe_glVertexAttribPointer(curSS.h_aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, p));
    safe_glVertexAttribPointer(curSS.h_aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, n));

    /* Bind ibo */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    /* Draw! */
    glDrawElements(GL_TRIANGLES, iboLen, GL_UNSIGNED_SHORT, 0);

    /* Disable the attributes used by our shader */
    safe_glDisableVertexAttribArray(curSS.h_aPosition);
    safe_glDisableVertexAttribArray(curSS.h_aNormal);
  }
};

typedef SgGeometryShapeNode<Geometry> MyShapeNode;


/**
 * Vertex buffer and index buffer associated with the ground and cube geometry
 */
static shared_ptr<Geometry> g_ground, g_cube, g_sphere;
static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode;

/** SCENE */

/** Define two lights positions in world space */
static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);
static const int g_numObjects = 2;
static int g_currentViewIndex = 0;
static shared_ptr<SgRbtNode> g_currentView; /* set to g_skyNode in initScene() */
static Cvec3f g_objectColors[g_numObjects] = {
  Cvec3f(1, 0, 0),
  Cvec3f(0, 1, 0)
};

static const Cvec3f g_arcballColor = Cvec3f(0, 0.47, 1);
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
 *
 * 0 = world-sky
 * 1 = sky-sky
 */
static int g_skyViewChoice = 0;

/** METHOD PROTOTYPES *********************************************************/
static void enablePickingMode();
static void disablePickingMode();

/** METHODS *******************************************************************/

static void initGround() {
  /**
   * An x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
   */
  VertexPN vtx[4] = {
    VertexPN(-g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
    VertexPN(-g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
  };
  unsigned short idx[] = {0, 1, 2, 0, 2, 3};
  g_ground.reset(new Geometry(&vtx[0], &idx[0], 4, 6));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  /* Temporary storage for cube geometry */
  vector<VertexPN> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeCube(1, vtx.begin(), idx.begin());
  g_cube.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initSpheres() {
  const int slices = 25;
  const int stacks = 25;

  int ibLen, vbLen;
  getSphereVbIbLen(slices, stacks, vbLen, ibLen);

  /* Temporary storage for sphere geometry */
  vector<VertexPN> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeSphere(1, slices, stacks, vtx.begin(), idx.begin());
  g_sphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

/** Takes a projection matrix and send to the the shaders */
static void sendProjectionMatrix(const ShaderState& curSS, const Matrix4& projMatrix) {
  GLfloat glmatrix[16];
  /* Send projection matrix */
  projMatrix.writeToColumnMajorMatrix(glmatrix);
  safe_glUniformMatrix4fv(curSS.h_uProjMatrix, glmatrix);
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
      if (g_skyViewChoice == 0) {
        g_aFrame = linFact(g_skyNode->getRbt()); /* world-sky */
      } else {
        g_aFrame = g_skyNode->getRbt(); /* sky-sky */
      }
    }
  } else {
    if (g_currentView == g_skyNode) { /* view is sky */
      // g_aFrame = transFact(g_currentPickedRbtNode->getRbt()) * linFact(g_skyNode->getRbt());
      g_aFrame = transFact(getPathAccumRbt(g_world, g_currentPickedRbtNode)) * linFact(getPathAccumRbt(g_world, g_skyNode));
      g_aFrame = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) * g_aFrame;
    } else { /* view is cube */
      // TODO update this
      g_aFrame = transFact(g_currentPickedRbtNode->getRbt()) * linFact(g_currentPickedRbtNode->getRbt());
    }
  }
}

static RigTForm getEyeRBT() {
  switch (g_currentViewIndex) {
    case 0:
      return getPathAccumRbt(g_world, g_skyNode);
    case 1:
      return getPathAccumRbt(g_world, g_robot1Node);
    case 2:
      return getPathAccumRbt(g_world, g_robot2Node);
  }
}

static bool nonEgoCubeManipulation() {
  /* manipulating cube, and view not from that cube */
  return g_currentPickedRbtNode != g_skyNode && g_currentView != g_currentPickedRbtNode;
}

static bool useArcball() {
  return (g_currentPickedRbtNode == g_skyNode && g_skyViewChoice == 0) || nonEgoCubeManipulation();
}

static bool worldSkyManipulation() {
  /* manipulating sky camera, while eye is sky camera, and while in world-sky mode */
  return g_currentPickedRbtNode == g_skyNode && g_currentView == g_skyNode && g_skyViewChoice == 0;
}

static void drawStuff(const ShaderState& curSS, bool picking) {
  /* need to call this here so that the arcball moves when we change the object we're manipulating */
  setWrtFrame();

  /* Build & send proj. matrix to vshader */
  const Matrix4 projmat = makeProjectionMatrix();
  sendProjectionMatrix(curSS, projmat);

  /* Set the camera view */
  const RigTForm eyeRbt = getEyeRBT();
  const RigTForm invEyeRbt = inv(eyeRbt);

  /* g_light1 position in eye coordinates */
  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1));
  /* g_light2 position in eye coordinates */
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1));
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  if (!picking) {
    Drawer drawer(invEyeRbt, curSS);
    g_world->accept(drawer);

    RigTForm sphereTarget;
    if (g_currentPickedRbtNode == g_skyNode) {
      if (g_skyViewChoice == 0) {
        sphereTarget = inv(RigTForm());
      } else {
        sphereTarget = eyeRbt;
      }
    } else {
      // sphereTarget = g_aFrame;
      // sphereTarget = g_currentPickedRbtNode->getRbt();
      // sphereTarget = g_robot1Node->getRbt() * getPathAccumRbt(g_robot1Node, g_currentPickedRbtNode);
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

    /* draw wireframes */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const Matrix4 scale = Matrix4::makeScale(g_arcballScale * g_arcballScreenRadius);
    Matrix4 MVM = rigTFormToMatrix(invEyeRbt * sphereTarget) * scale;
    Matrix4 NMVM = normalMatrix(MVM);
    sendModelViewNormalMatrix(curSS, MVM, NMVM);
    safe_glUniform3f(curSS.h_uColor, g_arcballColor[0], g_arcballColor[1], g_arcballColor[2]);
    g_sphere->draw(curSS);

    /* draw filled */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // draw filled again
  } else {
    Picker picker(invEyeRbt, curSS);
    g_world->accept(picker);
    glFlush();
    g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_mouseClickX, g_mouseClickY);
    if (g_currentPickedRbtNode == g_groundNode || g_currentPickedRbtNode == NULL)
      // g_currentPickedRbtNode = shared_ptr<SgRbtNode>();   // set to NULL
      g_currentPickedRbtNode = g_skyNode;
  }
}

static void display() {
  glUseProgram(g_shaderStates[g_activeShader]->program);
  /* Clear framebuffer color & depth */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawStuff(*g_shaderStates[g_activeShader], g_picking);

  /* Show the back buffer (where we rendered stuff) */
  if (!g_picking) {
    glutSwapBuffers();
  }

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
    cout << "YOU FUCKING MOTHER FUCKER" << endl;
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

  cout << "Sphere on screen coords: " << sphereOnScreenCoords[0] << "," << sphereOnScreenCoords[1] << endl;

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
  if (g_currentView != g_skyNode && g_currentPickedRbtNode == g_skyNode) {
    cout << "-----" << endl;
    cout << (g_currentView != g_skyNode) << endl;
    cout << (g_currentPickedRbtNode == g_skyNode) << endl;
    cout << "-----" << endl;
    return;
  }
    

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

static void pick() {
  // We need to set the clear color to black, for pick rendering.
  // so let's save the clear color
  GLdouble clearColor[4];
  glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

  glClearColor(0, 0, 0, 0);

  // using PICKING_SHADER as the shader
  glUseProgram(g_shaderStates[PICKING_SHADER]->program);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawStuff(*g_shaderStates[PICKING_SHADER], true);

  // Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
  // to see result of the pick rendering pass
  // glutSwapBuffers();

  //Now set back the clear color
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  checkGlErrors();
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
    if (g_skyViewChoice == 0) {
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
  cout << "picking enabled" << endl;
}

static void disablePickingMode() {
  g_picking = false;
  cout << "picking disabled" << endl;
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
    case 'f':
      g_activeShader ^= 1;
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
  glutCreateWindow("Assignment 2");

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

static void initShaders() {
  g_shaderStates.resize(g_numShaders);
  for (int i = 0; i < g_numShaders; ++i) {
    if (g_Gl2Compatible)
      g_shaderStates[i].reset(new ShaderState(g_shaderFilesGl2[i][0], g_shaderFilesGl2[i][1]));
    else
      g_shaderStates[i].reset(new ShaderState(g_shaderFiles[i][0], g_shaderFiles[i][1]));
  }
}

static void initGeometry() {
  initGround();
  initCubes();
  initSpheres();
}

static void constructRobot(shared_ptr<SgTransformNode> base, const Cvec3& color) {

  const double ARM_LEN = 0.7,
               ARM_THICK = 0.25,
               TORSO_LEN = 1.5,
               TORSO_THICK = 0.25,
               TORSO_WIDTH = 1,
               HEAD_RADIUS = 0.5;
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
  };

  struct ShapeDesc {
    int parentJointId;
    float x, y, z, sx, sy, sz;
    shared_ptr<Geometry> geometry;
  };

  ShapeDesc shapeDesc[NUM_SHAPES] = {
    {0, 0,         0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, g_cube}, // torso
    {1, ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // upper right arm
    {2, ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // lower right arm
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
      new MyShapeNode(shapeDesc[i].geometry,
                      color,
                      Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
                      Cvec3(0, 0, 0),
                      Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz)));
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
                           new MyShapeNode(g_ground, Cvec3(0.1, 0.95, 0.1))));

  g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
  g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));

  constructRobot(g_robot1Node, Cvec3(1, 0, 0)); // a Red robot
  constructRobot(g_robot2Node, Cvec3(0, 0, 1)); // a Blue robot

  g_world->addChild(g_skyNode);
  g_world->addChild(g_groundNode);
  g_world->addChild(g_robot1Node);
  g_world->addChild(g_robot2Node);
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
    initShaders();
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
