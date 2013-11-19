/* Some constant shits */
static const int    g_numShells = 24;
static       double g_furHeight = 0.21;
static       double g_hairyness = 0.7;
static const Cvec3  g_gravity(0, -0.5, 0);
static       double g_timeStep = 0.02;
static       double g_numStepsPerFrame = 10;
static       double g_damping = 0.96;
static       double g_stiffness = 4;

/**
 * This is used to cache shell vertex values during hair curving.
 * That way we don't have to re-compute intermediate values.
 */
static vector<vector<Cvec3> > g_ShellVertexCache;

/** Bunny node */
static shared_ptr<SgRbtNode> g_bunnyNode;

/** The bunny mesh */
static Mesh g_bunnyMesh;
/** Shell geometries */
static vector<shared_ptr<SimpleGeometryPNX> > g_bunnyShellGeometries;

/** Used for physical simulation */
static int g_simulationsPerSecond = 60;

/** The hair tip position in world-space coordinates */
static std::vector<Cvec3> g_tipPos;
/** The hair tip velocity in world-space coordinates */
static std::vector<Cvec3> g_tipVelocity;

/**
 * Given a vertex on the bunny, returns the at-rest position of the hair tip.
 * @param  v The vertex on the bunny.
 * @return   The position of the tip of the hair.
 */
Cvec3 getAtRestTipPosition(Mesh::Vertex v) {
  return v.getPosition() + v.getNormal() * g_furHeight;
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
 * @param          v The vertex on the bunny itself.
 * @param          i The layer of the bunny that we're computing.
 * @param texVec The vector corresponding to the texture location we should
 *                   map to.
 */
static VertexPNX computeHairVertex(
    Mesh::Vertex v, int i,
    Cvec2 texVec,
    RigTForm invBunnyRbt) {
  const Cvec3 p = v.getPosition();
  const Cvec3 n = v.getNormal() * (g_furHeight / g_numShells);
  const Cvec3 t = invBunnyRbt * g_tipPos[v.getIndex()];
  const Cvec3 d = ((t - p - n * g_numShells) /
    (g_numShells * g_numShells - g_numShells)) * 2;

  Cvec3 point;
  if (i == 0) {
    point = v.getPosition();
  } else {
    point = g_ShellVertexCache[v.getIndex()][i - 1];
  }
  g_ShellVertexCache[v.getIndex()][i] = point + n + d * i;

  return VertexPNX(g_ShellVertexCache[v.getIndex()][i], v.getNormal(), texVec);
}

/**
 * Returns the vertices for the layer-th layer of the bunny shell.
 */
static vector<VertexPNX> getBunnyShellGeometryVertices(
    Mesh &mesh,
    int layer,
    RigTForm invBunnyRbt) {

  /* initialize the shell vertex cache to the proper size */
  g_ShellVertexCache.resize(mesh.getNumVertices());
  for (int i = 0; i < mesh.getNumVertices(); i++) {
    g_ShellVertexCache[i].resize(g_numShells);
  }

  vector<VertexPNX> vs;
  /* For each face: */
  for (int i = 0; i < mesh.getNumFaces(); i++) {
    Mesh::Face f = mesh.getFace(i);
    /* For each vertex of each face: */
    for (int j = 1; j < f.getNumVertices() - 1; j++) {
      vs.push_back(computeHairVertex(f.getVertex(  0), layer, Cvec2(0, 0)          , invBunnyRbt));
      vs.push_back(computeHairVertex(f.getVertex(  j), layer, Cvec2(g_hairyness, 0), invBunnyRbt));
      vs.push_back(computeHairVertex(f.getVertex(j+1), layer, Cvec2(0, g_hairyness), invBunnyRbt));
    }
  }

  return vs;
}

/**
 * Updates the hair calculations according to the physics simulation described
 * in the assignment.
 *
 * Step 1: Compute f
 * Step 2: Update t
 * Step 3: Constrain t
 * Step 4: Update v
 *
 * @param vertex   The vertex on the bunny to compute the shell tip for.
 * @param bunnyRbt The RBT for the bunny.
 */
static void updateHairCalculation(Mesh::Vertex vertex, RigTForm bunnyRbt) {
  int vertexIndex = vertex.getIndex();
  /* Reassignments so that we're consistent with notation in the assignment. */
  double T = g_timeStep;
  Cvec3 p = bunnyRbt * vertex.getPosition();
  Cvec3 s = bunnyRbt * getAtRestTipPosition(vertex);
  Cvec3 t = g_tipPos[vertexIndex];
  Cvec3 v = g_tipVelocity[vertexIndex];

  /* Step 1: Compute f */
  Cvec3 f = g_gravity + (s - t) * g_stiffness;
  /* Step 2: Update t */
  t = t + v * T;
  /* Step 3: Constrain t */
  g_tipPos[vertexIndex] = (p + (t - p) / norm(t - p) * g_furHeight);
  /* Step 4: Update v */
  g_tipVelocity[vertexIndex] = ((v + f * T) * g_damping);
}

/**
 * Updates the hair calculations for the bunny based on the physics simulation
 * descriptions provided in the assignment.
 */
static void updateHairs(Mesh &mesh) {
  RigTForm bunnyRbt = getPathAccumRbt(g_world, g_bunnyNode);
  for (int i = 0; i < mesh.getNumVertices(); i++) {
    updateHairCalculation(mesh.getVertex(i), bunnyRbt);
  }
}

/**
 * Performs dynamics simulation g_simulationsPerSecond times per second
 */
static void hairsSimulationCallback(int _) {
  /* Update the hair dynamics. HACK: Ideally, we'd be passing in g_bunnyMesh to
     this function, but that's hard since it's a fucking callback. */
  for (int i = 0; i < g_numStepsPerFrame; i++) {
    updateHairs(g_bunnyMesh);
  }
  /* Schedule this to get called again */
  glutTimerFunc(1000 / g_simulationsPerSecond, hairsSimulationCallback, _);
}

/**
 * Updates the bunny hairs so that they're ready to be rendered.
 */
static void prepareBunnyForRendering() {
  RigTForm invBunnyRbt = inv(getPathAccumRbt(g_world, g_bunnyNode));
  for (int i = 0; i < g_numShells; ++i) {
    vector<VertexPNX> verticies =
      getBunnyShellGeometryVertices(g_bunnyMesh, i, invBunnyRbt);
    g_bunnyShellGeometries[i]->upload(&verticies[0], verticies.size());
  }
}
