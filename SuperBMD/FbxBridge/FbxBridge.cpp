#include "pch.h"

#include "FbxBridge.h"
#include <string>

using namespace FbxBridge;

static std::string ToUtf8(System::String^ s)
{
    using namespace System::Runtime::InteropServices;
    IntPtr p = Marshal::StringToHGlobalAnsi(s);
    std::string out = static_cast<const char*>(p.ToPointer());
    Marshal::FreeHGlobal(p);
    return out;
}

FbxSceneWriter::FbxSceneWriter()
{
    mMgr = FbxManager::Create();
    auto ios = FbxIOSettings::Create(mMgr, IOSROOT);
    mMgr->SetIOSettings(ios);
}

FbxSceneWriter::~FbxSceneWriter()
{
    if (mScene) mScene->Destroy();
    if (mMgr)   mMgr->Destroy();
}

void FbxSceneWriter::CreateScene(System::String^ name)
{
    if (mScene) { mScene->Destroy(); mScene = nullptr; }
    mScene = FbxScene::Create(mMgr, ToUtf8(name).c_str());

    auto& gs = mScene->GetGlobalSettings();
    gs.SetSystemUnit(FbxSystemUnit::m);
    gs.SetAxisSystem(FbxAxisSystem::MayaYUp);
}

void FbxSceneWriter::AddMeshWithMaterial(String^ nodeName,
    array<float>^ verts,
    array<int>^ indices,
    array<float>^ uvs,
    String^ texturePath)
{
    auto root = mScene->GetRootNode();

    // Create mesh and control points
    FbxMesh* mesh = FbxMesh::Create(mScene, ToUtf8(nodeName).c_str());
    int vCount = verts->Length / 3;
    mesh->InitControlPoints(vCount);

    for (int i = 0; i < vCount; ++i)
    {
        double x = verts[i * 3 + 0];
        double y = verts[i * 3 + 1];
        double z = verts[i * 3 + 2];
        mesh->SetControlPointAt(FbxVector4(x, y, z), i);
    }

    // Create layer 0 UVs as per polygon vertex
    FbxLayer* layer = mesh->GetLayer(0);
    if (!layer) { mesh->CreateLayer(); layer = mesh->GetLayer(0); }

    FbxLayerElementUV* uvElem = FbxLayerElementUV::Create(mesh, "UVChannel_1");
    uvElem->SetMappingMode(FbxLayerElement::eByPolygonVertex);
    uvElem->SetReferenceMode(FbxLayerElement::eDirect);

    // Build polygons from triangle index list
    int triCount = indices->Length / 3;
    int uvIt = 0;

    // We will push UVs per polygon vertex in the same order as indices
    uvElem->GetDirectArray().SetCount(triCount * 3);

    for (int t = 0; t < triCount; ++t)
    {
        mesh->BeginPolygon();
        for (int k = 0; k < 3; ++k)
        {
            int vi = indices[t * 3 + k];
            mesh->AddPolygon(vi);

            double u = uvs[uvIt * 2 + 0];
            double v = uvs[uvIt * 2 + 1];
            uvElem->GetDirectArray().SetAt(uvIt, FbxVector2(u, v));
            ++uvIt;
        }
        mesh->EndPolygon();
    }

    layer->SetUVs(uvElem);

    // Node
    FbxNode* node = FbxNode::Create(mScene, ToUtf8(nodeName).c_str());
    node->SetNodeAttribute(mesh);
    root->AddChild(node);

    // Create Phong material with a file texture
    FbxSurfacePhong* mat = FbxSurfacePhong::Create(mScene, "Mat");
    mat->Diffuse.Set(FbxDouble3(1.0, 1.0, 1.0));
    mat->DiffuseFactor.Set(1.0);
    node->AddMaterial(mat);

    std::string texFile = ToUtf8(texturePath);
    // Create a file texture and hook it to the material's Diffuse
    FbxFileTexture* tex = FbxFileTexture::Create(mScene, "Tex");
    tex->SetFileName(texFile.c_str());

    // usage and mapping
    tex->SetTextureUse(FbxTexture::eStandard);          // not eTextureUseDiffuse
    tex->SetMappingType(FbxTexture::eUV);
    tex->SetMaterialUse(FbxFileTexture::eModelMaterial);
    tex->UVSet.Set(FbxString("UVChannel_1"));

    // connect specifically to the Diffuse property
    FbxProperty diffuseProp = mat->FindProperty(FbxSurfaceMaterial::sDiffuse);
    diffuseProp.ConnectSrcObject(tex);
}

void FbxSceneWriter::Save(System::String^ outPath, bool ascii)
{
    std::string path = ToUtf8(outPath);
    FbxExporter* exp = FbxExporter::Create(mMgr, "exp");

    // Embed textures
    mMgr->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, true);

    int format = -1;
    auto reg = mMgr->GetIOPluginRegistry();
    if (ascii)
    {
        int c = reg->GetWriterFormatCount();
        for (int i = 0; i < c; ++i)
        {
            if (reg->WriterIsFBX(i) && strstr(reg->GetWriterFormatDescription(i), "ascii"))
            {
                format = i; break;
            }
        }
    }
    if (format < 0) format = reg->GetNativeWriterFormat();

    if (!exp->Initialize(path.c_str(), format, mMgr->GetIOSettings()))
        throw gcnew System::Exception(gcnew System::String(exp->GetStatus().GetErrorString()));
    if (!exp->Export(mScene))
        throw gcnew System::Exception(gcnew System::String(exp->GetStatus().GetErrorString()));
    exp->Destroy();
}
