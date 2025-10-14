#pragma once

using namespace System;

namespace FbxBridge {

    public ref class FbxSceneWriter
    {
    public:
        FbxSceneWriter();
        ~FbxSceneWriter();

        void CreateScene(String^ name);
        // verts: xyz repeated, uvs: uv repeated, both length must match, indices are triangles
        void AddMeshWithMaterial(String^ nodeName,
            array<float>^ verts,
            array<int>^ indices,
            array<float>^ uvs,
            String^ texturePath);
        void Save(String^ outPath, bool ascii);

    private:
        FbxManager* mMgr = nullptr;
        FbxScene* mScene = nullptr;
    };

}