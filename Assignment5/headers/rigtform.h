#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <iostream>
#include <cassert>

#include <fstream>
#include <sstream>

#include "matrix4.h"
#include "quat.h"
#include "utils.h"

using namespace std;

class RigTForm {
private:
  static const char SERIALIZATION_DELIMITER = ';';
  Cvec3 t_; // translation component
  Quat r_;  // rotation component represented as a quaternion

public:
  RigTForm() : t_(0) {
    assert(norm2(Quat(1,0,0,0) - r_) < CS175_EPS2);
  }

  RigTForm(const Cvec3& t, const Quat& r) : t_(t), r_(r) {}

  explicit RigTForm(const Cvec3& t) : t_(t), r_() {}

  explicit RigTForm(const Quat& r) : t_(0), r_(r) {}

  Cvec3 getTranslation() const {
    return t_;
  }

  Quat getRotation() const {
    return r_;
  }

  RigTForm& setTranslation(const Cvec3& t) {
    t_ = t;
    return *this;
  }

  RigTForm& setRotation(const Quat& r) {
    r_ = r;
    return *this;
  }

  Cvec4 operator * (const Cvec4& a) const {
    return r_ * a + Cvec4(t_, 0) * a[3];
  }

  RigTForm operator * (const RigTForm& a) const {
    return RigTForm(
      t_ + Cvec3(r_ * Cvec4(a.getTranslation(), 0)),
      r_ * a.getRotation()
    );
  }

  std::string serialize() {
    stringstream s;
    s << t_.serialize() << SERIALIZATION_DELIMITER << r_.serialize();
    return s.str();
  }

  static RigTForm deserialize(std::string serialized) {
    vector<string> parts = split(serialized, SERIALIZATION_DELIMITER);
    assert(parts.size() == 2);
    return RigTForm(deserializeCvec3(parts[0]), Quat::deserialize(parts[1]));
  }

  static Cvec3 lerp(Cvec3 c_0, Cvec3 c_1, double alpha) {
    return c_0 * (1 - alpha) + c_1 * alpha;
  }

  static Quat slerp(Quat q_0, Quat q_1, double alpha) {
    if (q_0 == q_1) {
      std::cout << "SUCCESSFULLY COMPUTED RESULT B/C ==." << std::endl;
      return q_0;
    }

    Quat result = cn(q_1 * inv(q_0)).raisedTo(alpha) * q_0;
    std::cout << "SUCCESSFULLY COMPUTED RESULT" << std::endl;
    return result;
  }

  static Quat cn(Quat q) {
    if (q[0] < 0) {
      return Quat(q[0] * -1, q[1] * -1, q[2] * -1, q[3] * -1);
    }
    return q;
  }

  std::string DEBUG_STRING() {
    double Number = t_[0];       // number to be converted to a string

std::string Result;          // string which will contain the result

std::ostringstream convert;   // stream used for the conversion

convert << Number;      // insert the textual representation of 'Number' in the characters in the stream

Result = convert.str(); // set 'Result' to the contents of the stream

    return Result;
  }
};

inline RigTForm inv(const RigTForm& tform) {
  Quat r_inv = inv(tform.getRotation());
  return RigTForm(
    Cvec3(r_inv * Cvec4(-tform.getTranslation(), 1)),
    r_inv);
}

inline RigTForm transFact(const RigTForm& tform) {
  return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform) {
  return RigTForm(tform.getRotation());
}

inline Matrix4 rigTFormToMatrix(const RigTForm& tform) {
  Matrix4 T = Matrix4::makeTranslation(tform.getTranslation());
  Matrix4 R = quatToMatrix(tform.getRotation());
  return T * R;
}

#endif
