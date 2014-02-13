#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#ifdef VISUALIZATIONDLL_EXPORTS
#define VISUALIZATIONDLL_API __declspec(dllexport) 
#else
#define VISUALIZATIONDLL_API __declspec(dllimport) 
#endif

class SoNode;

VISUALIZATIONDLL_API void foo(const char* filename);

VISUALIZATIONDLL_API SoNode* createOIVTree();

class Mesh
{
};

#endif
