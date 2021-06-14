#include <memory>
#include <unordered_map>

#include <QtCore/QFile>
#include <QtGui/QOpenGLExtraFunctions>

#include <utils/fileinfo.h>

#include "renderer.h"
#include "image.h"
#include "utils.h"
#include "hdrloader/hdrloader.h"
#include "rapidjson/document.h"

namespace trash
{
namespace core
{

bool Texture::stringToInternalFormat(const std::string& str, GLenum& internalFormat)
{
    static const std::unordered_map<std::string, GLenum> s_table {
        {"GL_R8", GL_R8},
        {"GL_RG8", GL_RG8},
        {"GL_RGB8", GL_RGB8},
        {"GL_RGBA8", GL_RGBA8},
        {"GL_RGB16F", GL_RGB16F},
        {"GL_RGB32F", GL_RGB32F},
        {"GL_RGB9_E5", GL_RGB9_E5},
    };

    auto it = s_table.find(str);

    if (it == s_table.end())
        return false;

    internalFormat = it->second;
    return true;
}

bool Texture::formatAndTypeToInternalFormat(GLenum format, GLenum type, GLenum& internalFormat)
{
    switch (format)
    {
    case GL_RGB:
    case GL_BGR:
    {
        switch (type)
        {
        case GL_UNSIGNED_BYTE: { internalFormat = GL_RGB8; return true; }
        case GL_HALF_FLOAT:
        case GL_FLOAT: { internalFormat = GL_RGB16F; return true; }
        default: break;
        }
        break;
    }
    case GL_RGBA:
    case GL_BGRA:
    {
        switch (type)
        {
        case GL_UNSIGNED_BYTE: { internalFormat = GL_RGBA8; return true; }
        case GL_HALF_FLOAT:
        case GL_FLOAT: { internalFormat = GL_RGBA16F; return true; }
        default: break;
        }
        break;
    }

    }

    return false;
}

bool Texture::stringToWrap(const std::string& str, GLenum& wrap)
{
    static const std::unordered_map<std::string, GLenum> s_table {
        {"GL_CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE},
        {"GL_CLAMP_TO_BORDER", GL_CLAMP_TO_BORDER},
        {"GL_MIRRORED_REPEAT", GL_MIRRORED_REPEAT},
        {"GL_REPEAT", GL_REPEAT}
    };

    auto it = s_table.find(str);

    if (it == s_table.end())
        return false;

    wrap = it->second;
    return true;
}

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filename)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(filename));
    if (!object)
    {
        GLuint id;

        if (utils::fileExt(filename) == "json")
        {
            static const std::vector<std::string> s_2dFields { "Image" };
            static const std::vector<std::string> s_cubemapFields { "Right", "Left", "Top", "Bottom", "Front", "Back" };

            const std::string dir = utils::fileDir(filename);

            static const auto is2D = [](const rapidjson::Document::ValueType& object)
            {
                bool flag = true;
                for (const auto& s : s_2dFields)
                    flag = flag && object.HasMember(s.c_str());
                return flag;
            };

            static const auto isCubemap = [](const rapidjson::Document::ValueType& object)
            {
                bool flag = true;
                for (const auto& s : s_cubemapFields)
                    flag = flag && object.HasMember(s.c_str());
                return flag;
            };

            static const auto readImageMipmaps = [](const rapidjson::Document::ValueType& object, std::vector<std::string>& mipmaps)
            {
                mipmaps.clear();

                if (object.IsString())
                {
                    mipmaps.push_back(object.GetString());
                }
                else if (object.IsArray())
                {
                    auto array = object.GetArray();
                    mipmaps.reserve(array.Size());
                    for (size_t i = 0; i < array.Size(); ++i)
                    {
                        if (!array[i].IsString())
                            return false;

                        mipmaps.push_back(array[i].GetString());
                    }
                }
                else
                    return false;

                return true;
            };

            QFile file(QString::fromStdString(filename));
            if (!file.open(QFile::ReadOnly))
                return nullptr;

            auto byteArray = file.readAll();
            file.close();

            rapidjson::Document document;
            document.Parse(byteArray);

            uint32_t numLayers = 0;
            std::vector<std::reference_wrapper<const rapidjson::Document::ValueType>> layerFields;
            while (document.HasMember(("Layer" + std::to_string(numLayers)).c_str()))
                layerFields.push_back(document[("Layer" + std::to_string(numLayers++)).c_str()]);
            if (!numLayers)
                layerFields.push_back(document);

            bool firstLayerIs2D;
            if (is2D(layerFields.front()))
                firstLayerIs2D = true;
            else if (isCubemap(layerFields.front()))
                firstLayerIs2D = false;
            else
                return nullptr;

            const auto& imageFields = firstLayerIs2D ? s_2dFields : s_cubemapFields;

            std::vector<std::vector<std::vector<std::string>>> imageFilenames; // layer/side/mipmap
            imageFilenames.resize(layerFields.size());

            for (size_t layer = 0 ; layer < layerFields.size(); ++layer)
            {
                imageFilenames[layer].resize(imageFields.size());
                for (size_t image = 0; image < imageFields.size(); ++image)
                {
                    if (!layerFields[layer].get().HasMember(imageFields[image].c_str()))
                        return nullptr;

                    if (!readImageMipmaps((layerFields[layer].get())[imageFields[image].c_str()], imageFilenames[layer][image]))
                         return nullptr;

                    if (imageFilenames[layer][image].front().empty()) // zero-mipmap can't be empty
                        return nullptr;
                }
            }

            size_t numMipmaps = 0;
            for (const auto& l : imageFilenames)
                for (const auto& s : l)
                    numMipmaps = std::max(numMipmaps, s.size());

            if (numMipmaps == 0) // it can't be true but if any...
                return nullptr;

            std::shared_ptr<Image> imageDesc;
            for (const auto& l : imageFilenames) {
                for (const auto& s : l) {
                    imageDesc = Image::loadDescription(dir + s.front());
                    break;
                }
                if (imageDesc) break;
            }
            if (!imageDesc)
                return nullptr;


            GLenum internalFormat;
            if (document.HasMember("InternalFormat"))
            {
                auto& internalFormatField = document["InternalFormat"];

                if (!internalFormatField.IsString())
                    return nullptr;

                if (!Texture::stringToInternalFormat(internalFormatField.GetString(), internalFormat))
                    return nullptr;
            }
            else
            {
                if (!Texture::formatAndTypeToInternalFormat(imageDesc->format(), imageDesc->type(), internalFormat))
                    return nullptr;
            }

            int32_t numGeneratedMipmaps = numMipmaps;
            bool autoGenMipmaps = false;
            int filter = (numMipmaps > 1) ? 3 : 2;

            if (document.HasMember("AutoGenMipmaps"))
            {
                auto& autoGenMipmapsField = document["AutoGenMipmaps"];
                if (!autoGenMipmapsField.IsBool())
                    return false;

                autoGenMipmaps = document["AutoGenMipmaps"].GetBool();
                if (autoGenMipmaps)
                {
                    filter = 3;
                    numGeneratedMipmaps = numberOfMipmaps(imageDesc->width(), imageDesc->height());
                }
            }

            if (document.HasMember("Filter"))
            {
                auto& filterField = document["Filter"];
                if (!filterField.IsInt())
                    return false;

                filter = document["Filter"].GetInt();
            }

            GLenum wrap = GL_REPEAT;
            if (document.HasMember("Wrap"))
            {
                auto& wrapField = document["Wrap"];
                if (!wrapField.IsString())
                    return nullptr;

                if (!Texture::stringToWrap(wrapField.GetString(), wrap))
                    return nullptr;
            }

            GLenum target = numLayers ? (firstLayerIs2D ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_CUBE_MAP_ARRAY) :
                                        (firstLayerIs2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP);

            m_functions.glGenTextures(1, &id);
            m_functions.glBindTexture(target, id);
            numLayers ? m_functions.glTexStorage3D(target, numGeneratedMipmaps, internalFormat, imageDesc->width(), imageDesc->height(), numLayers) :
                        m_functions.glTexStorage2D(target, numGeneratedMipmaps, internalFormat, imageDesc->width(), imageDesc->height());
            m_functions.glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, numGeneratedMipmaps-1);

            for (size_t layer = 0; layer < layerFields.size(); ++layer)
                for (size_t i = 0; i < imageFields.size(); ++i)
                    for (size_t level = 0; level < imageFilenames[layer][i].size(); ++level)
                    {
                        auto image = Image::load(dir + imageFilenames[layer][i][level]);
                        if (image)
                        {
                            numLayers ?
                                m_functions.glTexSubImage3D(firstLayerIs2D ? GL_TEXTURE_2D_ARRAY : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), level, 0, 0, layer, image->width(), image->height(), 1u, image->format(), image->type(), image->data()) :
                                m_functions.glTexSubImage2D(firstLayerIs2D ? GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), level, 0, 0, image->width(), image->height(), image->format(), image->type(), image->data());
                        }
                    }

            object = std::make_shared<Texture>(id, target, glm::uvec3(imageDesc->width(), imageDesc->height(), numLayers));
            object->setFilter(filter);
            object->setWrap(wrap);

            if (autoGenMipmaps)
                m_functions.glGenerateMipmap(target);
        }
        else
        {
            auto image = Image::load(filename);

            if (image == nullptr)
                return nullptr;

            GLenum internalFormat;
            if (!Texture::formatAndTypeToInternalFormat(image->format(), image->type(), internalFormat))
                return nullptr;

            int32_t numMipmaps = numberOfMipmaps(image->width(), image->height());

            m_functions.glGenTextures(1, &id);
            m_functions.glBindTexture(GL_TEXTURE_2D, id);
            m_functions.glTexStorage2D(GL_TEXTURE_2D, numMipmaps, internalFormat, image->width(), image->height());
            m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps-1);
            m_functions.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->width(), image->height(), image->format(), image->type(), image->data());
            m_functions.glGenerateMipmap(GL_TEXTURE_2D);

            object = std::make_shared<Texture>(id, GL_TEXTURE_2D, glm::uvec3(image->width(), image->height(), 0u));
            object->setFilter(3);
        }

        m_resourceStorage->store(filename, object);
    }

    return object;
}

std::shared_ptr<Texture> Renderer::createTexture2D(GLenum internalFormat,
                                                   GLint width,
                                                   GLint height,
                                                   GLenum format,
                                                   GLenum type,
                                                   const void *data,
                                                   uint32_t numMipmaps, // caclulate automatically if it's 0
                                                   const std::string &resourceName)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(resourceName));
    if (!object)
    {
        int32_t filter = 1;

        if (numMipmaps == 0)
            numMipmaps = numberOfMipmaps(width, height);

        GLuint id;
        m_functions.glGenTextures(1, &id);
        m_functions.glBindTexture(GL_TEXTURE_2D, id);
        m_functions.glTexStorage2D(GL_TEXTURE_2D, numMipmaps, internalFormat, width, height);
        m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps-1);

        if (data)
        {
            m_functions.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
            filter = 2;
        }

        if (numMipmaps > 1)
        {
            m_functions.glGenerateMipmap(GL_TEXTURE_2D);
            filter = 3;
        }

        object = std::make_shared<Texture>(id, GL_TEXTURE_2D, glm::uvec3(width, height, 0u));
        object->setFilter(filter);

        if (!resourceName.empty())
            m_resourceStorage->store(resourceName, object);
    }

    return object;
}

std::shared_ptr<Texture> Renderer::createTexture2DArray(GLenum internalFormat,
                                                        GLint width,
                                                        GLint height,
                                                        GLint numLayers,
                                                        GLenum format,
                                                        GLenum type,
                                                        const void *data,
                                                        const std::string &resourceName)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(resourceName));
    if (!object)
    {
        GLuint id;
        m_functions.glGenTextures(1, &id);
        m_functions.glBindTexture(GL_TEXTURE_2D_ARRAY, id);
        m_functions.glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, static_cast<GLint>(internalFormat), width, height, numLayers, 0, format, type, data);
        m_functions.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_functions.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        object = std::make_shared<Texture>(id, GL_TEXTURE_2D_ARRAY, glm::uvec3(width, height, numLayers));

        if (!resourceName.empty())
            m_resourceStorage->store(resourceName, object);
    }

    return object;
}

} // namespace
} // namespace
