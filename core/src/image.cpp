#include <algorithm>

#include <QImage>

#include <utils/fileinfo.h>

#include "hdrloader/hdrloader.h"
#include "image.h"

namespace trash
{
namespace core
{

class QtImage : public Image
{
public:
    QtImage(const std::string& filename)
        : Image()
        , m_img(QString::fromStdString(filename))
    {
        m_img = m_img.convertToFormat(m_img.hasAlphaChannel() ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    }

    bool isLoaded() const override { return !m_img.isNull(); }

    GLsizei width() const override { return m_img.width(); }
    GLsizei height() const override { return m_img.height(); }
    GLenum format() const override { return m_img.hasAlphaChannel() ? GL_RGBA : GL_RGB; }
    GLenum type() const override { return GL_UNSIGNED_BYTE; }
    const void* data() const override { return m_img.bits(); }

private:
    QImage m_img;

};

class HdrImage : public Image
{
public:
    HdrImage(const std::string& filename)
        : Image()
    {
        HDRLoader::load(filename.c_str(), m_img);
    }

    bool isLoaded() const override { return m_img.width && m_img.height && !m_img.cols.empty(); }

    GLsizei width() const override { return m_img.width; }
    GLsizei height() const override { return m_img.height; }
    GLenum format() const override { return GL_RGB; }
    GLenum type() const override { return GL_FLOAT; }
    const void* data() const override { return m_img.cols.data(); }

private:
    HDRLoaderResult m_img;

};

std::shared_ptr<Image> Image::load(const std::string& filename)
{
    auto ext = utils::fileExt(filename);

    std::shared_ptr<Image> result;

    if (ext == "hdr")
        result = std::make_shared<HdrImage>(filename);
    else
        result = std::make_shared<QtImage>(filename);

    return result;
}

} // namespace
} // namespace
