#ifndef TEXTUREFORMATS_H
#define TEXTUREFORMATS_H

#include <string>
#include <unordered_map>

#include <QtOpenGL/QGL>

namespace trash
{
namespace utils
{

inline bool stringToInternalFormat(const std::string& str, GLenum& internalFormat)
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

inline bool formatAndTypeToInternalFormat(GLenum format, GLenum type, GLenum& internalFormat)
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

inline bool stringToWrap(const std::string& str, GLenum& wrap)
{
    static const std::unordered_map<std::string, GLenum> s_table {
        {"GL_CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE},
        {"GL_MIRRORED_REPEAT", GL_MIRRORED_REPEAT},
        {"GL_REPEAT", GL_REPEAT}
    };

    auto it = s_table.find(str);

    if (it == s_table.end())
        return false;

    wrap = it->second;
    return true;
}

} // namespace
} // namespace


#endif // TEXTUREFORMATS_H
