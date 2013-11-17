#ifndef DEBUG_H
#define DEBUG_H

using namespace std;

static int printVector(Cvec3 vec, string prefix) {
  cout << prefix << "(" << vec[0] << ", " << vec[1] << ", " << vec[2] << endl;
}

static int printVector(Cvec3 vec) {
  printVector(vec, "Vector: ");
}

#endif
