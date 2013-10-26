#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <iostream>
#include <cassert>

#include <iostream>
#include <fstream>

#include "matrix4.h"
#include "quat.h"

class RigTForm {
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
    // TODO
  }

  static RigTForm deserialize(std::string serialized) {
    // TODO
  }

  static Cvec3 lerp(Cvec3 c_0, Cvec3 c_1, double alpha) {
    return c_0 * (1 - alpha) + c_1 * alpha;
  }

  static Quat slerp(Quat q_0, Quat q_1, double alpha) {
    return (cn(q_1 * (q_0.raisedTo(-1)))).raisedTo(alpha) * q_0;
  }

  static Quat cn(Quat q) {
    if (q[0] < 0) {
      return Quat(q[0] * -1, q[1] * -1, q[2] * -1, q[3] * -1);
    }
    return q;
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
