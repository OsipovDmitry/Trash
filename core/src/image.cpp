#include <algorithm>

#include <QtGui/QImage>

#include <utils/fileinfo.h>
#include <utils/noncopyble.h>

#include "hdrloader/hdrloader.h"
#include "image.h"

namespace trash
{
namespace core
{

class QtImage : public Image
{
    NONCOPYBLE(QtImage)

public:
    QtImage(QImage&& img)
        : Image()
        , m_img(std::move(img))
    {
        m_img = m_img.convertToFormat(m_img.hasAlphaChannel() ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    }

    static std::shared_ptr<QtImage> load(const std::string& filename) {
        QImage img(QString::fromStdString(filename));
        if (img.isNull())
            return nullptr;

        return std::make_shared<QtImage>(std::move(img));
    }

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
    NONCOPYBLE(HdrImage)

public:
    HdrImage(HDRLoaderResult&& result)
        : Image()
        , m_img(std::move(result))
    {
    }

    static std::shared_ptr<HdrImage> load(const std::string& filename) {
        HDRLoaderResult result;
        if (!HDRLoader::load(filename.c_str(), result))
            return nullptr;

        return std::make_shared<HdrImage>(std::move(result));
    }

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
        result = HdrImage::load(filename);
    else
        result = QtImage::load(filename);

    return result;
}

} // namespace
} // namespace
