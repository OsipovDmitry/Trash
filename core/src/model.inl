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

        std::unordered_map<std::string, int32_t> boneMapping;
        std::vector<std::shared_ptr<Model::Mesh>> meshes(scene->mNumMeshes);
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
        {
            auto meshFrom = scene->mMeshes[m];

            std::vector<std::pair<bool, VertexAttribute>> attribsInfo {
                std::make_pair(meshFrom->HasPositions(), VertexAttribute::Position),
                std::make_pair(meshFrom->HasNormals(), VertexAttribute::Normal),
                std::make_pair(meshFrom->HasTextureCoords(0), VertexAttribute::TexCoord),
                std::make_pair(meshFrom->HasBones(), VertexAttribute::BonesIDs),
                std::make_pair(meshFrom->HasBones(), VertexAttribute::BonesWeights),
                std::make_pair(meshFrom->HasTangentsAndBitangents(), VertexAttribute::TangentBinormal),
            };

            std::unordered_map<VertexAttribute, uint32_t> offsets;
            uint32_t vdSize = 0;

            for (size_t i = 0; i < attribsInfo.size(); ++i)
            {
                auto& attribInfo = attribsInfo.at(i);
                if (attribInfo.first)
                {
                    offsets[attribInfo.second] = vdSize;
                    vdSize += numAttributeComponents(attribInfo.second);
                }
            }

            std::vector<float> vertices(static_cast<size_t>(vdSize * meshFrom->mNumVertices));
            std::vector<uint32_t> indices(3 * meshFrom->mNumFaces);
            BoundingSphere boundingSphere;

            if (meshFrom->HasPositions())
            {
                const uint32_t offset = offsets[VertexAttribute::Position];
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                meshFrom->mVertices + i,
                                numAttributeComponents(VertexAttribute::Position) * sizeof(float));

                aiVector3D center(0.f, 0.f, 0.f);
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                    center += meshFrom->mVertices[i];
                if (meshFrom->mNumVertices)
                    center /= static_cast<float>(meshFrom->mNumVertices);

                float radius = -1.f;
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                {
                    const float curRadius = (meshFrom->mVertices[i] - center).SquareLength();
                    if (curRadius > radius)
                        radius = curRadius;
                }

                boundingSphere.setCenter(glm::vec3(center.x, center.y, center.z));
                boundingSphere.setRadius(glm::sqrt(radius));
            }

            if (meshFrom->HasNormals())
            {
                const uint32_t offset = offsets[VertexAttribute::Normal];
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                meshFrom->mNormals + i,
                                numAttributeComponents(VertexAttribute::Normal) * sizeof(float));
            }

            if (meshFrom->HasTextureCoords(0))
            {
                const uint32_t offset = offsets[VertexAttribute::TexCoord];
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                    std::memcpy(vertices.data() + vdSize * i + offset,
                                meshFrom->mTextureCoords[0] + i,
                                numAttributeComponents(VertexAttribute::TexCoord) * sizeof(float));
            }

            if (meshFrom->HasBones())
            {
                const uint32_t offsetIds = offsets[VertexAttribute::BonesIDs];
                const uint32_t offsetWeights = offsets[VertexAttribute::BonesWeights];
                for (unsigned int i = 0; i < meshFrom->mNumBones; ++i)
                {
                    auto& bone = meshFrom->mBones[i];

                    int32_t boneIndex;
                    std::string boneName = bone->mName.C_Str();

                    auto it = boneMapping.find(boneName);
                    if (it == boneMapping.end())
                    {
                        boneIndex = static_cast<int32_t>(model->boneTransforms.size());
                        boneMapping[boneName] = boneIndex;

                        aiVector3D t, s;
                        aiQuaternion r;
                        bone->mOffsetMatrix.Decompose(s, r, t);

                        model->boneNames.push_back(boneName);
                        model->boneTransforms.push_back(Transform(glm::vec3(s.x, s.y, s.z), glm::quat(r.w, r.x, r.y, r.z), glm::vec3(t.x, t.y, t.z)));

                    }
                    else
                    {
                        boneIndex = it->second;
                    }


                    for (unsigned int j = 0; j < bone->mNumWeights; ++j)
                    {
                        auto& weight = bone->mWeights[j];
                        float* vertexBoneId = vertices.data() + vdSize * weight.mVertexId + offsetIds;
                        float* vertexBoneWeights = vertices.data() + vdSize * weight.mVertexId + offsetWeights;
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
                }
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                {
                    float weight = (vertices.data() + vdSize * i + offsetWeights)[0]
                            + (vertices.data() + vdSize * i + offsetWeights)[1]
                            + (vertices.data() + vdSize * i + offsetWeights)[2]
                            + (vertices.data() + vdSize * i + offsetWeights)[3];
                    if (weight > 0.0f)
                    {
                        (vertices.data() + vdSize * i + offsetWeights)[0] /= weight;
                        (vertices.data() + vdSize * i + offsetWeights)[1] /= weight;
                        (vertices.data() + vdSize * i + offsetWeights)[2] /= weight;
                        (vertices.data() + vdSize * i + offsetWeights)[3] /= weight;
                    }
                }
            }

            if (meshFrom->HasTangentsAndBitangents())
            {
                const uint32_t offset = offsets[VertexAttribute::TangentBinormal];
                for (unsigned int i = 0; i < meshFrom->mNumVertices; ++i)
                {
                    auto& t = meshFrom->mTangents[i];
                    auto& b = meshFrom->mBitangents[i];
                    auto& n = meshFrom->mNormals[i];
                    float sign = ((n ^ t) * b > 0.0f) ? +1.0f : -1.0f;

                    std::memcpy(vertices.data() + vdSize * i + offset,
                                &t, 3 * sizeof(float));
                    std::memcpy(vertices.data() + vdSize * i + offset + 3,
                                &sign, sizeof(float));

                }
            }

            for (unsigned int i = 0; i < meshFrom->mNumFaces; ++i)
            {
                auto& face = meshFrom->mFaces[i];
                memcpy(indices.data() + 3 * i,
                       face.mIndices,
                       3 * sizeof(unsigned int));
            }

            auto vbo = std::make_shared<VertexBuffer>(vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            auto ibo = std::make_shared<IndexBuffer>(GL_TRIANGLES, indices.size(), indices.data(), GL_STATIC_DRAW);
            auto glMesh = std::make_shared<::Mesh>();

            for (auto& offset : offsets)
                glMesh->attachVertexBuffer(offset.first,
                                           vbo,
                                           static_cast<GLint>(numAttributeComponents(offset.first)),
                                           static_cast<GLsizei>(vdSize * sizeof(float)),
                                           static_cast<GLsizei>(offset.second * sizeof(float)));

            glMesh->attachIndexBuffer(ibo);

            meshes[m] = std::shared_ptr<Model::Mesh>(new Model::Mesh(glMesh, materials[meshFrom->mMaterialIndex], boundingSphere));
        }

        for (unsigned int a = 0; a < scene->mNumAnimations; ++a)
        {
            auto animFrom = scene->mAnimations[a];
            auto animTo = std::make_shared<Model::Animation>(static_cast<float>(animFrom->mTicksPerSecond),static_cast<float>(animFrom->mDuration));
            model->animations[animFrom->mName.C_Str()] = animTo;

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
        ResourceStorage::instance().store(filename, object);
    }

    return std::static_pointer_cast<Model>(object);
}

uint32_t Model::numBones() const
{
    return static_cast<uint32_t>(boneTransforms.size());
}

bool Model::calcBoneTransforms(const std::string &animName, float timeInSecs, std::vector<Transform>& transforms) const
{
    transforms.resize(numBones(), Transform());

    auto iter = animations.find(animName);
    if (iter == animations.end())
        return false;

    auto animation = iter->second;

    float ticksPerSecond = animation->framesPerSecond > 0.0f ? animation->framesPerSecond : 25.0f;
    float timeInTicks = timeInSecs * ticksPerSecond;
    float animTime = std::fmod(timeInTicks, animation->duration);

    std::queue<std::pair<std::shared_ptr<Node>, Transform>> nodes;
    if (rootNode)
        nodes.push(std::make_pair(rootNode, Transform()));
    while (!nodes.empty())
    {
        auto node = nodes.front().first;
        auto parentTransform = nodes.front().second;
        nodes.pop();

        Transform globalTransform = parentTransform;
        if (node->boneIndex >= 0)
        {
            Transform boneTransform = node->transform;
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

BoundingSphere Model::calcBoundingSphere() const
{
    BoundingSphere boundingSphere;

    std::queue<std::pair<std::shared_ptr<Node>, Transform>> nodes;
    if (rootNode)
        nodes.push(std::make_pair(rootNode, Transform()));
    while (!nodes.empty())
    {
        auto node = nodes.front().first;
        auto parentTransform = nodes.front().second;
        nodes.pop();

        auto thisTransform = parentTransform * node->transform;

        for (auto mesh : node->meshes)
            boundingSphere += thisTransform * mesh->boundingSphere;

        for (auto child : node->children())
            nodes.push(std::make_pair(child, thisTransform));

    }

    return boundingSphere;
}
