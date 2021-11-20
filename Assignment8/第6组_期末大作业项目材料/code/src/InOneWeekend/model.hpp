#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.hpp"
#include "texture.h"
#include "transformer.hpp"

class model : public hittable_list
{
public:
    explicit model(const char *path);
    model(const char *path, std::shared_ptr<transformer> t);
    model(const char *path, std::shared_ptr<material> mat);
    model(const char *path, std::shared_ptr<material> mat, std::shared_ptr<transformer> t);
    model(const char *path,
          std::unordered_map<std::string, std::shared_ptr<material>> mats,
          std::shared_ptr<transformer> t);
    void add_to_scene(hittable_list &scene) const;

private:
    std::string directory;
    std::unordered_map<std::string, std::shared_ptr<material>> _mats;
    std::shared_ptr<transformer> _t;

    void load_model(std::string path);
    void process_node(aiNode *node, const aiScene *scene);
    mesh process_mesh(aiMesh *mesh, const aiScene *scene);
    std::vector<std::shared_ptr<texture>> load_material_textures(
        aiMaterial *mat, aiTextureType type,
        std::string typeName);
};
