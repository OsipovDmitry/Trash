#include <queue>

#include <QtCore/QFile>
#include <QtGui/QOpenGLExtraFunctions>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "renderer.h"

std::shared_ptr<Model> Renderer::loadModel(const std::string& filename)
{
    auto object = ResourceStorage::instance().get(filename);
    if (!object)
    {
        QFile file(QString::fromStdString(filename));
        if (!file.open(QFile::ReadOnly))
            return nullptr;

        auto byteArray = file.readAll();
        file.close();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(byteArray.data(), static_cast<size_t>(byteArray.size()), aiProcess_Triangulate | aiProcess_FlipUVs);
        byteArray.clear();

        if (!scene)
            return nullptr;

        auto model = std::make_shared<Model>();
        object = model;

        std::vector<std::shared_ptr<Model::Material>> materials(scene->mNumMaterials);
        for (unsigned int m = 0; m < scene->mNumMaterials; ++m)
        {
            auto material = scene->mMaterials[m];
            auto materialTo = std::make_shared<Model::Material>();

            if (material->GetTextureCount(aiTextureType_DIFFUSE))
            {
                aiString path;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
                    materialTo->diffuseTexture = loadTexture(path.C_Str());
            }

            materials[m] = materialTo;
        }

        std::vector<std::shared_ptr<Model::Mesh>> meshes(scene->mNumMeshes);
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
        {
            auto mesh = scene->mMeshes[m];
            std::bitset<numElementsVertexAttribute()> vertexDeclaration;

            if (mesh->HasPositions()) { vertexDeclaration.set(castFromVertexAttribute(VertexAttribute::Position)); }
            if (mesh->HasNormals()) { vertexDeclaration.set(castFromVertexAttribute(VertexAttribute::Normal)); }
            if (mesh->HasTextureCoords(0)) { vertexDeclaration.set(castFromVertexAttribute(VertexAttribute::TexCoord)); }
            if (mesh->HasTangentsAndBitangents()) { vertexDeclaration.set(castFromVertexAttribute(VertexAttribute::TangentBinormal)); }

            uint32_t vdSize = vertexDeclarationSize(vertexDeclaration);

            std::vector<float> vertices(vdSize * mesh->mNumVertices);
            std::vector<uint32_t> indices(3 * mesh->mNumFaces);

            if (mesh->HasPositions())
            {
                const uint32_t offset = vertexDeclarationOffset(vertexDeclaration, VertexAttribute::Position);
                for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                mesh->mVertices + i,
                                numAttributeComponents(VertexAttribute::Position) * sizeof(float));
            }

            if (mesh->HasNormals())
            {
                const uint32_t offset = vertexDeclarationOffset(vertexDeclaration, VertexAttribute::Normal);
                for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                mesh->mNormals + i,
                                numAttributeComponents(VertexAttribute::Normal) * sizeof(float));
            }

            if (mesh->HasTextureCoords(0))
            {
                const uint32_t offset = vertexDeclarationOffset(vertexDeclaration, VertexAttribute::TexCoord);
                for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                mesh->mTextureCoords[0] + i,
                                numAttributeComponents(VertexAttribute::TexCoord) * sizeof(float));
            }

            if (mesh->HasTangentsAndBitangents())
            {
                const uint32_t offset = vertexDeclarationOffset(vertexDeclaration, VertexAttribute::TangentBinormal);
                for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
                {
                    auto& t = mesh->mTangents[i];
                    auto& b = mesh->mBitangents[i];
                    auto& n = mesh->mNormals[i];
                    float sign = ((n ^ t) * b > 0.0f) ? +1.0f : -1.0f;

                    std::memcpy(vertices.data() + vdSize * i + offset,
                                &t, 3 * sizeof(float));
                    std::memcpy(vertices.data() + vdSize * i + offset + 3,
                                &sign, sizeof(float));

                }
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
            {
                auto& face = mesh->mFaces[i];
                memcpy(indices.data() + 3 * i,
                       face.mIndices,
                       3 * sizeof(unsigned int));
            }

            GLuint vao;
            m_functions.glGenVertexArrays(1, &vao);
            m_functions.glBindVertexArray(vao);

            GLuint vbo, ibo;
            m_functions.glGenBuffers(1, &vbo);
            m_functions.glBindBuffer(GL_ARRAY_BUFFER, vbo);
            m_functions.glBufferData(GL_ARRAY_BUFFER, vdSize * mesh->mNumVertices * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            for (uint32_t i = 0; i < numElementsVertexAttribute(); ++i)
            {
                if (vertexDeclaration.test(i)) {
                    VertexAttribute attrib = castToVertexAttribute(i);
                    m_functions.glVertexAttribPointer(i,
                                                      static_cast<GLint>(numAttributeComponents(attrib)),
                                                      GL_FLOAT,
                                                      GL_FALSE,
                                                      static_cast<GLsizei>(vdSize * sizeof(float)),
                                                      reinterpret_cast<const GLvoid*>(vertexDeclarationOffset(vertexDeclaration, attrib) * sizeof(float)));
                    m_functions.glEnableVertexAttribArray(i);
                }
                else {
                    m_functions.glDisableVertexAttribArray(i);
                }
            }

            m_functions.glGenBuffers(1, &ibo);
            m_functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            m_functions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->mNumFaces * 3 * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            m_functions.glBindVertexArray(0);

            auto meshTo = std::shared_ptr<Model::Mesh>(new Model::Mesh(vao, vbo, ibo, mesh->mNumFaces * 3, vertexDeclaration), m_meshDeleter);
            meshTo->material = materials[mesh->mMaterialIndex];
            meshes[m] = meshTo;
        }

        auto copyNode = [&meshes](aiNode* node) -> std::shared_ptr<Model::Node> {
            aiVector3D t, s;
            aiQuaternion r;
            node->mTransformation.Decompose(s, r, t);
            auto result = std::make_shared<Model::Node>(Transform(glm::vec3(s.x, s.y, s.z), glm::quat(r.w, r.x, r.y, r.z), glm::vec3(t.x, t.y, t.z)));
            for (unsigned int i = 0; i < node->mNumMeshes; ++i)
                result->meshes.push_back(meshes[node->mMeshes[i]]);
            return result;
        };

        std::queue<std::pair<aiNode*, std::shared_ptr<Model::Node>>> nodes;
        if (scene->mRootNode)
        {
            model->rootNode = copyNode(scene->mRootNode);
            model->rootNode->transform = Transform();
            nodes.push(std::make_pair(scene->mRootNode, model->rootNode));
        }
        while (!nodes.empty())
        {
            auto nodeFrom = nodes.front().first;
            auto nodeTo = nodes.front().second;
            nodes.pop();

            for (unsigned int i = 0; i < nodeFrom->mNumChildren; ++i)
            {
                auto childFrom = nodeFrom->mChildren[i];
                auto childTo = copyNode(childFrom);
                nodeTo->attach(childTo);
                nodes.push(std::make_pair(childFrom, childTo));
            }
        }

        importer.FreeScene();
        ResourceStorage::instance().store(filename, object);
    }

    return std::static_pointer_cast<Model>(object);
}
