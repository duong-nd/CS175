/*******************************************************************************
 *
 *   Harvard Computer Science
 *   CS 175: Computer Graphics
 *   Professor Steven Gortler
 *   Students: Michael {Tingley, Traver}
 *   Emails: {michaeltingley, mtraver}@college.harvard.edu
 *
 ******************************************************************************/

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#if __GNUG__
#   include <tr1/memory>
#endif

#include <GL/glew.h>
#ifdef __MAC__
#   define IS_MAC true
#   include <GLUT/glut.h>
#else
#   define IS_MAC false
#   include <GL/glut.h>
#endif

#include "ppm.h"
#include "glsupport.h"

using namespace std;      // for string, vector, iostream and other standard C++ stuff
using namespace std::tr1; // for shared_ptr

/* G L O B A L S **************************************************/

/* !!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!
 * Before you start working on this assignment, set the following variable properly
 * to indicate whether you want to use OpenGL 2.x with GLSL 1.0 or OpenGL 3.x+ with
 * GLSL 1.3.
 *
 * Set g_Gl2Compatible = true to use GLSL 1.0 and g_Gl2Compatible = false to use GLSL 1.3.
 * Make sure that your machine supports the version of GLSL you are using. In particular,
 * on Mac OS X currently there is no way of using OpenGL 3.x with GLSL 1.3 when
 * GLUT is used.
 *
 * If g_Gl2Compatible=true, shaders with -gl2 suffix will be loaded.
 * If g_Gl2Compatible=false, shaders with -gl3 suffix will be loaded.
 * To complete the assignment you only need to edit the shader files that get loaded
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
static const bool g_Gl2Compatible = true;
static const float g_initialWidth  = 512.0;
static const float g_initialHeight = g_initialWidth;

static int g_width             = g_initialWidth;  /** screen width */
static int g_height            = g_initialHeight; /** screen height */
static bool g_leftClicked      = false;     /** is the left mouse button down? */
static bool g_rightClicked     = false;     /** is the right mouse button down? */
static float g_objScale        = 1.0;       /** scale factor for object */
static int g_leftClickX, g_leftClickY;      /** coordinates for mouse left click event */
static int g_rightClickX, g_rightClickY;    /** coordinates for mouse right click event */

/**
 * Represents the offset of the triangle in the x-direction. This increments
 * each time the shape should move to the right, and decrements each time the
 * shape should move to the left.
 */
static int g_xOffset           = 0.0;

/** Like xOffset, but in the y direction. */
static int g_yOffset           = 0.0;

/** Global shader states */
struct SquareShaderState {
  GlProgram program;

  /** Handles to uniform variables */
  GLint h_uVertexScale;
  GLint h_uTex0, h_uTex1;
  GLint h_uXCoefficient, h_uYCoefficient;

  /** Handles to vertex attributes */
  GLint h_aPosition;
  GLint h_aTexCoord;
};

struct TriangleShaderState {
  GlProgram program;

  /** Handles to uniform variables */
  GLint h_uVertexScale;
  GLint h_uTex2;
  GLint h_uXCoefficient, h_uYCoefficient;
  GLint h_uXOffset, h_uYOffset;

  /** Handles to vertex attributes */
  GLint h_aPosition;
  GLint h_aTexCoord;
  GLint h_aColor;
};

static shared_ptr<SquareShaderState> g_squareShaderState;
static shared_ptr<TriangleShaderState> g_triangleShaderState;

/** Global texture instance */
static shared_ptr<GlTexture> g_tex0, g_tex1, g_tex2;

/** Global geometries */
struct GeometryPX {
  GlBufferObject posVbo, texVbo, colorVbo;
};

static shared_ptr<GeometryPX> g_square;
static shared_ptr<GeometryPX> g_triangle;


/* C A L L B A C K S **************************************************/

static void drawSquare() {
  /* Activate the glsl program */
  glUseProgram(g_squareShaderState->program);

  /* Bind textures */
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, *g_tex0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, *g_tex1);

  /* Compute coefficients for maintaining aspect ratio */
  float scaleCoefficient = min(g_width / g_initialWidth, g_height / g_initialHeight);

  /* Set glsl uniform variables */
  safe_glUniform1i(g_squareShaderState->h_uTex0, 0); /* 0 means GL_TEXTURE0 */
  safe_glUniform1i(g_squareShaderState->h_uTex1, 1); /* 1 means GL_TEXTURE1 */
  safe_glUniform1f(g_squareShaderState->h_uVertexScale, g_objScale);
  safe_glUniform1f(g_squareShaderState->h_uXCoefficient, g_initialWidth / g_width * scaleCoefficient);
  safe_glUniform1f(g_squareShaderState->h_uYCoefficient, g_initialHeight / g_height * scaleCoefficient);

  /* Bind vertex buffers */
  glBindBuffer(GL_ARRAY_BUFFER, g_square->posVbo);
  safe_glVertexAttribPointer(g_squareShaderState->h_aPosition,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_square->texVbo);
  safe_glVertexAttribPointer(g_squareShaderState->h_aTexCoord,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(g_squareShaderState->h_aPosition);
  safe_glEnableVertexAttribArray(g_squareShaderState->h_aTexCoord);

  /* Draw using 6 vertices, forming two triangles */
  glDrawArrays(GL_TRIANGLES, 0, 6);

  safe_glDisableVertexAttribArray(g_squareShaderState->h_aPosition);
  safe_glDisableVertexAttribArray(g_squareShaderState->h_aTexCoord);

  /* Check for errors */
  checkGlErrors();
}

static void drawTriangle() {

  /* Activate the glsl program */
  glUseProgram(g_triangleShaderState->program);

  /* Bind textures */
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, *g_tex2);

  /* Compute coefficients for maintaining aspect ratio */
  float scaleCoefficient = min(g_width / g_initialWidth, g_height / g_initialHeight);

  /* Set glsl uniform variables */
  safe_glUniform1i(g_triangleShaderState->h_uTex2, 2); /* 2 means GL_TEXTURE2 */
  safe_glUniform1f(g_triangleShaderState->h_uVertexScale, g_objScale);
  safe_glUniform1f(g_triangleShaderState->h_uXCoefficient, g_initialWidth / g_width * scaleCoefficient);
  safe_glUniform1f(g_triangleShaderState->h_uYCoefficient, g_initialHeight / g_height * scaleCoefficient);

  /* Uniform variables used to move the triangle around the screen */
  safe_glUniform1f(g_triangleShaderState->h_uXOffset, g_xOffset * .05);
  safe_glUniform1f(g_triangleShaderState->h_uYOffset, g_yOffset * .05);

  /* Bind vertex buffers */
  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->posVbo);

  safe_glVertexAttribPointer(g_triangleShaderState->h_aPosition,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->texVbo);
  safe_glVertexAttribPointer(g_triangleShaderState->h_aTexCoord,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->colorVbo);
  safe_glVertexAttribPointer(g_triangleShaderState->h_aColor,
                             3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aPosition);
  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aTexCoord);
  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aColor);

  /* Draw using 3 vertices, forming a triangle */
  glDrawArrays(GL_TRIANGLES, 0, 3);

  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aPosition);
  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aTexCoord);
  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aColor);

  /* Check for errors */
  checkGlErrors();
}

/**
 * Display
 *
 * Whenever OpenGL requires a screen refresh it will call display() to draw the
 * scene. We specify that this is the correct function to call with the
 * glutDisplayFunc() function during initialization.
 */
static void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawSquare();
  drawTriangle();

  glutSwapBuffers();

  /* check for errors */
  checkGlErrors();
}


/**
 * Reshape
 *
 * Whenever a window is resized, a "resize" event is generated and glut is told
 * to call this reshape callback function to handle it appropriately.
 */
static void reshape(int w, int h) {
  g_width = w;
  g_height = h;
  glViewport(0, 0, w, h);
  glutPostRedisplay();
}


/**
 * Mouse
 *
 * Whenever a mouse button is clicked, a "mouse" event is generated and this
 * mouse callback function is called to handle the user input.
 */

static void mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      /* right mouse button has been clicked */
      g_leftClicked = true;
      g_leftClickX = x;
      g_leftClickY = g_height - y - 1;
    }
    else {
      /* right mouse button has been released */
      g_leftClicked = false;
    }
  }
  if (button == GLUT_RIGHT_BUTTON) {
    if (state == GLUT_DOWN) {
      /* right mouse button has been clicked */
      g_rightClicked = true;
      g_rightClickX = x;
      g_rightClickY = g_height - y - 1;
    }
    else {
      /* right mouse button has been released */
      g_rightClicked = false;
    }
  }
}

/**
 * Motion
 *
 * Whenever the mouse is moved while a button is pressed, a "mouse move" event
 * is triggered and this callback is called to handle the event.
 */
static void motion(int x, int y) {
  const int newx = x;
  const int newy = g_height - y - 1;
  if (g_leftClicked) {
    g_leftClickX = newx;
    g_leftClickY = newy;
  }
  if (g_rightClicked) {
    float deltax = (newx - g_rightClickX) * 0.02;
    g_objScale += deltax;

    g_rightClickX = newx;
    g_rightClickY = newy;
  }
  glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'h':
    cout << " ============== H E L P ==============\n\n"
    << "h\t\thelp menu\n"
    << "s\t\tsave screenshot\n"
    << "drag right mouse to change square size\n";
    break;
  case 'q':
    exit(0);
  case 'i':
    g_yOffset++;
    break;
  case 'j':
    g_xOffset--;
    break;
  case 'k':
    g_yOffset--;
    break;
  case 'l':
    g_xOffset++;
    break;
  case 's':
    glFinish();
    writePpmScreenshot(g_width, g_height, "out.ppm");
    break;
  }
  glutPostRedisplay();
}

/* H E L P E R    F U N C T I O N S ***********************************/

static void initGlutState(int argc, char **argv) {
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(g_width, g_height);      /* create a window */
  glutCreateWindow("CS 175: Hello World");    /* title the window */

  glutDisplayFunc(display);                   /* display rendering callback */
  glutReshapeFunc(reshape);                   /* window reshape callback */
  glutMotionFunc(motion);                     /* mouse movement callback */
  glutMouseFunc(mouse);                       /* mouse click callback */
  glutKeyboardFunc(keyboard);
}

static void initGLState() {
  glClearColor(128./255,200./255,1,0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  if (!g_Gl2Compatible)
    glEnable(GL_FRAMEBUFFER_SRGB);
}

static void loadSquareShader(SquareShaderState& ss) {
  const GLuint h = ss.program; /* Short hand */

  if (!g_Gl2Compatible) {
    readAndCompileShader(ss.program, "shaders/asst1-sq-gl3.vshader", "shaders/asst1-sq-gl3.fshader");
  }
  else {
    readAndCompileShader(ss.program, "shaders/asst1-sq-gl2.vshader", "shaders/asst1-sq-gl2.fshader");
  }

  /* Retrieve handles to uniform variables */
  ss.h_uVertexScale = safe_glGetUniformLocation(h, "uVertexScale");
  ss.h_uTex0 = safe_glGetUniformLocation(h, "uTex0");
  ss.h_uTex1 = safe_glGetUniformLocation(h, "uTex1");
  ss.h_uXCoefficient = safe_glGetUniformLocation(h, "uXCoefficient");
  ss.h_uYCoefficient = safe_glGetUniformLocation(h, "uYCoefficient");

  /* Retrieve handles to vertex attributes */
  ss.h_aPosition = safe_glGetAttribLocation(h, "aPosition");
  ss.h_aTexCoord = safe_glGetAttribLocation(h, "aTexCoord");

  if (!g_Gl2Compatible)
    glBindFragDataLocation(h, 0, "fragColor");
  checkGlErrors();
}

static void loadTriangleShader(TriangleShaderState& ss) {
  const GLuint h = ss.program; /* Short hand */

  if (!g_Gl2Compatible) {
    readAndCompileShader(ss.program, "shaders/asst1-tr-gl3.vshader", "shaders/asst1-tr-gl3.fshader");
  }
  else {
    readAndCompileShader(ss.program, "shaders/asst1-tr-gl2.vshader", "shaders/asst1-tr-gl2.fshader");
  }

  /* Retrieve handles to uniform variables */
  ss.h_uVertexScale = safe_glGetUniformLocation(h, "uVertexScale");
  ss.h_uTex2 = safe_glGetUniformLocation(h, "uTex2");
  ss.h_uXCoefficient = safe_glGetUniformLocation(h, "uXCoefficient");
  ss.h_uYCoefficient = safe_glGetUniformLocation(h, "uYCoefficient");
  ss.h_uXOffset = safe_glGetUniformLocation(h, "uXOffset");
  ss.h_uYOffset = safe_glGetUniformLocation(h, "uYOffset");

  /* Retrieve handles to vertex attributes */
  ss.h_aPosition = safe_glGetAttribLocation(h, "aPosition");
  ss.h_aTexCoord = safe_glGetAttribLocation(h, "aTexCoord");
  ss.h_aColor = safe_glGetAttribLocation(h, "aColor");

  if (!g_Gl2Compatible)
    glBindFragDataLocation(h, 0, "fragColor");
  checkGlErrors();
}

static void initShaders() {
  g_squareShaderState.reset(new SquareShaderState);
  loadSquareShader(*g_squareShaderState);

  g_triangleShaderState.reset(new TriangleShaderState);
  loadTriangleShader(*g_triangleShaderState);
}

static void loadSquareGeometry(const GeometryPX& g) {
  const int dim = 6;
  GLfloat pos[2 * dim] = {
    -.5, -.5,
    .5,  .5,
    .5,  -.5,

    -.5, -.5,
    -.5, .5,
    .5,  .5
  };

  GLfloat tex[2 * dim] = {
    0, 0,
    1, 1,
    1, 0,

    0, 0,
    0, 1,
    1, 1
  };

  glBindBuffer(GL_ARRAY_BUFFER, g.posVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*dim*sizeof(GLfloat),
    pos,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.texVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*dim*sizeof(GLfloat),
    tex,
    GL_STATIC_DRAW);
  checkGlErrors();
}

static void loadTriangleGeometry(const GeometryPX& g) {
  const int dim = 3;
  GLfloat pos[2 * dim] = {
    0.0, -0.45,
    -0.45, 0.45,
    0.45, 0.45
  };

  /* Center shield in triangle */
  GLfloat tex[2 * dim] = {
    0.5, -.60,
    -.35, 1.1,
    1.35, 1.1
  };

  /* Give each vertex a different color */
  GLfloat color[3 * dim] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
  };

  glBindBuffer(GL_ARRAY_BUFFER, g.posVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*dim*sizeof(GLfloat),
    pos,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.texVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*dim*sizeof(GLfloat),
    tex,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.colorVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    3*dim*sizeof(GLfloat),
    color,
    GL_STATIC_DRAW);
  checkGlErrors();
}

static void initGeometry() {
  g_square.reset(new GeometryPX());
  loadSquareGeometry(*g_square);

  g_triangle.reset(new GeometryPX());
  loadTriangleGeometry(*g_triangle);
}

static void loadTexture(GLuint texHandle, const char *ppmFilename) {
  int texWidth, texHeight;
  vector<PackedPixel> pixData;

  ppmRead(ppmFilename, texWidth, texHeight, pixData);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texHandle);
  glTexImage2D(GL_TEXTURE_2D, 0, g_Gl2Compatible ? GL_RGB : GL_SRGB, texWidth, texHeight,
               0, GL_RGB, GL_UNSIGNED_BYTE, &pixData[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  checkGlErrors();
}

static void initTextures() {
  g_tex0.reset(new GlTexture());
  g_tex1.reset(new GlTexture());
  g_tex2.reset(new GlTexture());

  loadTexture(*g_tex0, "smiley.ppm");
  loadTexture(*g_tex1, "reachup.ppm");
  loadTexture(*g_tex2, "shield.ppm");
}

/* M A I N ************************************************************/

/**
 * Main
 *
 * The main entry-point for the HelloWorld example application.
 */
int main(int argc, char **argv) {
  try {
    initGlutState(argc,argv);

    glewInit(); // load the OpenGL extensions

    cout << (g_Gl2Compatible ? "Will use OpenGL 2.x / GLSL 1.0" : "Will use OpenGL 3.x / GLSL 1.3") << endl;
    if ((!g_Gl2Compatible) && !GLEW_VERSION_3_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.3");
    else if (g_Gl2Compatible && !GLEW_VERSION_2_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.0");

    initGLState();
    initShaders();
    initGeometry();
    initTextures();

    glutMainLoop();
    return 0;
  }
  catch (const runtime_error& e) {
    cout << "Exception caught: " << e.what() << endl;
    return -1;
  }
}
