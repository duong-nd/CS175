//------------------------------------------------------------------
// STEP 1: Add global varaibles that hold additional materials,
//         geometry, and scene graph nodes used by the bunny mesh
//         and shell, and the corresponding initialization code. After
//         this step you should see a bunny getting drawn but without
//         any fur.
//------------------------------------------------------------------
// Material
static shared_ptr<Material> g_bunnyMat; // for the bunny

static vector<shared_ptr<Material> > g_bunnyShellMats; // for bunny shells

// New Geometry
static const int g_numShells = 24; // constants defining how many layers of shells
static double g_furHeight = 0.21;
static double g_hairyness = 0.7;

static shared_ptr<SimpleGeometryPN> g_bunnyGeometry;
static vector<shared_ptr<SimpleGeometryPNX> > g_bunnyShellGeometries;
static Mesh g_bunnyMesh;


// New Scene node
static shared_ptr<SgRbtNode> g_bunnyNode;

...

// New function that loads the bunny mesh and initializes the bunny shell meshes
static void initBunnyMeshes() {
  g_bunnyMesh.load("bunny.mesh");

  // TODO: Init the per vertex normal of g_bunnyMesh, using code from asst7
  // ...

  // TODO: Initialize g_bunnyGeometry from g_bunnyMesh, similar to
  // what you did for asst7 ...

  // Now allocate array of SimpleGeometryPNX to for shells, one per layer
  g_bunnyShellGeometries.resize(g_numShells);
  for (int i = 0; i < g_numShells; ++i) {
    g_bunnyShellGeometries[i].reset(new SimpleGeometryPNX());
  }
}

...

// Load material for the bunny and shell
static void initMaterials() {

  ...

  // bunny material
  g_bunnyMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/bunny-gl3.fshader"));
  g_bunnyMat->getUniforms()
  .put("uColorAmbient", Cvec3f(0.45f, 0.3f, 0.3f))
  .put("uColorDiffuse", Cvec3f(0.2f, 0.2f, 0.2f));

  // bunny shell materials;
  shared_ptr<ImageTexture> shellTexture(new ImageTexture("shell.ppm", false)); // common shell texture

  // needs to enable repeating of texture coordinates
  shellTexture->bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // eachy layer of the shell uses a different material, though the materials will share the
  // same shader files and some common uniforms. hence we create a prototype here, and will
  // copy from the prototype later
  Material bunnyShellMatPrototype("./shaders/bunny-shell-gl3.vshader", "./shaders/bunny-shell-gl3.fshader");
  bunnyShellMatPrototype.getUniforms().put("uTexShell", shellTexture);
  bunnyShellMatPrototype.getRenderStates()
  .blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) // set blending mode
  .enable(GL_BLEND) // enable blending
  .disable(GL_CULL_FACE); // disable culling

  // allocate array of materials
  g_bunnyShellMats.resize(g_numShells);
  for (int i = 0; i < g_numShells; ++i) {
    g_bunnyShellMats[i].reset(new Material(bunnyShellMatPrototype)); // copy from the prototype
    // but set a different exponent for blending transparency
    g_bunnyShellMats[i]->getUniforms().put("uAlphaExponent", 2.f + 5.f * float(i + 1)/g_numShells);
  }
};

// Load additional geometry
static void initGeometry() {
  ...

  initBunnyMeshes();
}

// Create corresponding scene graph nodes
static void initScene() {

  // Remember to move the subdiv mesh node away a bit so it doesn't block the bunny.
  ...

  // create a single transform node for both the bunny and the bunny shells
  g_bunnyNode.reset(new SgRbtNode());

  // add bunny as a shape nodes
  g_bunnyNode->addChild(shared_ptr<MyShapeNode>(
                          new MyShapeNode(g_bunnyGeometry, g_bunnyMat)));

  // add each shell as shape node
  for (int i = 0; i < g_numShells; ++i) {
    g_bunnyNode->addChild(shared_ptr<MyShapeNode>(
                            new MyShapeNode(g_bunnyShellGeometries[i], g_bunnyShellMats[i])));
  }
  // from this point, calling g_bunnyShellGeometries[i]->upload(...) will change the
  // geometry of the ith layer of shell that gets drawn

  ...

  g_world->addChild(g_bunnyNode);
}


//------------------------------------------------------------------
// STEP 2: Add GLUT special key handlers for controlling rendering
//         of furs
//------------------------------------------------------------------

// new  special keyboard callback, for arrow keys
static void specialKeyboard(const int key, const int x, const int y) {
  switch (key) {
  case GLUT_KEY_RIGHT:
    g_furHeight *= 1.05;
    cerr << "fur height = " << g_furHeight << std::endl;
    break;
  case GLUT_KEY_LEFT:
    g_furHeight /= 1.05;
    std::cerr << "fur height = " << g_furHeight << std::endl;
    break;
  case GLUT_KEY_UP:
    g_hairyness *= 1.05;
    cerr << "hairyness = " << g_hairyness << std::endl;
    break;
  case GLUT_KEY_DOWN:
    g_hairyness /= 1.05;
    cerr << "hairyness = " << g_hairyness << std::endl;
    break;
  }
  glutPostRedisplay();
}

// and register it in initGlutState(...)
static void initGlutState(int argc, char * argv[]) {
  ...

  glutSpecialFunc(specialKeyboard);                       // special keyboard callback
}




//------------------------------------------------------------------
// STEP 3: Add global variables used for physical simulation
//         control fur rendering, as well as GLUT special key handlers
//         for changing their values.
//------------------------------------------------------------------

// Global variables for used physical simulation
static const Cvec3 g_gravity(0, -0.5, 0);  // gavity vector
static double g_timeStep = 0.02;
static double g_numStepsPerFrame = 10;
static double g_damping = 0.96;
static double g_stiffness = 4;
static int g_simulationsPerSecond = 60;

static std::vector<Cvec3> g_tipPos,        // should be hair tip pos in world-space coordinates
                          g_tipVelocity;   // should be hair tip velocity in world-space coordinates


// Specifying shell geometries based on g_tipPos, g_furHeight, and g_numShells.
// You need to call this function whenver the shell needs to be updated
static void updateShellGeometry() {
  // TASK 1 and 3 TODO: finish this function as part of Task 1 and Task 3
}

// New glut timer call back that perform dynamics simulation
// every g_simulationsPerSecond times per second
static void hairsSimulationCallback(int dontCare) {

  // TASK 2 TODO: wrte dynamics simulation code here as part of TASK2

  ...

  // schedule this to get called again
  glutTimerFunc(1000/g_simulationsPerSecond, hairsSimulationCallback, 0);
  glutPostRedisplay(); // signal redisplaying
}

// New function that initialize the dynamics simulation
static void initSimulation() {
  g_tipPos.resize(g_bunnyMesh.getNumVertices(), Cvec3(0));
  g_tipVelocity = g_tipPos;

  // TASK 1 TODO: initialize g_tipPos to "at-rest" hair tips in world coordinates

  ...

  // Starts hair tip simulation
  hairsSimulationCallback(0);
}

// Remember to call initSimulation in main()
int main(int argc, char * argv[]) {
  ...
  initSimulation();
  ...
}


//------------------------------------------------------------------------
// CONGRATULATIONS! YOU'RE DONE WITH CODE PREPARATION FOR THIS ASSINGMENT
//------------------------------------------------------------------------
