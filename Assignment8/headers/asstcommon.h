#ifndef ASSTCOMMON_H
#define ASSTCOMMON_H

#include <cstddef>
#include <vector>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include "glsupport.h"
#include "uniforms.h"
#include "material.h"

extern const bool g_Gl2Compatible;

extern std::tr1::shared_ptr<Material> g_overridingMaterial;

// takes MVM and its normal matrix to the shaders
inline void sendModelViewNormalMatrix(Uniforms& uniforms, const Matrix4& MVM, const Matrix4& NMVM) {
  uniforms.put("uModelViewMatrix", MVM).put("uNormalMatrix", NMVM);
}

#endif