#include <memory>
#include <unordered_map>

#include <QtCore/QFile>
#include <QtGui/QOpenGLExtraFunctions>

#include <glm/gtc/type_ptr.hpp>

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
        m_functions.glGenTextures(1, &id);

        if (utils::fileExt(filename) == "json")
        {
            const std::string dir = utils::fileDir(filename);

            static const auto isImage = [](const rapidjson::Document::ValueType& object)
            {
                return object.HasMember("Image");
            };

            static const auto isCubemap = [](const rapidjson::Document::ValueType& object)
            {
                return
                        object.HasMember("Right") &&
                        object.HasMember("Left") &&
                        object.HasMember("Top") &&
                        object.HasMember("Bottom") &&
                        object.HasMember("Front") &&
                        object.HasMember("Back");
            };

            static const auto readImages = [](const rapidjson::Document::ValueType& object, GLenum target, const std::string& dir, std::array<std::shared_ptr<Image>, 6>& images) -> bool
            {
                if (target == GL_TEXTURE_2D)
                {
                    images[0] = Image::load(dir + object["Image"].GetString());
                    if (images[0] == nullptr)
                        return false;
                }
                else if (target == GL_TEXTURE_CUBE_MAP)
                {
                    images[0] = Image::load(dir + object["Right"].GetString());
                    images[1] = Image::load(dir + object["Left"].GetString());
                    images[2] = Image::load(dir + object["Top"].GetString());
                    images[3] = Image::load(dir + object["Bottom"].GetString());
                    images[4] = Image::load(dir + object["Front"].GetString());
                    images[5] = Image::load(dir + object["Back"].GetString());

                    for (size_t i = 0; i < 6; ++i)
                    {
                        if (images[i] == nullptr)
                            return false;
                    }
                }
                return true;
            };

            static const auto loadImages = [](QOpenGLExtraFunctions& f, GLenum target, GLint level, const std::array<std::shared_ptr<Image>, 6>& images)
            {
                if (target == GL_TEXTURE_2D)
                    f.glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, images[0]->width(), images[0]->height(), images[0]->format(), images[0]->type(), images[0]->data());
                else
                    for (size_t i = 0; i < 6; ++i)
                        f.glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(i), level, 0, 0, images[i]->width(), images[i]->height(), images[i]->format(), images[i]->type(), images[i]->data());
            };

            QFile file(QString::fromStdString(filename));
            if (!file.open(QFile::ReadOnly))
                return nullptr;

            auto byteArray = file.readAll();
            file.close();

            rapidjson::Document document;
            document.Parse(byteArray);

            int32_t numMipmaps = 0;
            while (document.HasMember(("Mipmap" + std::to_string(numMipmaps)).c_str()))
                ++numMipmaps;

            if (numMipmaps < 1)
                return nullptr;

            auto& mipmap0 = document["Mipmap0"];
            if (!isImage(mipmap0) && !isCubemap(mipmap0))
                return nullptr;

            GLenum target = isImage(mipmap0) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;

            std::array<std::shared_ptr<Image>, 6> images;
            if (!readImages(mipmap0, target, dir, images))
                return nullptr;

            GLenum internalFormat;
            if (document.HasMember("InternalFormat"))
            {
                if (!Texture::stringToInternalFormat(document["InternalFormat"].GetString(), internalFormat))
                    return nullptr;
            }
            else
            {
                if (!Texture::formatAndTypeToInternalFormat(images[0]->format(), images[0]->type(), internalFormat))
                    return nullptr;
            }

            int32_t numGeneratedMipmaps = numMipmaps;
            bool autoGenMipmaps = false;
            int filter = (numMipmaps > 1) ? 3 : 2;

            if (document.HasMember("AutoGenMipmaps"))
            {
                autoGenMipmaps = document["AutoGenMipmaps"].GetBool();
                if (autoGenMipmaps)
                {
                    filter = 3;
                    numGeneratedMipmaps = numberOfMipmaps(images[0]->width(), images[0]->height());
                }
            }

            if (document.HasMember("Filter"))
                filter = document["Filter"].GetInt();


            m_functions.glBindTexture(target, id);
            m_functions.glTexStorage2D(target, numGeneratedMipmaps, internalFormat, images[0]->width(), images[0]->height());
            m_functions.glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, numGeneratedMipmaps-1);

            loadImages(m_functions, target, 0, images);

            for (int n = 1; n < numMipmaps; ++n)
            {
                auto& mipmap = document[("Mipmap" + std::to_string(n)).c_str()];

                if (((target == GL_TEXTURE_2D) && !isImage(mipmap)) ||
                    ((target == GL_TEXTURE_CUBE_MAP) && !isCubemap(mipmap)))
                        return nullptr;

                if (!readImages(mipmap, target, dir, images))
                    return nullptr;

                loadImages(m_functions, target, n, images);
            }

            object = std::make_shared<Texture>(id, target);
            object->setFilter(filter);

            if (autoGenMipmaps)
                m_functions.glGenerateMipmap(target);

            if (document.HasMember("Wrap"))
            {
                GLenum wrap;
                if (!Texture::stringToWrap(document["Wrap"].GetString(), wrap))
                    return nullptr;
                object->setWrap(wrap);
            }
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

            m_functions.glBindTexture(GL_TEXTURE_2D, id);
            m_functions.glTexStorage2D(GL_TEXTURE_2D, numMipmaps, internalFormat, image->width(), image->height());
            m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps-1);
            m_functions.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->width(), image->height(), image->format(), image->type(), image->data());
            m_functions.glGenerateMipmap(GL_TEXTURE_2D);

            object = std::make_shared<Texture>(id, GL_TEXTURE_2D);
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

        object = std::make_shared<Texture>(id, GL_TEXTURE_2D);
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

        object = std::make_shared<Texture>(id, GL_TEXTURE_2D_ARRAY);

        if (!resourceName.empty())
            m_resourceStorage->store(resourceName, object);
    }

    return object;
}

} // namespace
} // namespace
