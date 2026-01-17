#include "stdafx.h"
#include "FBXMesh.h"



FBXMesh::FBXMesh()
{
	pData_ = new MeshData;
}

FBXMesh::~FBXMesh()
{
	CleanUp();
}


void FBXMesh::CleanUp()
{
	FbxNode;
	FbxMesh;
	
	if (pData_)
	{
		delete pData_;
		pData_ = nullptr;
	}
}
