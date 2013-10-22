#ifndef QUAT_H
#define QUAT_H

#include <iostream>
#include <cassert>
#include <cmath>

#include "cvec.h"
#include "matrix4.h"

// Forward declarations used in the definition of Quat;
class Quat;
double dot(const Quat& q, const Quat& p);
double norm2(const Quat& q);
Quat inv(const Quat& q);
Quat normalize(const Quat& q);
Matrix4 quatToMatrix(const Quat& q);

class Quat {
  Cvec4 q_;  // layout is: q_[0]==w, q_[1]==x, q_[2]==y, q_[3]==z

public:
  double operator [] (const int i) const {
    return q_[i];
  }

  double& operator [] (const int i) {
    return q_[i];
  }

  double operator () (const int i) const {
    return q_[i];
  }

  double& operator () (const int i) {
    return q_[i];
  }

  Quat() : q_(1,0,0,0) {}
  Quat(const double w, const Cvec3& v) : q_(w, v[0], v[1], v[2]) {}
  Quat(const double w, const double x, const double y, const double z) : q_(w, x,y,z) {}

  Quat& operator += (const Quat& a) {
    q_ += a.q_;
    return *this;
  }

  Quat& operator -= (const Quat& a) {
    q_ -= a.q_;
    return *this;
  }

  Quat& operator *= (const double a) {
    q_ *= a;
    return *this;
  }

  Quat& operator /= (const double a) {
    q_ /= a;
    return *this;
  }

  Quat operator + (const Quat& a) const {
    return Quat(*this) += a;
  }

  Quat operator - (const Quat& a) const {
    return Quat(*this) -= a;
  }

  Quat operator * (const double a) const {
    return Quat(*this) *= a;
  }

  Quat operator / (const double a) const {
    return Quat(*this) /= a;
  }

  Quat operator * (const Quat& a) const {
    const Cvec3 u(q_[1], q_[2], q_[3]), v(a.q_[1], a.q_[2], a.q_[3]);
    return Quat(q_[0]*a.q_[0] - dot(u, v), (v*q_[0] + u*a.q_[0]) + cross(u, v));
  }

  Cvec4 operator * (const Cvec4& a) const {
    const Quat r = *this * (Quat(0, a[0], a[1], a[2]) * inv(*this));
    return Cvec4(r[1], r[2], r[3], a[3]);
  }

  static Quat makeXRotation(const double ang) {
    Quat r;
    const double h = 0.5 * ang * CS175_PI/180;
    r.q_[1] = std::sin(h);
    r.q_[0] = std::cos(h);
    return r;
  }

  static Quat makeYRotation(const double ang) {
    Quat r;
    const double h = 0.5 * ang * CS175_PI/180;
    r.q_[2] = std::sin(h);
    r.q_[0] = std::cos(h);
    return r;
  }

  static Quat makeZRotation(const double ang) {
    Quat r;
    const double h = 0.5 * ang * CS175_PI/180;
    r.q_[3] = std::sin(h);
    r.q_[0] = std::cos(h);
    return r;
  }
};

inline double dot(const Quat& q, const Quat& p) {
  double s = 0.0;
  for (int i = 0; i < 4; ++i) {
    s += q(i) * p(i);
  }
  return s;
}

inline double norm2(const Quat& q) {
  return dot(q, q);
}

inline Quat inv(const Quat& q) {
  const double n = norm2(q);
  assert(n > CS175_EPS2);
  return Quat(q(0), -q(1), -q(2), -q(3)) * (1.0/n);
}

inline Quat normalize(const Quat& q) {
  return q / std::sqrt(norm2(q));
}

inline Matrix4 quatToMatrix(const Quat& q) {
  Matrix4 r;
  const double n = norm2(q);
  if (n < CS175_EPS2)
    return Matrix4(0);

  const double two_over_n = 2/n;
  r(0, 0) -= (q(2)*q(2) + q(3)*q(3)) * two_over_n;
  r(0, 1) += (q(1)*q(2) - q(0)*q(3)) * two_over_n;
  r(0, 2) += (q(1)*q(3) + q(2)*q(0)) * two_over_n;
  r(1, 0) += (q(1)*q(2) + q(0)*q(3)) * two_over_n;
  r(1, 1) -= (q(1)*q(1) + q(3)*q(3)) * two_over_n;
  r(1, 2) += (q(2)*q(3) - q(1)*q(0)) * two_over_n;
  r(2, 0) += (q(1)*q(3) - q(2)*q(0)) * two_over_n;
  r(2, 1) += (q(2)*q(3) + q(1)*q(0)) * two_over_n;
  r(2, 2) -= (q(1)*q(1) + q(2)*q(2)) * two_over_n;

  assert(isAffine(r));
  return r;
}

#endif
