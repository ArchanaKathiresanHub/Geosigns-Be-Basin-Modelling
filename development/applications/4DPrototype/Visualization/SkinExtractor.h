//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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

  std::shared_ptr<MiSurfaceMeshUnstructured> m_skin;

public:

  explicit SkinExtractor(const BpaMesh& mesh);

  const MiSurfaceMeshUnstructured& extractSkin(const MiCellFilterIjk* cellFilter = 0);

  static MiSurfaceMeshUnstructured* doSkinExtraction(const BpaMesh& mesh, const MiCellFilterIjk* cellFilter);
};


#endif
