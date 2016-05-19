//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Seismic.h"
#include "GeometryUtil.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCallback.h>

#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>

#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureUnit.h>
#include <Inventor/nodes/SoBufferedShape.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoFragmentShader.h>

#include <Inventor/devices/SoCpuBufferObject.h>
#include <Inventor/SbPlane.h>

#include <LDM/readers/SoVolumeReader.h>
#include <LDM/SoLDMDataAccess.h>

#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoDataRange.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/nodes/SoVolumeBufferedShape.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include <VolumeViz/readers/SoVRSegyFileReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>

#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>

#include <algorithm>

namespace
{
  const float defaultColorMap[] = 
  {
    0.000000f, 1.000000f, 1.000000f, 1.000000f, 0.000000f, 0.984367f, 1.000000f, 1.000000f,
    0.000000f, 0.968734f, 1.000000f, 1.000000f, 0.000000f, 0.953101f, 1.000000f, 1.000000f,
    0.000000f, 0.937468f, 1.000000f, 1.000000f, 0.000000f, 0.921835f, 1.000000f, 1.000000f,
    0.000000f, 0.906202f, 1.000000f, 1.000000f, 0.000000f, 0.890570f, 1.000000f, 1.000000f,
    0.000000f, 0.874937f, 1.000000f, 1.000000f, 0.000000f, 0.859304f, 1.000000f, 1.000000f,
    0.000000f, 0.843671f, 1.000000f, 1.000000f, 0.000000f, 0.828038f, 1.000000f, 1.000000f,
    0.000000f, 0.812405f, 1.000000f, 1.000000f, 0.000000f, 0.796772f, 1.000000f, 1.000000f,
    0.000000f, 0.781139f, 1.000000f, 1.000000f, 0.000000f, 0.765506f, 1.000000f, 1.000000f,
    0.000000f, 0.749873f, 1.000000f, 1.000000f, 0.000000f, 0.734240f, 1.000000f, 1.000000f,
    0.000000f, 0.718607f, 1.000000f, 1.000000f, 0.000000f, 0.702974f, 1.000000f, 1.000000f,
    0.000000f, 0.687341f, 1.000000f, 1.000000f, 0.000000f, 0.671708f, 1.000000f, 1.000000f,
    0.000000f, 0.656075f, 1.000000f, 1.000000f, 0.000000f, 0.640442f, 1.000000f, 1.000000f,
    0.000000f, 0.624810f, 1.000000f, 1.000000f, 0.000000f, 0.609177f, 1.000000f, 1.000000f,
    0.000000f, 0.593544f, 1.000000f, 1.000000f, 0.000000f, 0.577911f, 1.000000f, 1.000000f,
    0.000000f, 0.562278f, 1.000000f, 1.000000f, 0.000000f, 0.546645f, 1.000000f, 1.000000f,
    0.000000f, 0.531012f, 1.000000f, 1.000000f, 0.000000f, 0.515379f, 1.000000f, 1.000000f,
    0.000000f, 0.499746f, 1.000000f, 1.000000f, 0.000000f, 0.484113f, 1.000000f, 1.000000f,
    0.000000f, 0.468480f, 1.000000f, 1.000000f, 0.000000f, 0.452847f, 1.000000f, 1.000000f,
    0.000000f, 0.437214f, 1.000000f, 1.000000f, 0.000000f, 0.421581f, 1.000000f, 1.000000f,
    0.000000f, 0.405948f, 1.000000f, 1.000000f, 0.000000f, 0.390316f, 1.000000f, 1.000000f,
    0.000000f, 0.374683f, 1.000000f, 1.000000f, 0.000000f, 0.359050f, 1.000000f, 1.000000f,
    0.000000f, 0.343417f, 1.000000f, 1.000000f, 0.000000f, 0.327784f, 1.000000f, 1.000000f,
    0.000000f, 0.312151f, 1.000000f, 1.000000f, 0.000000f, 0.296518f, 1.000000f, 1.000000f,
    0.000000f, 0.280885f, 1.000000f, 1.000000f, 0.000000f, 0.265252f, 1.000000f, 1.000000f,
    0.000000f, 0.249619f, 1.000000f, 1.000000f, 0.000000f, 0.233986f, 1.000000f, 1.000000f,
    0.000000f, 0.218353f, 1.000000f, 1.000000f, 0.000000f, 0.202720f, 1.000000f, 1.000000f,
    0.000000f, 0.187087f, 1.000000f, 1.000000f, 0.000000f, 0.171454f, 1.000000f, 1.000000f,
    0.000000f, 0.155822f, 1.000000f, 1.000000f, 0.000000f, 0.140189f, 1.000000f, 1.000000f,
    0.000000f, 0.124556f, 1.000000f, 1.000000f, 0.000000f, 0.108923f, 1.000000f, 1.000000f,
    0.000000f, 0.093290f, 1.000000f, 1.000000f, 0.000000f, 0.077657f, 1.000000f, 1.000000f,
    0.000000f, 0.062024f, 1.000000f, 1.000000f, 0.000000f, 0.046391f, 1.000000f, 1.000000f,
    0.000000f, 0.030758f, 1.000000f, 1.000000f, 0.000000f, 0.015125f, 1.000000f, 1.000000f,
    0.000260f, 0.000247f, 0.999235f, 1.000000f, 0.008270f, 0.007838f, 0.975691f, 1.000000f,
    0.016279f, 0.015430f, 0.952147f, 1.000000f, 0.024289f, 0.023021f, 0.928603f, 1.000000f,
    0.032298f, 0.030613f, 0.905059f, 1.000000f, 0.040308f, 0.038204f, 0.881515f, 1.000000f,
    0.048317f, 0.045796f, 0.857971f, 1.000000f, 0.056327f, 0.053387f, 0.834427f, 1.000000f,
    0.064336f, 0.060979f, 0.810883f, 1.000000f, 0.072346f, 0.068570f, 0.787339f, 1.000000f,
    0.080355f, 0.076162f, 0.763795f, 1.000000f, 0.088365f, 0.083753f, 0.740251f, 1.000000f,
    0.096374f, 0.091344f, 0.716707f, 1.000000f, 0.104384f, 0.098936f, 0.693163f, 1.000000f,
    0.112393f, 0.106527f, 0.669619f, 1.000000f, 0.120403f, 0.114119f, 0.646075f, 1.000000f,
    0.128412f, 0.121711f, 0.622531f, 1.000000f, 0.136422f, 0.129302f, 0.598987f, 1.000000f,
    0.144432f, 0.136893f, 0.575443f, 1.000000f, 0.152441f, 0.144485f, 0.551899f, 1.000000f,
    0.160451f, 0.152076f, 0.528355f, 1.000000f, 0.168460f, 0.159668f, 0.504811f, 1.000000f,
    0.176470f, 0.167259f, 0.481267f, 1.000000f, 0.184479f, 0.174851f, 0.457723f, 1.000000f,
    0.192489f, 0.182443f, 0.434179f, 1.000000f, 0.200498f, 0.190034f, 0.410635f, 1.000000f,
    0.208508f, 0.197626f, 0.387091f, 1.000000f, 0.216517f, 0.205217f, 0.363547f, 1.000000f,
    0.224527f, 0.212809f, 0.340003f, 1.000000f, 0.232536f, 0.220400f, 0.316459f, 1.000000f,
    0.240546f, 0.227992f, 0.292915f, 1.000000f, 0.248555f, 0.235583f, 0.269371f, 1.000000f,
    0.256565f, 0.243174f, 0.245827f, 1.000000f, 0.272406f, 0.258598f, 0.260565f, 1.000000f,
    0.288319f, 0.274093f, 0.275652f, 1.000000f, 0.304231f, 0.289589f, 0.290740f, 1.000000f,
    0.320144f, 0.305084f, 0.305827f, 1.000000f, 0.336057f, 0.320579f, 0.320915f, 1.000000f,
    0.351970f, 0.336074f, 0.336002f, 1.000000f, 0.367882f, 0.351569f, 0.351089f, 1.000000f,
    0.383795f, 0.367064f, 0.366177f, 1.000000f, 0.399708f, 0.382559f, 0.381264f, 1.000000f,
    0.415621f, 0.398055f, 0.396351f, 1.000000f, 0.431533f, 0.413550f, 0.411439f, 1.000000f,
    0.447446f, 0.429045f, 0.426526f, 1.000000f, 0.463359f, 0.444540f, 0.441613f, 1.000000f,
    0.479272f, 0.460035f, 0.456701f, 1.000000f, 0.495185f, 0.475530f, 0.471788f, 1.000000f,
    0.511097f, 0.491025f, 0.486875f, 1.000000f, 0.527010f, 0.506521f, 0.501963f, 1.000000f,
    0.542923f, 0.522016f, 0.517050f, 1.000000f, 0.558836f, 0.537511f, 0.532138f, 1.000000f,
    0.574748f, 0.553006f, 0.547225f, 1.000000f, 0.590661f, 0.568501f, 0.562312f, 1.000000f,
    0.606574f, 0.583996f, 0.577399f, 1.000000f, 0.622486f, 0.599491f, 0.592487f, 1.000000f,
    0.638399f, 0.614986f, 0.607574f, 1.000000f, 0.654312f, 0.630482f, 0.622662f, 1.000000f,
    0.670225f, 0.645977f, 0.637749f, 1.000000f, 0.686137f, 0.661472f, 0.652836f, 1.000000f,
    0.702050f, 0.676967f, 0.667924f, 1.000000f, 0.717963f, 0.692462f, 0.683011f, 1.000000f,
    0.733876f, 0.707957f, 0.698098f, 1.000000f, 0.749788f, 0.723452f, 0.713186f, 1.000000f,
    0.765388f, 0.738542f, 0.727764f, 1.000000f, 0.758071f, 0.723925f, 0.705046f, 1.000000f,
    0.750753f, 0.709307f, 0.682328f, 1.000000f, 0.743436f, 0.694689f, 0.659610f, 1.000000f,
    0.736118f, 0.680071f, 0.636891f, 1.000000f, 0.728800f, 0.665453f, 0.614173f, 1.000000f,
    0.721483f, 0.650836f, 0.591455f, 1.000000f, 0.714165f, 0.636218f, 0.568737f, 1.000000f,
    0.706848f, 0.621600f, 0.546019f, 1.000000f, 0.699530f, 0.606982f, 0.523301f, 1.000000f,
    0.692212f, 0.592364f, 0.500582f, 1.000000f, 0.684895f, 0.577747f, 0.477864f, 1.000000f,
    0.677577f, 0.563129f, 0.455146f, 1.000000f, 0.670260f, 0.548511f, 0.432428f, 1.000000f,
    0.662942f, 0.533893f, 0.409710f, 1.000000f, 0.655624f, 0.519275f, 0.386991f, 1.000000f,
    0.648307f, 0.504658f, 0.364273f, 1.000000f, 0.640989f, 0.490040f, 0.341555f, 1.000000f,
    0.633672f, 0.475422f, 0.318837f, 1.000000f, 0.626354f, 0.460804f, 0.296119f, 1.000000f,
    0.619036f, 0.446187f, 0.273400f, 1.000000f, 0.611719f, 0.431569f, 0.250682f, 1.000000f,
    0.604401f, 0.416951f, 0.227964f, 1.000000f, 0.597083f, 0.402333f, 0.205246f, 1.000000f,
    0.589766f, 0.387715f, 0.182528f, 1.000000f, 0.582448f, 0.373098f, 0.159809f, 1.000000f,
    0.575131f, 0.358480f, 0.137091f, 1.000000f, 0.567813f, 0.343862f, 0.114373f, 1.000000f,
    0.560495f, 0.329244f, 0.091655f, 1.000000f, 0.553178f, 0.314627f, 0.068937f, 1.000000f,
    0.545860f, 0.300009f, 0.046219f, 1.000000f, 0.538543f, 0.285391f, 0.023500f, 1.000000f,
    0.531225f, 0.270773f, 0.000782f, 1.000000f, 0.545155f, 0.272177f, 0.000000f, 1.000000f,
    0.559842f, 0.274152f, 0.000000f, 1.000000f, 0.574529f, 0.276126f, 0.000000f, 1.000000f,
    0.589217f, 0.278101f, 0.000000f, 1.000000f, 0.603904f, 0.280076f, 0.000000f, 1.000000f,
    0.618591f, 0.282051f, 0.000000f, 1.000000f, 0.633278f, 0.284025f, 0.000000f, 1.000000f,
    0.647966f, 0.286000f, 0.000000f, 1.000000f, 0.662653f, 0.287975f, 0.000000f, 1.000000f,
    0.677340f, 0.289950f, 0.000000f, 1.000000f, 0.692028f, 0.291925f, 0.000000f, 1.000000f,
    0.706715f, 0.293899f, 0.000000f, 1.000000f, 0.721402f, 0.295874f, 0.000000f, 1.000000f,
    0.736090f, 0.297849f, 0.000000f, 1.000000f, 0.750777f, 0.299824f, 0.000000f, 1.000000f,
    0.765464f, 0.301798f, 0.000000f, 1.000000f, 0.780152f, 0.303773f, 0.000000f, 1.000000f,
    0.794839f, 0.305748f, 0.000000f, 1.000000f, 0.809526f, 0.307723f, 0.000000f, 1.000000f,
    0.824214f, 0.309698f, 0.000000f, 1.000000f, 0.838901f, 0.311672f, 0.000000f, 1.000000f,
    0.853588f, 0.313647f, 0.000000f, 1.000000f, 0.868276f, 0.315622f, 0.000000f, 1.000000f,
    0.882963f, 0.317597f, 0.000000f, 1.000000f, 0.897650f, 0.319572f, 0.000000f, 1.000000f,
    0.912337f, 0.321546f, 0.000000f, 1.000000f, 0.927025f, 0.323521f, 0.000000f, 1.000000f,
    0.941712f, 0.325496f, 0.000000f, 1.000000f, 0.956399f, 0.327471f, 0.000000f, 1.000000f,
    0.971087f, 0.329446f, 0.000000f, 1.000000f, 0.985774f, 0.331420f, 0.000000f, 1.000000f,
    1.000000f, 0.333665f, 0.000000f, 1.000000f, 1.000000f, 0.344242f, 0.000000f, 1.000000f,
    1.000000f, 0.354819f, 0.000000f, 1.000000f, 1.000000f, 0.365395f, 0.000000f, 1.000000f,
    1.000000f, 0.375972f, 0.000000f, 1.000000f, 1.000000f, 0.386549f, 0.000000f, 1.000000f,
    1.000000f, 0.397126f, 0.000000f, 1.000000f, 1.000000f, 0.407702f, 0.000000f, 1.000000f,
    1.000000f, 0.418279f, 0.000000f, 1.000000f, 1.000000f, 0.428856f, 0.000000f, 1.000000f,
    1.000000f, 0.439433f, 0.000000f, 1.000000f, 1.000000f, 0.450009f, 0.000000f, 1.000000f,
    1.000000f, 0.460586f, 0.000000f, 1.000000f, 1.000000f, 0.471163f, 0.000000f, 1.000000f,
    1.000000f, 0.481740f, 0.000000f, 1.000000f, 1.000000f, 0.492316f, 0.000000f, 1.000000f,
    1.000000f, 0.502893f, 0.000000f, 1.000000f, 1.000000f, 0.513470f, 0.000000f, 1.000000f,
    1.000000f, 0.524047f, 0.000000f, 1.000000f, 1.000000f, 0.534623f, 0.000000f, 1.000000f,
    1.000000f, 0.545200f, 0.000000f, 1.000000f, 1.000000f, 0.555777f, 0.000000f, 1.000000f,
    1.000000f, 0.566354f, 0.000000f, 1.000000f, 1.000000f, 0.576930f, 0.000000f, 1.000000f,
    1.000000f, 0.587507f, 0.000000f, 1.000000f, 1.000000f, 0.598084f, 0.000000f, 1.000000f,
    1.000000f, 0.608661f, 0.000000f, 1.000000f, 1.000000f, 0.619237f, 0.000000f, 1.000000f,
    1.000000f, 0.629814f, 0.000000f, 1.000000f, 1.000000f, 0.640391f, 0.000000f, 1.000000f,
    1.000000f, 0.650967f, 0.000000f, 1.000000f, 1.000000f, 0.661544f, 0.000000f, 1.000000f,
    1.000000f, 0.672121f, 0.000000f, 1.000000f, 1.000000f, 0.682698f, 0.000000f, 1.000000f,
    1.000000f, 0.693274f, 0.000000f, 1.000000f, 1.000000f, 0.703851f, 0.000000f, 1.000000f,
    1.000000f, 0.714428f, 0.000000f, 1.000000f, 1.000000f, 0.725005f, 0.000000f, 1.000000f,
    1.000000f, 0.735581f, 0.000000f, 1.000000f, 1.000000f, 0.746158f, 0.000000f, 1.000000f,
    1.000000f, 0.756735f, 0.000000f, 1.000000f, 1.000000f, 0.767312f, 0.000000f, 1.000000f,
    1.000000f, 0.777888f, 0.000000f, 1.000000f, 1.000000f, 0.788465f, 0.000000f, 1.000000f,
    1.000000f, 0.799042f, 0.000000f, 1.000000f, 1.000000f, 0.809619f, 0.000000f, 1.000000f,
    1.000000f, 0.820195f, 0.000000f, 1.000000f, 1.000000f, 0.830772f, 0.000000f, 1.000000f,
    1.000000f, 0.841349f, 0.000000f, 1.000000f, 1.000000f, 0.851926f, 0.000000f, 1.000000f,
    1.000000f, 0.862502f, 0.000000f, 1.000000f, 1.000000f, 0.873079f, 0.000000f, 1.000000f,
    1.000000f, 0.883656f, 0.000000f, 1.000000f, 1.000000f, 0.894233f, 0.000000f, 1.000000f,
    1.000000f, 0.904809f, 0.000000f, 1.000000f, 1.000000f, 0.915386f, 0.000000f, 1.000000f,
    1.000000f, 0.925963f, 0.000000f, 1.000000f, 1.000000f, 0.936539f, 0.000000f, 1.000000f,
    1.000000f, 0.947116f, 0.000000f, 1.000000f, 1.000000f, 0.957693f, 0.000000f, 1.000000f,
    1.000000f, 0.968270f, 0.000000f, 1.000000f, 1.000000f, 0.978846f, 0.000000f, 1.000000f,
    1.000000f, 0.989423f, 0.000000f, 1.000000f, 1.000000f, 1.000000f, 0.000000f, 1.000000f
 };

  const char* fragGetDataSrc = R"%%%(
  //!oiv_include <VolumeViz/vvizGetData_frag.h>

  VVIZ_DATATYPE VVizGetData(in VVizDataSetId dataset, in vec3 dataCoord)
  {
    if(
      any(lessThan(dataCoord, vec3(0.0, 0.0, 0.0))) ||
      any(greaterThan(dataCoord, vec3(1.0, 1.0, 1.0))))
    {
      return 0.5; // TODO: make this a uniform variable?
    }
    else
    {
      return VVizGetRawData(dataset, dataCoord);
    }
  }
  )%%%";

  const char* fragGetColorSrc = R"%%%(
  //!oiv_include <VolumeViz/vvizGetData_frag.h>           // The shader API lets you include 
  //!oiv_include <VolumeViz/vvizTransferFunction_frag.h>	// declarations of shader functions

  varying in vec3 texcoord;
  varying in float depth;

  uniform VVizDataSetId data1;  // Data texture of 1st volume
  uniform float isolineInterval;
  uniform bool enableIsoline;

  // Method in VolumeViz shader framework to override for custom color computation
  vec4 VVizComputeFragmentColor(VVIZ_DATATYPE vox, vec3 coord)
  {
    VVIZ_DATATYPE value1 = VVizGetData(data1, texcoord);      // Value from 1st volume
    vec4 color1 = VVizTransferFunction(value1, 0);         // Color for 1st volume from TF 0

    if(enableIsoline)
    {
      float dz = fwidth(depth);
      float z = mod(depth + 0.5 * isolineInterval, isolineInterval) - 0.5 * isolineInterval;
      float att = 1.0 - (smoothstep(-dz, -0.5 * dz, z) - smoothstep(0.5 * dz, dz, z));
      color1 = color1 * vec4(att, att, att, 1.0);
    }

    return color1;
  }
  )%%%";

  const char* vertexPPSrc = R"%%%(
  varying out vec3 texcoord;
  varying out float depth;

  uniform mat4 texMat;

  void VVizVertexPostProcessing()
  {
    depth = gl_Vertex.z;
    texcoord = (texMat * gl_MultiTexCoord0).xyz;
  }
  )%%%";

  SoVolumeRenderingQuality* createStretchSqueezeVolumeShader(const SbMatrix& texMat, bool enableIsoline)
  {
    auto fragmentShaderData = new SoFragmentShader;
    fragmentShaderData->sourceProgram = fragGetDataSrc;
    fragmentShaderData->sourceType = SoShaderObject::GLSL_PROGRAM;

    auto fragmentShaderColor = new SoFragmentShader;
    fragmentShaderColor->sourceProgram = fragGetColorSrc;
    fragmentShaderColor->sourceType = SoShaderObject::GLSL_PROGRAM;
    fragmentShaderColor->addShaderParameter1i("data1", 1);
    fragmentShaderColor->addShaderParameter1f("isolineInterval", 100.f);
    fragmentShaderColor->addShaderParameter1i("enableIsoline", enableIsoline ? 1 : 0);

    auto vertexShader = new SoVertexShader;
    vertexShader->sourceProgram = vertexPPSrc;
    vertexShader->sourceType = SoShaderObject::GLSL_PROGRAM;
    vertexShader->addShaderParameterMatrix("texMat", texMat);

    auto program = new SoVolumeRenderingQuality;
    program->forVolumeOnly = false; // only doing 2D rendering
    program->shaderObject.set1Value(SoVolumeShader::GET_DATA_FUNCTION, fragmentShaderData);
    program->shaderObject.set1Value(SoVolumeShader::FRAGMENT_COMPUTE_COLOR, fragmentShaderColor);
    program->shaderObject.set1Value(SoVolumeShader::VERTEX_POSTPROCESSING, vertexShader);
    return program;
  }

  bool loadColorMap(SoTransferFunction* tf, const char* filename)
  {
    std::ifstream in(filename);
    if (!in)
      return false;

    std::string s1, s2;
    int ncolors;
    in >> s1 >> s2 >> ncolors;

    tf->colorMap.setNum(4 * ncolors);
    for(int i=0, j=0; i < ncolors; ++i)
    {
      unsigned int r, g, b;
      in >> r >> g >> b;

      tf->colorMap.set1Value(j++, r / 65535.f);
      tf->colorMap.set1Value(j++, g / 65535.f);
      tf->colorMap.set1Value(j++, b / 65535.f);
      tf->colorMap.set1Value(j++, 1.f);
    }

    tf->colorMapType = SoTransferFunction::RGBA;
    tf->predefColorMap = SoTransferFunction::NONE;

    return true;
  }

  inline int toInt(float v)
  {
    int intval = (int)(65535 * v);
    if (intval < 0)
      intval = 0;

    if (intval > 65535)
      intval = 65535;

    return intval;
  }

  void saveColorMap(SoTransferFunction* transferFunc, const char* filename)
  {
    std::ofstream out(filename);
    if (!out)
      return;

    int n = transferFunc->actualColorMap.getNum() / 4;
    out << "ncolors = " << n << '\n';

    for (int i = 0, j=0; i < n; ++i)
    {
      out
        << toInt(transferFunc->actualColorMap[j++]) << ' '
        << toInt(transferFunc->actualColorMap[j++]) << ' '
        << toInt(transferFunc->actualColorMap[j++]) << '\n';

      j++; // skip alpha
    }
  }
}

void CustomLDMInfo::writeXML(FILE* fp)
{
  SbVec2d p[4] = { p1, p2, p3, p4 };

  const char* customSectionTag = "Shell";
  fprintf(fp, "<%s>\n", customSectionTag);
  fprintf(fp, "  <Coordinates>\n");

  for (int i = 0; i < 4; ++i)
  {
    fprintf(fp, "    <P>\n");
    fprintf(fp, "      <X>%f</X>\n", p[i][0]);
    fprintf(fp, "      <Y>%f</Y>\n", p[i][1]);
    fprintf(fp, "    </P>\n");
  }

  fprintf(fp, "  </Coordinates>\n");

  fprintf(fp, "  <MinDepth>%f</MinDepth>\n", minDepth);
  fprintf(fp, "  <MaxDepth>%f</MaxDepth>\n", maxDepth);

  fprintf(fp, "</%s>\n", customSectionTag);
}

void CustomLDMInfo::readXML(SoVRLdmFileReader* reader)
{
  SbXmlTag rootTag = reader->getXmlTag("Shell");
  
  SbXmlTag coordinatesTag = rootTag.getFirstChildTag();

  SbVec2d p[4];
  SbXmlTag pTag = coordinatesTag.getFirstChildTag();
  for (int i = 0; i < 4; ++i)
  {
    SbXmlTag xTag = pTag.getFirstChildTag();
    p[i][0] = strtod(xTag.getText(), nullptr);
    SbXmlTag yTag = xTag.getNextSiblingTag();
    p[i][1] = strtod(yTag.getText(), nullptr);

    pTag = pTag.getNextSiblingTag();
  }

  p1 = p[0];
  p2 = p[1];
  p3 = p[2];
  p4 = p[3];

  SbXmlTag minDepthTag = coordinatesTag.getNextSiblingTag();
  minDepth = strtod(minDepthTag.getText(), nullptr);
  SbXmlTag maxDepthTag = minDepthTag.getNextSiblingTag();
  maxDepth = strtod(maxDepthTag.getText(), nullptr);
}

void SeismicScene::computeVolumeTransform(SoVolumeReader* reader)
{
  SbVec2d p1, p2, p3, p4;
  double minDepth;
  double maxDepth;

  if (reader->getTypeId() == SoVRSegyFileReader::getClassTypeId())
  {
    SoVRSegyFileReader* segyReader = static_cast<SoVRSegyFileReader*>(reader);
    //auto header = segyReader->getSegyTextHeader();

    segyReader->getP1P2P3Coordinates(p1, p2, p3, p4);
    maxDepth = 3722.5;
    minDepth = -1.49;
  }
  else if (reader->getTypeId() == SoVRLdmFileReader::getClassTypeId())
  {
    SoVRLdmFileReader* ldmReader = static_cast<SoVRLdmFileReader*>(reader);
    
    CustomLDMInfo ldmInfo;
    ldmInfo.readXML(ldmReader);
    
    p1 = ldmInfo.p1;
    p2 = ldmInfo.p2;
    p3 = ldmInfo.p3;
    p4 = ldmInfo.p4;

    minDepth = ldmInfo.minDepth;
    maxDepth = ldmInfo.maxDepth;
  }

  auto d1 = p2 - p1;
  auto d2 = p3 - p2;

  float w = (float)(maxDepth - minDepth);
  float h = (float)d1.length();
  float d = (float)d2.length();
  m_data->extent = SbBox3f(0.f, 0.f, 0.f, w, h, d);
  m_normalizeTransform.setScale(SbVec3f(1 / w, 1 / h, 1 / d));

  SbMatrix scaleMatrix;
  float zscale = (d1[0] * d2[1] - d1[1] * d2[0] > 0.0) ? -1.f : 1.f;
  scaleMatrix.setScale(SbVec3f(1.f, 1.f, zscale)); // mirror

  const SbVec3f xAxis(1.f, 0.f, 0.f);
  const SbVec3f yAxis(0.f, 1.f, 0.f);
  const SbVec3f zAxis(0.f, 0.f, 1.f);

  SbMatrix rotMatrix1;
  rotMatrix1.setRotate(SbRotation(xAxis, -zAxis));

  d1.normalize();
  SbMatrix rotMatrix2;
  rotMatrix2.setRotate(SbRotation(yAxis, SbVec3f((float)d1[0], (float)d1[1], 0.f)));

  SbMatrix translateMatrix;
  translateMatrix.setTranslate(
    SbVec3f(
    (float)(p1[0] - m_dimensions.minX),
    (float)(p1[1] - m_dimensions.minY),
    (float)-minDepth));

  m_seismicTransform = scaleMatrix * rotMatrix1 * rotMatrix2 * translateMatrix;
  m_invSeismicTransform = m_seismicTransform.inverse();
  m_matrixTransform->matrix = m_seismicTransform;
}

void SeismicScene::createCrossSection(PlaneSlice& slice)
{
  if (!m_mesh)
    return;

  assert(m_presentDayMesh);

  auto const& geometry0 = m_presentDayMesh->getGeometry();
  auto const& topology0 = m_presentDayMesh->getTopology();
  auto const& geometry  = m_mesh->getGeometry();
  auto const& topology  = m_mesh->getTopology();

  double w = m_dimensions.numCellsI * m_dimensions.deltaX;
  double h = m_dimensions.numCellsJ * m_dimensions.deltaY;
  SbBox2d box(0.0, 0.0, w, h);
  SbVec2d p0 = slice.p0;
  SbVec2d p1 = slice.p1;

  if (!clip(p0, p1, box))
    return;

  SbVec2d dir = p1 - p0;
  dir.normalize();
  SbVec3f normal((float)dir[1], (float)-dir[0], 0.f);

  slice.points = computeGridLineIntersections(p0, p1, m_dimensions.deltaX, m_dimensions.deltaY);

  auto dim = m_dimensions;
  auto newEnd = std::remove_if(
    slice.points.begin(), 
    slice.points.end(), 
    [&geometry, &dim](const SbVec2d& p)
    {
      bool ok = true;
      getZ(
        geometry, 
        dim.numCellsI, 
        dim.numCellsJ, 
        p[0] / dim.deltaX, 
        p[1] / dim.deltaY, 0, 
        &ok);

      return !ok;
    });

  slice.points.erase(newEnd, slice.points.end());
  if (slice.points.empty())
    return;

  size_t nk  = topology.getNumCellsK();
  size_t nk0 = topology0.getNumCellsK();
  assert(nk0 >= nk);
  size_t npoints = slice.points.size();
  size_t nverts = npoints * (nk + 1);
  size_t nquads = (npoints - 1) * nk;

  // Init texcoord buffer
  SoCpuBufferObject* tb = new SoCpuBufferObject;
  tb->setSize(nverts * 3 * sizeof(float));
  float* texcoords = (float*)tb->map(SoBufferObject::SET);

  // Init normals buffer
  SoCpuBufferObject* nb = new SoCpuBufferObject;
  nb->setSize(nverts * 3 * sizeof(float));
  float* normals = (float*)nb->map(SoBufferObject::SET);

  // Init vertex buffer
  SoCpuBufferObject* vb = new SoCpuBufferObject;
  vb->setSize(nverts * 3 * sizeof(float));
  float* verts = (float*)vb->map(SoBufferObject::SET);

  for (int k = (int)nk, k0 = (int)nk0; k >= 0; --k, --k0)
  {
    for (auto p : slice.points)
    {
      double i = p[0] / dim.deltaX;
      double j = p[1] / dim.deltaY;
      double z  = getZ(geometry,  dim.numCellsI, dim.numCellsJ, i, j, k);
      double z0 = getZ(geometry0, dim.numCellsI, dim.numCellsJ, i, j, k0);

      *verts++ = (float)p[0];
      *verts++ = (float)p[1];
      *verts++ = (float)z;

      *texcoords++ = (float)p[0];
      *texcoords++ = (float)p[1];
      *texcoords++ = (float)z0;

      *normals++ = normal[0];
      *normals++ = normal[1];
      *normals++ = normal[2];
    }
  }

  vb->unmap();
  tb->unmap();
  nb->unmap();

  // Init index buffer
  SoCpuBufferObject* ib = new SoCpuBufferObject;
  ib->setSize(nquads * 4 * sizeof(uint32_t));
  int* indices = (int*)ib->map(SoBufferObject::SET);
  for (size_t k = 0; k < nk; ++k)
  {
    for (size_t i = 0; i < npoints - 1; ++i)
    {
      uint32_t base = (uint32_t)(k * npoints + i);
      *indices++ = base;
      *indices++ = base + 1;
      *indices++ = (uint32_t)(base + 1 + npoints);
      *indices++ = (uint32_t)(base + npoints);
    }
  }

  ib->unmap();

  auto* shape = new SoVolumeBufferedShape;
  shape->vertexBuffer = vb;
  shape->vertexComponentsCount = 3;
  shape->vertexComponentsType = SbDataType::FLOAT;

  shape->texCoordsBuffer = tb;
  shape->texCoordsComponentsCount = 3;
  shape->texCoordsComponentsType = SbDataType::FLOAT;

  shape->normalBuffer = nb;
  shape->normalComponentsType = SbDataType::FLOAT;

  shape->indexBuffer = ib;
  shape->indexType = SbDataType::UNSIGNED_INT32;
  shape->shapeType = SoBufferedShape::QUADS;
  shape->numVertices = 4 * (int)nquads;
  shape->useNormalsGenerator = false;

  slice.vertices = vb;
  slice.texcoords = tb;
  slice.normals = nb;
  slice.indices = ib;
  slice.shape = shape;
}

void SeismicScene::updateSurface()
{
  if (m_surface.shape)
  {
    m_surfaceGroup->removeChild(m_surface.shape);
    m_surface.shape = nullptr;
  }

  if (!m_mesh)
    return;

  if (m_surface.enabled)
  {
    if (!m_surface.shape)
    {
      m_surface.vertices = new SoCpuBufferObject;
      m_surface.texcoords = new SoCpuBufferObject;

      auto indices = computeSurfaceIndices(m_mesh->getTopology());
      if (indices.empty())
        return;

      m_surface.indices = new SoCpuBufferObject;
      m_surface.indices->setSize(indices.size() * sizeof(uint32_t));
      uint32_t* p = (uint32_t*)m_surface.indices->map(SoBufferObject::SET);
      memcpy(p, indices.data(), indices.size() * sizeof(uint32_t));
      m_surface.indices->unmap();

      m_surface.shape = new SoVolumeBufferedShape;
      m_surface.shape->shapeType = SoBufferedShape::QUADS;
      m_surface.shape->numVertices = (uint32_t)indices.size();

    }

    if (
      !computeSurfaceCoordinates(*m_mesh, m_surface.position, m_surface.vertices.ptr()) ||
      !computeSurfaceCoordinates(*m_presentDayMesh, m_surface.position, m_surface.texcoords.ptr()))
    {
      return;
    }

    m_surface.shape->vertexBuffer = m_surface.vertices.ptr();
    m_surface.shape->vertexComponentsCount = 3;
    m_surface.shape->vertexComponentsType = SbDataType::FLOAT;

    m_surface.shape->texCoordsBuffer = m_surface.texcoords.ptr();
    m_surface.shape->texCoordsComponentsCount = 3;
    m_surface.shape->texCoordsComponentsType = SbDataType::FLOAT;

    m_surface.shape->indexBuffer = m_surface.indices.ptr();
    m_surface.shape->indexType = SbDataType::UNSIGNED_INT32;
    m_surfaceGroup->addChild(m_surface.shape);
  }
}

void SeismicScene::updatePlaneSlice(int index)
{
  PlaneSlice& slice = m_planeSlice[index];

  // remove previous shape
  if (slice.shape)
  {
    m_sliceGroup->removeChild(slice.shape);
    slice.shape = nullptr;
  }

  if (!m_mesh)
    return;

  auto extent = m_data->extent.getValue();

  SbVec3f p0, p1;
  if (slice.type == SliceInline)
  {
    p0 = SbVec3f(0.f, slice.position, extent.getMin()[2]);
    p1 = SbVec3f(0.f, slice.position, extent.getMax()[2]);
  }
  else
  {
    p0 = SbVec3f(0.f, extent.getMin()[1], slice.position);
    p1 = SbVec3f(0.f, extent.getMax()[1], slice.position);
  }

  m_seismicTransform.multVecMatrix(p0, p0);
  m_seismicTransform.multVecMatrix(p1, p1);

  slice.p0 = SbVec2d(p0[0], p0[1]);
  slice.p1 = SbVec2d(p1[0], p1[1]);

  if (slice.enabled)
  {
    createCrossSection(slice);
    m_sliceGroup->addChild(slice.shape);
  }
}

SeismicScene::SeismicScene(const char* filename, const Project::Dimensions& dim)
: m_mesh(nullptr)
, m_dimensions(dim)
, m_root(new SoSeparator)
, m_sliceGroup(new SoSeparator)
, m_surfaceGroup(new SoSeparator)
, m_sliceShader(nullptr)
, m_surfaceShader(nullptr)
, m_transformSeparator(new SoTransformSeparator)
, m_matrixTransform(new SoMatrixTransform)
, m_data(new SoVolumeData)
, m_material(new SoMaterial)
, m_transferFunction(new SoTransferFunction)
, m_range(new SoDataRange)
{
  double rangeMin = -2e4, rangeMax = 2e4;

  auto reader = SoVolumeReader::getAppropriateReader(filename);
  m_data->setReader(*reader, true);
  m_data->dataSetId = 1;

  auto params = new SoLDMResourceParameters;
  params->loadPolicy = SoLDMResourceParameters::ALWAYS;
  //params->fixedResolution = true;
  //params->resolution = 2;
  m_data->ldmResourceParameters = params;

  m_material->diffuseColor.setValue(SbColor(1.f, 1.f, 1.f));
  m_transferFunction->colorMap.setValues(0, 1024, defaultColorMap);
  m_transferFunction->predefColorMap = SoTransferFunction::NONE;
  
  m_range->min = rangeMin;
  m_range->max = rangeMax;
  std::cout << "range = { " << rangeMin << " : " << rangeMax << " }" << std::endl;

  // Build scene graph
  m_transformSeparator->addChild(m_matrixTransform);
  m_transformSeparator->addChild(m_data);
  m_transformSeparator->addChild(m_material);
  m_transformSeparator->addChild(m_transferFunction);
  m_transformSeparator->addChild(m_range);
  m_root->addChild(m_transformSeparator);

  computeVolumeTransform(reader);

  m_sliceShader = createStretchSqueezeVolumeShader(m_invSeismicTransform * m_normalizeTransform, false);
  m_sliceGroup->addChild(m_sliceShader);

  m_surfaceShader = createStretchSqueezeVolumeShader(m_invSeismicTransform * m_normalizeTransform, true);
  m_surfaceGroup->addChild(m_surfaceShader);

  m_root->addChild(m_sliceGroup);
  m_root->addChild(m_surfaceGroup);

  m_planeSlice[0].type = SliceInline;
  m_planeSlice[1].type = SliceCrossline;
}

void SeismicScene::setMesh(const MiVolumeMeshCurvilinear* mesh)
{
  if (mesh != m_mesh)
  {
    m_mesh = mesh;

    if (!m_presentDayMesh)
      m_presentDayMesh.reset(m_mesh->getNewClone());

    for(int i=0; i < 2; ++i)
      updatePlaneSlice(i);

    updateSurface();
  }
}

SoNode* SeismicScene::getRoot() const
{
  return m_root;
}

SbVec3i32 SeismicScene::getDimensions() const
{
  return m_data->data.getSize();
}

SbBox3f SeismicScene::getExtent() const
{
  return m_data->extent.getValue();
}

void SeismicScene::enableSlice(SliceType type, bool value)
{
  int index = 0;
  switch (type)
  {
  case SliceInline: index = 0; break;
  case SliceCrossline: index = 1; break;
  }

  if (m_planeSlice[index].enabled != value)
  {
    m_planeSlice[index].enabled = value;
    updatePlaneSlice(index);
  }
}

void SeismicScene::setSlicePosition(SliceType type, float position)
{
  int index = 0;
  switch (type)
  {
  case SliceInline: index = 0; break;
  case SliceCrossline: index = 1; break;
  }

  if (m_planeSlice[index].position != position)
  {
    m_planeSlice[index].position = position;
    updatePlaneSlice(index);
  }
}

void SeismicScene::enableInterpolatedSurface(bool enabled)
{
  if (enabled != m_surface.enabled)
  {
    m_surface.enabled = enabled;
    updateSurface();
  }
}

void SeismicScene::setInterpolatedSurfacePosition(float position)
{
  if (position != m_surface.position)
  {
    m_surface.position = position;
    updateSurface();
  }
}

void SeismicScene::setDataRange(double rangeMin, double rangeMax)
{
  m_range->min = rangeMin;
  m_range->max = rangeMax;
}

void SeismicScene::loadColorMap(const char* filename)
{
  const char* extension = strrchr(filename, '.');
  if (!extension)
    return;

  if (!strcmp(extension, ".cmap"))
  {
    ::loadColorMap(m_transferFunction, filename);
  }
  else if (!strcmp(extension, ".col") || !strcmp(extension, ".am"))
  {
    m_transferFunction->loadColormap(filename);
  }

  m_transferFunction->predefColorMap = SoTransferFunction::NONE;
}

