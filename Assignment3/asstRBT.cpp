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

#define ESCAPE_KEY 27

/* For string, vector, iostream, and other standard C++ stuff */
using namespace std;
/* For shared_ptr */
using namespace tr1;

/** G L O B A L S *************************************************************/
static const bool g_Gl2Compatible = true;


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
static int g_originalMouseClickX, g_originalMouseClickY;
static int g_activeShader = 0;

struct ShaderState {
  GlProgram program;

  /** Handles to uniform variables */
  GLint h_uLight, h_uLight2;
  GLint h_uProjMatrix;
  GLint h_uModelViewMatrix;
  GLint h_uNormalMatrix;
  GLint h_uColor;

  /** Handles to vertex attributes */
  GLint h_aPosition;
  GLint h_aNormal;

  ShaderState(const char* vsfn, const char* fsfn) {
    readAndCompileShader(program, vsfn, fsfn);

    /** Short hand reference for the program */
    const GLuint h = program;

    /** Retrieve handles to uniform variables */
    h_uLight = safe_glGetUniformLocation(h, "uLight");
    h_uLight2 = safe_glGetUniformLocation(h, "uLight2");
    h_uProjMatrix = safe_glGetUniformLocation(h, "uProjMatrix");
    h_uModelViewMatrix = safe_glGetUniformLocation(h, "uModelViewMatrix");
    h_uNormalMatrix = safe_glGetUniformLocation(h, "uNormalMatrix");
    h_uColor = safe_glGetUniformLocation(h, "uColor");

    /** Retrieve handles to vertex attributes */
    h_aPosition = safe_glGetAttribLocation(h, "aPosition");
    h_aNormal = safe_glGetAttribLocation(h, "aNormal");

    if (!g_Gl2Compatible)
      glBindFragDataLocation(h, 0, "fragColor");
    checkGlErrors();
  }

};

static const int g_numShaders = 2;
static const char * const g_shaderFiles[g_numShaders][2] = {
  {"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = {
  {"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"}
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


/**
 * Vertex buffer and index buffer associated with the ground and cube geometry
 */
static shared_ptr<Geometry> g_ground, g_cube, g_cube2, g_sphere;

/** SCENE */

/** Define two lights positions in world space */
static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);
static RigTForm g_skyRbt = RigTForm(Cvec3(0.0, 0.25, 4.0));
static const int g_numObjects = 2;
static int g_currentViewIndex = 0;
static RigTForm g_objectRbt[g_numObjects] = {
  RigTForm(Cvec3(-1,0,0)),
  RigTForm(Cvec3(1,0,0))
};
static Cvec3f g_objectColors[g_numObjects] = {
  Cvec3f(1, 0, 0),
  Cvec3f(0, 1, 0)
};

static const Cvec3f g_arcballColor = Cvec3f(1, 0, 1);
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
 * Initialize it to use world-sky.
 */
static RigTForm g_aFrame = linFact(g_skyRbt);

/** Start with the sky camera as the object that's manipulated by the mouse */
static int g_objectBeingManipulated = 0;

/**
 * controls whether we use the world-sky or sky-sky frame
 * when modifying the sky while the using the sky camera
 *
 * 0 = world-sky
 * 1 = sky-sky
 */
static int g_skyViewChoice = 0;

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

  vector<VertexPN> vtx_2(vbLen);
  vector<unsigned short> idx_2(ibLen);
  makeCube(1, vtx_2.begin(), idx_2.begin());
  g_cube2.reset(new Geometry(&vtx_2[0], &idx_2[0], vbLen, ibLen));
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

/** Takes MVM and its normal matrix to the shaders */
static void sendModelViewNormalMatrix(const ShaderState& curSS, const Matrix4& MVM, const Matrix4& NMVM) {
  GLfloat glmatrix[16];
  /* Send MVM */
  MVM.writeToColumnMajorMatrix(glmatrix);
  safe_glUniformMatrix4fv(curSS.h_uModelViewMatrix, glmatrix);

  /* Send NMVM */
  NMVM.writeToColumnMajorMatrix(glmatrix);
  safe_glUniformMatrix4fv(curSS.h_uNormalMatrix, glmatrix);
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
  if (g_objectBeingManipulated == 0) { /* manipulating sky */
    if (g_currentViewIndex == 0) { /* view is sky */
      if (g_skyViewChoice == 0) {
        g_aFrame = linFact(g_skyRbt); /* world-sky */
      } else {
        g_aFrame = g_skyRbt; /* sky-sky */
      }
    }
  } else { /* manipulating cube */
    if (g_currentViewIndex == 0) { /* view is sky */
      g_aFrame = transFact(g_objectRbt[g_objectBeingManipulated - 1]) * linFact(g_skyRbt);
    } else { /* view is cube */
      g_aFrame = transFact(g_objectRbt[g_objectBeingManipulated - 1]) * linFact(g_objectRbt[g_currentViewIndex - 1]);
    }
  }
}

static void drawStuff() {
  // TODO need to call this here so that the arcball moves when we change the object we're manipulating; any way to call this function less?
  setWrtFrame();

  /* Short hand for current shader state */
  const ShaderState& curSS = *g_shaderStates[g_activeShader];

  /* Build & send proj. matrix to vshader */
  const Matrix4 projmat = makeProjectionMatrix();
  sendProjectionMatrix(curSS, projmat);

  /* Set the camera view */
  const RigTForm eyeRbt =
    (g_currentViewIndex == 0) ? g_skyRbt : g_objectRbt[g_currentViewIndex - 1];

  const RigTForm invEyeRbt = inv(eyeRbt);

  /* g_light1 position in eye coordinates */
  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1));
  /* g_light2 position in eye coordinates */
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1));
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  /* Now we'll draw the ground. */
  const RigTForm groundRbt = RigTForm(); // identity
  Matrix4 MVM = rigTFormToMatrix(invEyeRbt * groundRbt);
  Matrix4 NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, 0.1, 0.95, 0.1);
  g_ground->draw(curSS);

  /* Now we'll draw the cubes. */
  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[0]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[0][0], g_objectColors[0][1], g_objectColors[0][2]);
  g_cube->draw(curSS);

  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[1]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[1][0], g_objectColors[1][1], g_objectColors[1][2]);
  g_cube2->draw(curSS);

  RigTForm sphereTarget;
  if (g_objectBeingManipulated == 0) {
    if (g_skyViewChoice == 0) {
      sphereTarget = inv(RigTForm());
    } else {
      sphereTarget = eyeRbt;
    }
  } else {
    sphereTarget = g_objectRbt[g_objectBeingManipulated - 1];
  }

  /* don't update g_arcballScale if we're translating in the z direction */
  if (!g_mouseMClickButton && !(g_mouseLClickButton && g_mouseRClickButton)) {
    g_arcballScale = getScreenToEyeScale(
      (inv(eyeRbt) * sphereTarget).getTranslation()[2],
      g_frustFovY,
      g_windowHeight
    );
  }

  /* draw wireframes */
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  const Matrix4 scale = Matrix4::makeScale(g_arcballScale * g_arcballScreenRadius);
  MVM = rigTFormToMatrix(invEyeRbt * sphereTarget) * scale;
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_arcballColor[0], g_arcballColor[1], g_arcballColor[2]);
  g_sphere->draw(curSS);

  /* draw filled */
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // draw filled again
}

static void display() {
  glUseProgram(g_shaderStates[g_activeShader]->program);
  /* Clear framebuffer color & depth */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawStuff();

  /* Show the back buffer (where we rendered stuff) */
  glutSwapBuffers();

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

static void motion(const int x, const int y) {
  /* don't allow the sky frame to be manipulated if we're in a cube view */
  if (g_currentViewIndex != 0 && g_objectBeingManipulated == 0) return;

  const double raw_dx = x - g_mouseClickX;
  const double raw_dy = g_windowHeight - y - 1 - g_mouseClickY;

  const RigTForm eyeRbt =
      (g_currentViewIndex == 0) ? g_skyRbt : g_objectRbt[g_currentViewIndex - 1];
  RigTForm object = (g_objectBeingManipulated == 0) ? g_skyRbt : g_objectRbt[g_objectBeingManipulated - 1];
  Cvec2 sphereOnScreenCoords = getScreenSpaceCoord(
    (inv(eyeRbt) * object).getTranslation(),
    makeProjectionMatrix(),
    g_frustNear,
    g_frustFovY,
    g_windowWidth,
    g_windowHeight
  );
  double sphere_x = g_mouseClickX;
  double sphere_y = g_mouseClickY;
  double r = g_arcballScreenRadius;
  double c_x = sphereOnScreenCoords[0];
  double c_y = sphereOnScreenCoords[1];
  double sphere_z = sqrt(max(0.0, pow(r, 2) - pow(sphere_x-c_x, 2) - pow(sphere_y-c_y, 2)));
  cout << "Mouse click x: " << g_mouseClickX << endl;
  cout << "Mouse click y: " << g_mouseClickY << endl;
  cout << "object x: " << object.getTranslation()[0] << endl;
  cout << "object y: " << object.getTranslation()[1] << endl;
  cout << "object z: " << object.getTranslation()[2] << endl;
  cout << "c_x: " << c_x << endl;
  cout << "c_y: " << c_y << endl;
  cout << "z: " << sphere_z << endl;
  double original_sphere_z = sqrt(max(0.0, pow(r, 2) - pow(g_originalMouseClickX-c_x, 2) - pow(g_originalMouseClickY-c_y, 2)));
  Cvec3 v_1 = Cvec3(g_originalMouseClickX, g_originalMouseClickY, original_sphere_z);
  cout << "v_1: " << v_1[0] << ", " << v_1[1] << ", " << v_1[2] << endl;
  Cvec3 v_2 = Cvec3(sphere_x, sphere_y, sphere_z) - (inv(eyeRbt) * object).getTranslation();
  cout << "v_2: " << v_2[0] << ", " << v_2[1] << ", " << v_2[2] << endl;

  /* invert dx and/or dy depending on the situation */
  double dx_t, dx_r, dy_t, dy_r;
  if (g_objectBeingManipulated != 0 && g_currentViewIndex != g_objectBeingManipulated) {
    /* manipulating cube, and view not from that cube */
    dx_t = raw_dx; dx_r = raw_dx;
    dy_t = raw_dy; dy_r = raw_dy;
  } else if (g_objectBeingManipulated == 0 && g_currentViewIndex == 0 && g_skyViewChoice == 0) {
    /* manipulating sky camera, while eye is sky camera, and while in world-sky mode */
    dx_t = -raw_dx; dx_r = -raw_dx;
    dy_t = -raw_dy; dy_r = -raw_dy;
  } else {
    dx_t = raw_dx; dx_r = -raw_dx;
    dy_t = raw_dy; dy_r = -raw_dy;
  }

  /* Setting the auxiliary frame here because it needs to be updated whenever a
   * translation occurs; this also covers all other cases for which it needs to
   * be updated, including view and object manipulation changes. */
  setWrtFrame();

  RigTForm m;
  /* Left button down? */
  if (g_mouseLClickButton && !g_mouseRClickButton) {
    m = RigTForm(Quat::makeXRotation(-dy_r) * Quat::makeYRotation(dx_r));
  }
  /* Right button down? */
  else if (g_mouseRClickButton && !g_mouseLClickButton) {
    m = RigTForm(Cvec3(dx_t, dy_t, 0) * 0.01);
  }
  /* Middle or (left and right) button down? */
  else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {
    m = RigTForm(Cvec3(0, 0, -dy_t) * 0.01);
  }
  m = g_aFrame * m * inv(g_aFrame);

  if (g_mouseClickDown) {
    if (g_objectBeingManipulated == 0) {
      g_skyRbt = m * g_skyRbt;
    } else {
      g_objectRbt[g_objectBeingManipulated - 1] = m * g_objectRbt[g_objectBeingManipulated - 1];
    }
    /* Always redraw if we changed the scene */
    glutPostRedisplay();
  }

  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}


static void mouse(const int button, const int state, const int x, const int y) {
  g_originalMouseClickX = g_mouseClickX = x;
  /* Conversion from GLUT window-coordinate-system to OpenGL window-coordinate-system */
  g_originalMouseClickY = g_mouseClickY = g_windowHeight - y - 1;

  g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
  g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
  g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

  g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
  g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
  g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

  g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;

  glutPostRedisplay();
}

static void cycleSkyAChoice() {
  /* Only allow this to be toggled if we're manipulating the sky while using the sky camera */
  if (g_objectBeingManipulated == 0 && g_currentViewIndex == 0) {
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
}

/**
 * Cycles which object is being manipulated.
 */
static void cycleManipulation() {
  g_objectBeingManipulated = (g_objectBeingManipulated + 1) % g_numberOfViews;
  if (g_objectBeingManipulated == 0) {
    cout << "Manipulating sky frame" << endl;
  } else {
    cout << "Manipulating object " << g_objectBeingManipulated << endl;
  }
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
    case 'o':
      cycleManipulation();
      break;
    case 'm':
      cycleSkyAChoice();
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

    glutMainLoop();
    return 0;
  }
  catch (const runtime_error& e) {
    cout << "Exception caught: " << e.what() << endl;
    return -1;
  }
}
