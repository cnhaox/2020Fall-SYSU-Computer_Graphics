#include "model.hpp"
#include "material.h"
#include "triangle.h"
#include <assimp/Importer.hpp>  // C++ model interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure
#include <cassert>
#include <iostream>

using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::pair;
using std::shared_ptr;
using std::tuple;
using std::vector;

model::model(const char *path)
    : model(path, make_shared<identity_transformer>())
{
}

model::model(const char *path, shared_ptr<transformer> t)
    : model(path, {{"default", make_shared<lambertian>(color(0.5, 0.5, 0.5))}}, t)
{
}

model::model(const char *path, shared_ptr<material> mat)
    : model(path, mat, make_shared<identity_transformer>())
{
}

model::model(const char *path, shared_ptr<material> mat, shared_ptr<transformer> t)
    : model(path, {{"default", mat}}, t)
{
}

model::model(const char *path, std::unordered_map<std::string, shared_ptr<material>> mats, shared_ptr<transformer> t)
{
    _mats = mats;
    _t = t;
    load_model(path);
}

void model::load_model(std::string path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    process_node(scene->mRootNode, scene);
}

void model::process_node(aiNode *node, const aiScene *scene)
{
    // 处理节点所有的网格（如果有的话）
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        add(make_shared<::mesh>(process_mesh(mesh, scene)));
    }
    // 接下来对它的子节点重复这一过程
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

mesh model::process_mesh(aiMesh *ai_mesh, const aiScene *ai_scene)
{
    vector<vertex> vertices;
    mesh result;

    assert(ai_mesh->mNumVertices > 2);
    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++)
    {
        vertex v;

        vec3 pos = _t->transform(vec3(
            ai_mesh->mVertices[i].x,
            ai_mesh->mVertices[i].y,
            ai_mesh->mVertices[i].z));
        v[0] = pos.x();
        v[1] = pos.y();
        v[2] = pos.z();

        pair<double, double> texcoord = make_pair(0.0, 0.0);
        if (ai_mesh->mTextureCoords[0] != nullptr) // 网格是否有纹理坐标？
        {
            v.u = ai_mesh->mTextureCoords[0][i].x;
            v.v = ai_mesh->mTextureCoords[0][i].y;
        }

        // vertex normal position
        v.normal = vec3(ai_mesh->mNormals[i].x,
                        ai_mesh->mNormals[i].y,
                        ai_mesh->mNormals[i].z);
        // v.normal = _t->transform(v.normal);

        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++)
    {
        // TODO: support assimp material system
        aiFace face = ai_mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        auto mat = _mats["default"];
        if (ai_scene->HasMaterials() && ai_scene->mMaterials[ai_mesh->mMaterialIndex] != nullptr)
        {
            auto name = ai_scene->mMaterials[ai_mesh->mMaterialIndex]->GetName();
            if (_mats.count(name.C_Str()) > 0)
            {
                mat = _mats[name.C_Str()];
            }
        }
        const auto &v0 = vertices[face.mIndices[0]];
        const auto &v1 = vertices[face.mIndices[1]];
        const auto &v2 = vertices[face.mIndices[2]];
        result.add(make_shared<triangle>(v0, v1, v2, mat));
    }

    return result;
}

void model::add_to_scene(hittable_list &scene) const
{
    for (const auto &object : objects)
    {
        scene.add(object);
    }
}
