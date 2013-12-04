#ifndef DEBUG_H
#define DEBUG_H

// #include "rigtform.h"

using namespace std;

static void printVector(string prefix, Cvec3 vec) {
  cout << prefix << "(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")" << endl;
}

static void printVector(Cvec3 vec) {
  printVector("Vector: ", vec);
}

static int printRigTForm(RigTForm rbt) {
  Cvec3 t_ = rbt.getTranslation();
  Cvec4 r_ = rbt.getRotation().q_;
  cout << "(" << t_[0] << ", " << t_[1] << ", " << t_[2] << "; "
       << r_[0] << ", " << r_[1] << ", " << r_[2] << ", " << r_[3] << ")"
       << endl;
}

#endif
