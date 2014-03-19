#ifndef SKINEXTRACTOR_H_INCLUDED
#define SKINEXTRACTOR_H_INCLUDED

#include <memory>

class BpaMesh;
class SurfaceMeshUnstructured;
class MiSurfaceMeshUnstructured;
class MiCellFilterIjk;

class SkinExtractor
{
  const BpaMesh& m_mesh;

  std::shared_ptr<SurfaceMeshUnstructured> m_skin;

public:

  explicit SkinExtractor(const BpaMesh& mesh);

  const MiSurfaceMeshUnstructured& extractSkin(const MiCellFilterIjk* cellFilter = 0);
};


#endif
