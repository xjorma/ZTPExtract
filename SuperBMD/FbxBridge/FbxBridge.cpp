#include "pch.h"
#include "FbxBridge.h"

#include <msclr/marshal_cppstd.h>
#include <string>
#include <cstring>
#include <map>

using namespace FbxBridge;

std::string FbxSceneWriter::ToUtf8(System::String^ s)
{
    if (s == nullptr) return std::string();
    array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(s);
    std::string out;
    out.resize(bytes->Length);
    if (bytes->Length > 0)
        System::Runtime::InteropServices::Marshal::Copy(bytes, 0, System::IntPtr(&out[0]), bytes->Length);
    return out;
}

void FbxSceneWriter::EnsureLayer(FbxMesh* mesh)
{
    if (!mesh->GetLayer(0))
        mesh->CreateLayer();
}

FbxSceneWriter::FbxSceneWriter()
{
    mMgr = FbxManager::Create();
    if (!mMgr)
        throw gcnew System::Exception("Failed to create FbxManager");

    FbxIOSettings* ios = FbxIOSettings::Create(mMgr, IOSROOT);
    mMgr->SetIOSettings(ios);
}

FbxSceneWriter::~FbxSceneWriter()
{
    this->!FbxSceneWriter();
}

FbxSceneWriter::!FbxSceneWriter()
{
    if (mScene) { mScene->Destroy(); mScene = nullptr; }
    if (mMgr) { mMgr->Destroy();   mMgr = nullptr; }
}

void FbxSceneWriter::CreateScene(System::String^ name)
{
    if (!mMgr) throw gcnew System::Exception("FbxManager is null");
    if (mScene) { mScene->Destroy(); mScene = nullptr; }

    std::string sceneName = ToUtf8(name);
    mScene = FbxScene::Create(mMgr, sceneName.c_str());
    if (!mScene) throw gcnew System::Exception("Failed to create FbxScene");

    auto& gs = mScene->GetGlobalSettings();
    gs.SetSystemUnit(FbxSystemUnit::cm);
    gs.SetAxisSystem(FbxAxisSystem::MayaYUp);
}

void FbxSceneWriter::AddMeshWithMaterial(
    System::String^ nodeName,
    array<float>^ verts,
    array<int>^ indices,
    array<float>^ uvs,
    array<float>^ normals,
    System::String^ texturePath)
{
    if (!mScene) throw gcnew System::Exception("Call CreateScene first");
    if (verts == nullptr || indices == nullptr)
        throw gcnew System::ArgumentNullException("verts or indices is null");

    const int vCount = static_cast<int>(verts->Length / 3);
    const int triCount = static_cast<int>(indices->Length / 3);
    if (vCount <= 0 || triCount <= 0)
        throw gcnew System::Exception("Empty geometry input");

    FbxMesh* mesh = FbxMesh::Create(mScene, ToUtf8(nodeName).c_str());
    if (!mesh) throw gcnew System::Exception("Failed to create FbxMesh");

    // control points
    mesh->InitControlPoints(vCount);
    for (int i = 0; i < vCount; ++i) {
        mesh->SetControlPointAt(
            FbxVector4(verts[i * 3 + 0], verts[i * 3 + 1], verts[i * 3 + 2]), i);
    }

    EnsureLayer(mesh);
    FbxLayer* layer0 = mesh->GetLayer(0);

    // UVs
    // Check if the number of UVs matches the number of vertices.
    const bool hasUvs = (uvs != nullptr && uvs->Length >= vCount * 2);
    FbxLayerElementUV* uvElem = nullptr;
    if (hasUvs) {
        uvElem = FbxLayerElementUV::Create(mesh, "UVSet");
        uvElem->SetMappingMode(FbxLayerElement::eByPolygonVertex);
        uvElem->SetReferenceMode(FbxLayerElement::eDirect);

        // Add the UV coordinates to the direct array.
        uvElem->GetDirectArray().SetCount(indices->Length);
        for (int i = 0; i < indices->Length; ++i) {
            int native_index = indices[i];
            uvElem->GetDirectArray().SetAt(i, FbxVector2(uvs[native_index * 2 + 0], uvs[native_index * 2 + 1]));
        }
        // Assign the UVs to the layer.
        layer0->SetUVs(uvElem);
    }

    // build triangles
    for (int t = 0; t < triCount; ++t) {
        mesh->BeginPolygon();
        for (int k = 0; k < 3; ++k) {
            // The index array for the polygons remains the same.
            mesh->AddPolygon(indices[t * 3 + k]);
        }
        mesh->EndPolygon();
    }
    // Note: The call to layer0->SetUVs() was moved up to keep the UV logic together.

    // Normals (Your existing normal code is correct for per-vertex normals)
    const bool hasNormals = (normals != nullptr && normals->Length >= vCount * 3);
    if (hasNormals) {
        FbxLayerElementNormal* n = FbxLayerElementNormal::Create(mesh, "Normals");
        n->SetMappingMode(FbxLayerElement::eByPolygonVertex); // per vertex
        n->SetReferenceMode(FbxLayerElement::eDirect);
        n->GetDirectArray().SetCount(indices->Length);
        for (int i = 0; i < indices->Length; ++i) {
			int native_index = indices[i];
            n->GetDirectArray().SetAt(i, FbxVector4(
                normals[native_index * 3 + 0], normals[native_index * 3 + 1], normals[native_index * 3 + 2], 0.0));
        }
        layer0->SetNormals(n);
    }


    // node and material
    System::String^ baseNameCLI = System::IO::Path::GetFileNameWithoutExtension(texturePath);
    std::string baseNameStd = msclr::interop::marshal_as<std::string>(baseNameCLI);
    std::string matName = baseNameStd + "_mat";
    std::string texName = baseNameStd + "_tex";
    FbxNode* node = FbxNode::Create(mScene, ToUtf8(nodeName).c_str());
    node->SetNodeAttribute(mesh);
    mScene->GetRootNode()->AddChild(node);

	FbxSurfaceLambert* mat = FbxSurfaceLambert::Create(mScene, matName.c_str());
    mat->Diffuse.Set(FbxDouble3(1.0, 1.0, 1.0));
    mat->DiffuseFactor.Set(1.0);
    node->AddMaterial(mat);

    const std::string texPath = ToUtf8(texturePath);
    if (!texPath.empty()) {
        FbxFileTexture* tex = FbxFileTexture::Create(mScene, texName.c_str());
        tex->SetFileName(texPath.c_str());
        tex->SetTextureUse(FbxTexture::eStandard);
        tex->SetMappingType(FbxTexture::eUV);
        tex->SetMaterialUse(FbxFileTexture::eModelMaterial);
        if (hasUvs) tex->UVSet.Set(FbxString("UVSet"));
        FbxProperty diffuseProp = mat->FindProperty(FbxSurfaceMaterial::sDiffuse);
        diffuseProp.ConnectSrcObject(tex);
    }
}

void FbxSceneWriter::Save(System::String^ outPath, bool ascii, bool embeddedTextures)
{
    if (!mScene) throw gcnew System::Exception("Nothing to save, call CreateScene and add content");

    std::string path = ToUtf8(outPath);

    int writerFormat = -1;
    FbxIOPluginRegistry* reg = mMgr->GetIOPluginRegistry();
    if (ascii)
    {
        const int count = reg->GetWriterFormatCount();
        for (int i = 0; i < count; ++i)
        {
            if (reg->WriterIsFBX(i))
            {
                const char* desc = reg->GetWriterFormatDescription(i);
                if (desc && std::strstr(desc, "ascii"))
                {
                    writerFormat = i;
                    break;
                }
            }
        }
    }
    if (writerFormat < 0)
        writerFormat = reg->GetNativeWriterFormat();

    mMgr->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, embeddedTextures);

    FbxExporter* exp = FbxExporter::Create(mMgr, "Exporter");
    if (!exp->Initialize(path.c_str(), writerFormat, mMgr->GetIOSettings()))
    {
        System::String^ err = gcnew System::String(exp->GetStatus().GetErrorString());
        exp->Destroy();
        throw gcnew System::Exception("FBX Initialize failed: " + err);
    }

    if (!exp->Export(mScene))
    {
        System::String^ err = gcnew System::String(exp->GetStatus().GetErrorString());
        exp->Destroy();
        throw gcnew System::Exception("FBX Export failed: " + err);
    }

    exp->Destroy();
}
