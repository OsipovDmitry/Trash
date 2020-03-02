#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <string>
#include <memory>

#include <QtOpenGL/QGL>

#include <utils/noncopyble.h>

namespace trash
{
namespace core
{

class Image
{
    NONCOPYBLE(Image)
public:
    Image() = default;
    virtual ~Image() = default;

    virtual GLsizei width() const = 0;
    virtual GLsizei height() const = 0;
    virtual GLenum format() const = 0;
    virtual GLenum type() const = 0;
    virtual const void* data() const = 0;

    static std::shared_ptr<Image> load(const std::string&);
};

} // namespace
} // namespace

#endif // IMAGELOADER_H
