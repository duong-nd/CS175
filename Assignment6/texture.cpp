#include <vector>

#include "ppm.h"
#include "glsupport.h"
#include "texture.h"
#include "asstcommon.h"

using namespace std;

ImageTexture::ImageTexture(const char* ppmFileName, bool srgb) {
  int width, height;
  vector<PackedPixel> pixData;
  ppmRead(ppmFileName, width, height, pixData);

  glBindTexture(GL_TEXTURE_2D, tex);
  if (g_Gl2Compatible)
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

  glTexImage2D(GL_TEXTURE_2D, 0, (!srgb) || g_Gl2Compatible ? GL_RGB : GL_SRGB, width, height,
               0, GL_RGB, GL_UNSIGNED_BYTE, &pixData[0]);

  if (!g_Gl2Compatible)
    glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  checkGlErrors();
}
