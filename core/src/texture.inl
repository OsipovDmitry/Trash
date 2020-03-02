#include <memory>

#include <QtCore/QFile>
#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QImage>

#include <glm/gtc/type_ptr.hpp>

#include <utils/fileinfo.h>
#include <utils/textureformats.h>

#include "renderer.h"
#include "image.h"
#include "hdrloader/hdrloader.h"
#include "rapidjson/document.h"

namespace trash
{
namespace core
{

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filename)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(filename));
    if (!object)
    {
        GLuint id;
        m_functions.glGenTextures(1, &id);

        auto ext = utils::fileExt(filename);
        if (ext == "json")
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
                if (!utils::stringToInternalFormat(document["InternalFormat"].GetString(), internalFormat))
                    return nullptr;
            }
            else
            {
                if (!utils::formatAndTypeToInternalFormat(images[0]->format(), images[0]->type(), internalFormat))
                    return nullptr;
            }

            m_functions.glBindTexture(target, id);
            m_functions.glTexStorage2D(target, numMipmaps, internalFormat, images[0]->width(), images[0]->height());
            m_functions.glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, numMipmaps-1);

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

            bool autoGenMipmaps = false;
            int filter = (numMipmaps > 1) ? 3 : 2;

            if (document.HasMember("AutoGenMipmaps"))
                autoGenMipmaps = document["AutoGenMipmaps"].GetBool();

            if (document.HasMember("Filter"))
                filter = document["Filter"].GetInt();

            object = std::make_shared<Texture>(id, target);
            object->setFilter(filter);

            if (autoGenMipmaps)
                object->generateMipmaps();

            if (document.HasMember("Wrap"))
            {
                GLenum wrap;
                if (!utils::stringToWrap(document["Wrap"].GetString(), wrap))
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
            if (!utils::formatAndTypeToInternalFormat(image->format(), image->type(), internalFormat))
                return nullptr;

            m_functions.glBindTexture(GL_TEXTURE_2D, id);
            m_functions.glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), image->width(), image->height(), 0, image->format(), image->type(), image->data());
            m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            object = std::make_shared<Texture>(id, GL_TEXTURE_2D);
            object->generateMipmaps();
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
                                                   const std::string &resourceName)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(resourceName));
    if (!object)
    {
        GLuint id;
        m_functions.glGenTextures(1, &id);
        m_functions.glBindTexture(GL_TEXTURE_2D, id);
        m_functions.glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), width, height, 0, format, type, data);
        m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        object = std::make_shared<Texture>(id, GL_TEXTURE_2D);

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
