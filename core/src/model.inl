#include <queue>

#include <QtCore/QFile>
#include <QtGui/QOpenGLExtraFunctions>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "renderer.h"
#include "importexport.h"

namespace trash
{
namespace core
{

std::shared_ptr<Model> Renderer::loadModel(const std::string& filename)
{
    auto object = std::static_pointer_cast<Model>(m_resourceStorage->get(filename));
    if (!object)
    {
        if (filename.find(".mdl") != std::string::npos)
        {
            std::shared_ptr<Model> mdl;
            std::ifstream file(filename, std::ios_base::binary);
            pull(file, mdl);
            file.close();
            m_resourceStorage->store(filename, mdl);
            return mdl;
        }

        QFile file(QString::fromStdString(filename));
        if (!file.open(QFile::ReadOnly))
            return nullptr;

        auto byteArray = file.readAll();
        file.close();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(byteArray.data(), static_cast<size_t>(byteArray.size()),
                                                           aiProcess_Triangulate |
                                                           aiProcess_FlipUVs |
                                                           aiProcess_CalcTangentSpace);
        byteArray.clear();

        if (!scene)
            return nullptr;

        object = std::make_shared<Model>();

        std::vector<std::shared_ptr<Model::Material>> materials(scene->mNumMaterials);
        for (unsigned int m = 0; m < scene->mNumMaterials; ++m)
        {
            auto material = scene->mMaterials[m];
            auto materialTo = std::make_shared<Model::Material>();

            if (material->GetTextureCount(aiTextureType_DIFFUSE))
            {
                aiString path;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
                {
                    std::string filename = path.C_Str();
                    materialTo->diffuseTexture = std::make_pair(filename, loadTexture(filename));

                    std::string bumpFilename = filename;
                    bumpFilename.replace(bumpFilename.find("Diffuse"), 7, "Normal");
                    materialTo->normalTexture = std::make_pair(bumpFilename, loadTexture(bumpFilename));
                }

            }
            if (material->GetTextureCount(aiTextureType_NORMALS))
            {
                aiString path;
                if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
                {
                    std::string filename = path.C_Str();
                    materialTo->normalTexture = std::make_pair(filename, loadTexture(filename));
                }
            }

            materials[m] = materialTo;
        }

        std::unordered_map<std::string, int32_t> boneMapping;
        std::vector<std::shared_ptr<Model::Mesh>> meshes(scene->mNumMeshes);
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
        {
            auto meshFrom = scene->mMeshes[m];
            auto meshTo = std::make_shared<core::Mesh>();

            if (meshFrom->HasPositions())
            {
                meshTo->declareVertexAttribute(VertexAttribute::Position,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 3, reinterpret_cast<float*>(meshFrom->mVertices), GL_STATIC_DRAW));
            }

            if (meshFrom->HasNormals())
            {
                meshTo->declareVertexAttribute(VertexAttribute::Normal,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 3, reinterpret_cast<float*>(meshFrom->mNormals), GL_STATIC_DRAW));
            }

            if (meshFrom->HasTextureCoords(0))
            {
                meshTo->declareVertexAttribute(VertexAttribute::TexCoord,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 3, reinterpret_cast<float*>(meshFrom->mTextureCoords[0]), GL_STATIC_DRAW));
            }

            if (meshFrom->HasTangentsAndBitangents())
            {
                meshTo->declareVertexAttribute(VertexAttribute::Tangent,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 3, reinterpret_cast<float*>(meshFrom->mTangents), GL_STATIC_DRAW));
            }

            if (meshFrom->HasBones())
            {
                std::vector<float> boneIds(4 * meshFrom->mNumVertices);
                std::vector<float> boneWeights(4 * meshFrom->mNumVertices);

                for (unsigned int i = 0; i < meshFrom->mNumBones; ++i)
                {
                    auto& bone = meshFrom->mBones[i];

                    int32_t boneIndex;
                    std::string boneName = bone->mName.C_Str();

                    auto it = boneMapping.find(boneName);
                    if (it == boneMapping.end())
                    {
                        boneIndex = static_cast<int32_t>(object->boneTransforms.size());
                        boneMapping[boneName] = boneIndex;

                        aiVector3D t, s;
                        aiQuaternion r;
                        bone->mOffsetMatrix.Decompose(s, r, t);

                        object->boneNames.push_back(boneName);
                        object->boneTransforms.push_back(utils::Transform(glm::vec3(s.x, s.y, s.z), glm::quat(r.w, r.x, r.y, r.z), glm::vec3(t.x, t.y, t.z)));

                    }
                    else
                    {
                        boneIndex = it->second;
                    }

                    for (unsigned int j = 0; j < bone->mNumWeights; ++j)
                    {
                        auto& weight = bone->mWeights[j];
                        float* vertexBoneId = boneIds.data() + 4 * weight.mVertexId;
                        float* vertexBoneWeights = boneWeights.data() + 4 * weight.mVertexId;
                        unsigned int k = 4;
                        while (k > 0 && vertexBoneWeights[k-1] < weight.mWeight)
                            --k;
                        for (unsigned int t = 3; t > k; --t)
                        {
                            vertexBoneWeights[t] = vertexBoneWeights[t-1];
                            vertexBoneId[t] = vertexBoneId[t-1];
                        }
                        if (k < 4)
                        {
                            vertexBoneWeights[k] = weight.mWeight;
                            vertexBoneId[k] = static_cast<float>(boneIndex);
                        }
                    }

                    for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                    {
                        float* wights = boneWeights.data() + 4 * i;
                        float weight = wights[0] + wights[1] + wights[2] + wights[3];
                        if (weight > 0.0f)
                        {
                            wights[0] /= weight;
                            wights[1] /= weight;
                            wights[2] /= weight;
                            wights[3] /= weight;
                        }
                    }
                }

                meshTo->declareVertexAttribute(VertexAttribute::BonesIDs,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 4, boneIds.data(), GL_STATIC_DRAW));

                meshTo->declareVertexAttribute(VertexAttribute::BonesWeights,
                                               std::make_shared<VertexBuffer>(meshFrom->mNumVertices, 4, boneWeights.data(), GL_STATIC_DRAW));
            }

            std::vector<uint32_t> indices(3 * meshFrom->mNumFaces);
            for (unsigned int i = 0; i < meshFrom->mNumFaces; ++i)
            {
                auto& face = meshFrom->mFaces[i];
                memcpy(indices.data() + 3 * i,
                       face.mIndices,
                       3 * sizeof(unsigned int));
            }
            meshTo->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW));

            meshes[m] = std::shared_ptr<Model::Mesh>(new Model::Mesh(meshTo, materials[meshFrom->mMaterialIndex]));
        }

        for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
        {
            auto animFrom = scene->mAnimations[a];
            auto animTo = std::make_shared<Model::Animation>(static_cast<float>(animFrom->mTicksPerSecond),static_cast<float>(animFrom->mDuration));
            object->animations[animFrom->mName.C_Str()] = animTo;

            for (unsigned int c = 0; c < animFrom->mNumChannels; ++c)
            {
                auto animNode = animFrom->mChannels[c];
                std::string boneName = animNode->mNodeName.C_Str();

                auto iter = boneMapping.find(boneName);
                if (iter == boneMapping.end())
                    continue;

                auto& transformsTuple = animTo->transforms[boneName];

                auto& scales = std::get<0>(transformsTuple);
                scales.resize(static_cast<size_t>(animNode->mNumScalingKeys));
                for (unsigned int i = 0; i < animNode->mNumScalingKeys; ++i)
                {
                    auto& key = animNode->mScalingKeys[i];
                    scales[i] = std::make_pair(static_cast<float>(key.mTime),
                                               glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
                }

                auto& rotations = std::get<1>(transformsTuple);
                rotations.resize(animNode->mNumRotationKeys);
                for (unsigned int i = 0; i < animNode->mNumRotationKeys; ++i)
                {
                    auto& key = animNode->mRotationKeys[i];
                    rotations[i] = std::make_pair(static_cast<float>(key.mTime),
                                                  glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
                }

                auto& translations = std::get<2>(transformsTuple);
                translations.resize(animNode->mNumPositionKeys);
                for (unsigned int i = 0; i < animNode->mNumPositionKeys; ++i)
                {
                    auto& key = animNode->mPositionKeys[i];
                    translations[i] = std::make_pair(static_cast<float>(key.mTime),
                                                     glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z));
                }
            }
        }

        auto copyNode = [&meshes](aiNode* node) -> std::shared_ptr<Model::Node> {
            aiVector3D t, s;
            aiQuaternion r;
            node->mTransformation.Decompose(s, r, t);
            auto result = std::make_shared<Model::Node>(utils::Transform(glm::vec3(s.x, s.y, s.z), glm::quat(r.w, r.x, r.y, r.z), glm::vec3(t.x, t.y, t.z)));
            for (unsigned int i = 0; i < node->mNumMeshes; ++i)
                result->meshes.push_back(meshes[node->mMeshes[i]]);
            return result;
        };

        std::queue<std::pair<aiNode*, std::shared_ptr<Model::Node>>> nodes;
        if (scene->mRootNode)
        {
            object->rootNode = copyNode(scene->mRootNode);
            object->rootNode->transform = utils::Transform();
            nodes.push(std::make_pair(scene->mRootNode, object->rootNode));
        }
        while (!nodes.empty())
        {
            auto nodeFrom = nodes.front().first;
            auto nodeTo = nodes.front().second;
            nodes.pop();

            auto iter = boneMapping.find(nodeFrom->mName.C_Str());
            if (iter != boneMapping.end())
                nodeTo->boneIndex = iter->second;

            for (unsigned int i = 0; i < nodeFrom->mNumChildren; ++i)
            {
                auto childFrom = nodeFrom->mChildren[i];
                auto childTo = copyNode(childFrom);
                nodeTo->attach(childTo);
                nodes.push(std::make_pair(childFrom, childTo));
            }
        }

        importer.FreeScene();
        m_resourceStorage->store(filename, object);
    }

    return object;
}

uint32_t Model::numBones() const
{
    return static_cast<uint32_t>(boneTransforms.size());
}

bool Model::calcBoneTransforms(const std::string &animName, float timeInSecs, std::vector<utils::Transform>& transforms) const
{
    transforms.resize(numBones(), utils::Transform());

    auto iter = animations.find(animName);
    if (iter == animations.end())
        return false;

    auto animation = iter->second;

    float ticksPerSecond = animation->framesPerSecond > 0.0f ? animation->framesPerSecond : 25.0f;
    float timeInTicks = timeInSecs * ticksPerSecond;
    float animTime = std::fmod(timeInTicks, animation->duration);

    std::queue<std::pair<std::shared_ptr<Node>, utils::Transform>> nodes;
    if (rootNode)
        nodes.push(std::make_pair(rootNode, utils::Transform()));
    while (!nodes.empty())
    {
        auto node = nodes.front().first;
        auto parentTransform = nodes.front().second;
        nodes.pop();

        utils::Transform globalTransform = parentTransform;
        if (node->boneIndex >= 0)
        {
            utils::Transform boneTransform = node->transform;
            auto& transformTuple = animation->transforms[boneNames[static_cast<size_t>(node->boneIndex)]];

            auto& scales = std::get<0>(transformTuple);
            if (scales.size() == 1)
                boneTransform.scale = scales.front().second;
            else if (scales.size() > 1)
            {
                for (unsigned int i = 0; i < scales.size() - 1; ++i)
                    if (animTime < scales[i+1].first)
                    {
                        float factor = (animTime - scales[i].first) / (scales[i+1].first - scales[i].first);
                        boneTransform.scale = glm::mix(scales[i].second, scales[i+1].second, factor);
                        break;
                    }
            }

            auto& rotations = std::get<1>(transformTuple);
            if (rotations.size() == 1)
                boneTransform.rotation = rotations.front().second;
            else if (rotations.size() > 1)
            {
                for (unsigned int i = 0; i < rotations.size() - 1; ++i)
                    if (animTime < rotations[i+1].first)
                    {
                        float factor = (animTime - rotations[i].first) / (rotations[i+1].first - rotations[i].first);
                        boneTransform.rotation = glm::slerp(rotations[i].second, rotations[i+1].second, factor);
                        break;
                    }
            }

            auto& translations = std::get<2>(transformTuple);
            if (translations.size() == 1)
                boneTransform.translation = translations.front().second;
            else if (translations.size() > 1)
            {
                for (unsigned int i = 0; i < translations.size() - 1; ++i)
                    if (animTime < translations[i+1].first)
                    {
                        float factor = (animTime - translations[i].first) / (translations[i+1].first - translations[i].first);
                        boneTransform.translation = glm::mix(translations[i].second, translations[i+1].second, factor);
                        break;
                    }
            }

            globalTransform *= boneTransform;
            transforms[static_cast<size_t>(node->boneIndex)] = globalTransform * boneTransforms[static_cast<size_t>(node->boneIndex)];
        }
        else {
            globalTransform *= node->transform;
        }

        for (auto child: node->children())
            nodes.push(std::make_pair(child, globalTransform));
    }

    return true;
}

} // namespace
} // namespace
