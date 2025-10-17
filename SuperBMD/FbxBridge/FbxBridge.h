#pragma once
#include <fbxsdk.h>
#include <string>

namespace FbxBridge
{
    public ref class FbxSceneWriter
    {
    public:
        FbxSceneWriter();
        ~FbxSceneWriter();
        !FbxSceneWriter();

        void CreateScene(System::String^ name);

        void AddMeshWithMaterial(
            System::String^ nodeName,
            array<float>^ verts,
            array<int>^ indices,
            array<float>^ uvs,
            array<float>^ normals,
            System::String^ texturePath);

		void Save(System::String^ outPath, bool ascii, bool embeddedTextures);

    private:
        FbxManager* mMgr = nullptr;
        FbxScene* mScene = nullptr;

        static std::string ToUtf8(System::String^ s);
        static void EnsureLayer(FbxMesh* mesh);
    };
}
