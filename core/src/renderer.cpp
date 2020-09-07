#include <array>

#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtCore/QFile>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <utils/fileinfo.h>

#include <core/core.h>

#include "coreprivate.h"
#include "renderwidget.h"
#include "renderer.h"
#include "resourcestorage.h"
#include "drawables.h"
#include "resources.h"
#include "importexport.h"
#include "model.inl"
#include "texture.inl"

#include <iostream>

namespace trash
{
namespace core
{

RenderProgram::~RenderProgram()
{
    auto& functions = Renderer::instance().functions();

    GLuint shaders[2];
    GLsizei count = 0;
    functions.glGetAttachedShaders(id, 2, &count, shaders);
    functions.glDetachShader(id, shaders[0]);
    functions.glDeleteShader(shaders[0]);
    functions.glDetachShader(id, shaders[1]);
    functions.glDeleteShader(shaders[1]);
    functions.glDeleteProgram(id);
}

void RenderProgram::setupTransformFeedback(const std::vector<std::string>& varyings, GLenum mode)
{
    std::vector<const char*> names;
    names.reserve(varyings.size());
    for (const auto& varying : varyings)
        names.push_back(varying.c_str());

    auto& functions = Renderer::instance().functions();

    functions.glTransformFeedbackVaryings(id, static_cast<GLsizei>(names.size()), names.data(), mode);
    functions.glLinkProgram(id);

    GLint linked;
    functions.glGetProgramiv(id, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        functions.glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1) {
            char *infoLog = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(infoLen)));
            functions.glGetProgramInfoLog(id, infoLen, nullptr, infoLog);
            std::cout << "Transform feedback link: " << infoLog << std::endl;
            free(infoLog);
        }
    }
}

GLuint RenderProgram::uniformBufferIndexByName(const std::string& name)
{
    return Renderer::instance().functions().glGetUniformBlockIndex(id, name.c_str());
}

void RenderProgram::setUniformBufferBinding(GLuint index, GLuint unit)
{
    Renderer::instance().functions().glUniformBlockBinding(id, index, unit);
}

GLint RenderProgram::uniformBufferDataSize(GLuint blockIndex)
{
    GLint val = -1;
    Renderer::instance().functions().glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &val);
    return val;
}

std::unordered_map<std::string, GLint> RenderProgram::uniformBufferOffsets(GLuint blockIndex)
{
    auto& functions = Renderer::instance().functions();

    std::unordered_map<std::string, int32_t> result;

    GLint count = -1;
    functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &count);

    if (count >= 0)    {
        std::vector<GLuint> indices(static_cast<size_t>(count));
        functions.glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast<GLint*>(indices.data()));

        std::vector<GLint> nameLengths(static_cast<size_t>(count));
        std::string name;
        std::vector<GLint> offsets(static_cast<size_t>(count));
        GLsizei length;
        GLint size;
        GLenum type;
        functions.glGetActiveUniformsiv(id, count, indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());
        functions.glGetActiveUniformsiv(id, count, indices.data(), GL_UNIFORM_OFFSET, offsets.data());

        for (size_t i = 0; i < static_cast<size_t>(count); ++i)
        {
            name.resize(static_cast<size_t>(nameLengths[i]));
            functions.glGetActiveUniform(id, indices[i], nameLengths[i], &length, &size, &type, &(name[0]));
            name.resize(static_cast<size_t>(nameLengths[i])-1);
            result[name] = offsets[i];
        }
    }

    return result;
}

GLint RenderProgram::uniformLocation(const std::string& name)
{
    return Renderer::instance().functions().glGetUniformLocation(id, name.c_str());
}

void RenderProgram::setUniform(GLint loc, GLint value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniform1i(loc, value);
}

void RenderProgram::setUniform(GLint loc, float value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniform1f(loc, value);
}

void RenderProgram::setUniform(GLint loc, const glm::vec3& value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniform3fv(loc, 1, glm::value_ptr(value));
}

void RenderProgram::setUniform(GLint loc, const glm::vec4& value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniform4fv(loc, 1, glm::value_ptr(value));
}

void RenderProgram::setUniform(GLint loc, const glm::mat3x3& value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniformMatrix3fv(loc, 1, false, glm::value_ptr(value));
}

void RenderProgram::setUniform(GLint loc, const glm::mat4x4& value)
{
    auto& functions = Renderer::instance().functions();
    functions.glUseProgram(id);
    functions.glUniformMatrix4fv(loc, 1, false, glm::value_ptr(value));
}

Texture::~Texture()
{
    Renderer::instance().functions().glDeleteTextures(1, &id);
}

void Texture::generateMipmaps()
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    functions.glGenerateMipmap(target);
    functions.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    functions.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::setFilter(int32_t value)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    if (value == 1) {
        functions.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        functions.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else if (value == 2) {
        functions.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        functions.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if (value == 3) {
        functions.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        functions.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void Texture::setWrap(GLenum wrap)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    functions.glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap));
    functions.glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap));
    functions.glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLint>(wrap));
}

void Texture::setCompareMode(GLenum value)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    functions.glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, static_cast<GLint>(value));
}

void Texture::setCompareFunc(GLenum value)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    functions.glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(value));
}

void Texture::setBorderColor(const glm::vec4& value)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);
    functions.glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(value));
}

int32_t Texture::numMipmapLevels() const
{
    auto& functions = Renderer::instance().functions();

    functions.glBindTexture(target, id);

    GLint res;
    functions.glGetTexParameteriv(target, GL_TEXTURE_MAX_LEVEL, &res);

    return res;
}

Buffer::Buffer(GLsizeiptr size, const GLvoid *data, GLenum usage)
    : id(0)
    , m_cpuData(nullptr)
    , m_cpuDataIsDirty(true)
{
    auto& functions = Renderer::instance().functions();
    if (size)
    {
        functions.glGenBuffers(1, &id);
        functions.glBindBuffer(GL_ARRAY_BUFFER, id);
        functions.glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }
}

Buffer::~Buffer()
{
    Renderer::instance().functions().glDeleteBuffers(1, &id);
    delete [] m_cpuData;
}

int64_t Buffer::size() const
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBuffer(GL_ARRAY_BUFFER, id);

    GLint64 result;
    functions.glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &result);

    return result;
}

void Buffer::setSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBuffer(GL_ARRAY_BUFFER, id);
    functions.glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    m_cpuDataIsDirty = true;
}

void *Buffer::map(GLintptr offset, GLsizeiptr size, GLbitfield access)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBuffer(GL_ARRAY_BUFFER, id);
    m_cpuDataIsDirty = true;
    return functions.glMapBufferRange(GL_ARRAY_BUFFER, offset, size, access);
}

void Buffer::unmap()
{
    Renderer::instance().functions().glUnmapBuffer(GL_ARRAY_BUFFER);
}

const void *Buffer::cpuData() const
{
    if (m_cpuDataIsDirty)
    {
        auto nonConstThis = const_cast<Buffer*>(this);
        int64_t dataSize = size();

        if (!m_cpuData)
            nonConstThis->m_cpuData = new uint8_t [dataSize];

        std::memcpy(m_cpuData, nonConstThis->map(0, dataSize, GL_MAP_READ_BIT), dataSize);
        unmap();

        nonConstThis->m_cpuDataIsDirty = false;
    }
    return m_cpuData;
}

void Buffer::clearCpuData()
{
    delete [] m_cpuData;
    m_cpuData = nullptr;
    m_cpuDataIsDirty = true;
}

VertexBuffer::VertexBuffer(uint32_t nv, uint32_t nc, const float *data, GLenum usage)
    : Buffer(static_cast<GLsizeiptr>(nv*nc*sizeof(float)), data, usage)
    , numVertices(nv)
    , numComponents(nc)
{
}

IndexBuffer::IndexBuffer(GLenum primitiveType_, uint32_t numIndices_, const uint32_t *data, GLenum usage)
    : Buffer(static_cast<GLsizeiptr>(numIndices_ * sizeof(uint32_t)), data, usage)
    , numIndices(numIndices_)
    , primitiveType(primitiveType_)
{
}

Mesh::Mesh()
{
    Renderer::instance().functions().glGenVertexArrays(1, &id);
}

Mesh::~Mesh()
{
    Renderer::instance().functions().glDeleteVertexArrays(1, &id);
}

void Mesh::declareVertexAttribute(VertexAttribute attrib, std::shared_ptr<VertexBuffer> vb)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindVertexArray(id);
    functions.glBindBuffer(GL_ARRAY_BUFFER, vb->id);
    functions.glVertexAttribPointer(castFromVertexAttribute(attrib), static_cast<GLint>(vb->numComponents), GL_FLOAT, GL_FALSE, 0, nullptr);
    functions.glEnableVertexAttribArray(castFromVertexAttribute(attrib));
    functions.glBindVertexArray(0);

    attributesDeclaration[attrib] = vb;


    if (attrib == VertexAttribute::Position)
    {
        assert(vb->numComponents == 2 || vb->numComponents == 3);
        auto *p = vb->map(0, vb->numVertices * vb->numComponents * sizeof(float), GL_MAP_READ_BIT);
        if (vb->numComponents == 3)
            boundingBox = utils::BoundingBox(static_cast<glm::vec3*>(p), vb->numVertices);
        else if (vb->numComponents == 2)
            boundingBox = utils::BoundingBox(static_cast<glm::vec2*>(p), vb->numVertices);
        vb->unmap();
    }
}

void Mesh::undeclareVertexAttribute(VertexAttribute attrib)
{
    auto it = attributesDeclaration.find(attrib);
    if (it != attributesDeclaration.end())
    {
        auto& functions = Renderer::instance().functions();

        functions.glBindVertexArray(id);
        functions.glDisableVertexAttribArray(castFromVertexAttribute(attrib));
        functions.glBindVertexArray(0);

        attributesDeclaration.erase(it);
    }
}

std::shared_ptr<VertexBuffer> Mesh::vertexBuffer(VertexAttribute attrib) const
{
    auto it = attributesDeclaration.find(attrib);
    return (it != attributesDeclaration.end()) ? it->second : nullptr;
}

void Mesh::attachIndexBuffer(std::shared_ptr<IndexBuffer> b)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindVertexArray(id);
    functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->id);
    functions.glBindVertexArray(0);

    indexBuffers.insert(b);
}

Renderbuffer::Renderbuffer(GLenum internalFormat, GLsizei width, GLsizei height)
{
    auto& functions = Renderer::instance().functions();
    functions.glGenRenderbuffers(1, &id);
    functions.glBindRenderbuffer(GL_RENDERBUFFER, id);
    functions.glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
}

Renderbuffer::~Renderbuffer()
{
    auto& functions = Renderer::instance().functions();
    functions.glDeleteRenderbuffers(1, &id);
}

Framebuffer::Framebuffer()
{
    auto& functions = Renderer::instance().functions();
    functions.glGenFramebuffers(1, &id);
}

Framebuffer::~Framebuffer()
{
    for (size_t i = 0; i < colorAttachments.size(); ++i)
        detachColor(i);
    detachDepth();

    auto& functions = Renderer::instance().functions();
    functions.glDeleteFramebuffers(1, &id);
}

void Framebuffer::detachColor(size_t idx)
{
    if (!colorAttachments[idx])
        return;

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (colorAttachments[idx]->isTexture())
    {
        if (colorAttachments[idx]->texture->target == GL_TEXTURE_2D || colorAttachments[idx]->texture->target == GL_TEXTURE_CUBE_MAP)
            functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        else if (colorAttachments[idx]->texture->target == GL_TEXTURE_2D_ARRAY)
            functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
    }
    else if (colorAttachments[idx]->isRenderbuffer())
    {
        functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
    }

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());

    colorAttachments[idx] = nullptr;
}

void Framebuffer::detachDepth()
{
    if (!depthAttachment)
        return;

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (depthAttachment->isTexture())
    {
        if (depthAttachment->texture->target == GL_TEXTURE_2D || depthAttachment->texture->target == GL_TEXTURE_CUBE_MAP)
            functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        else if (depthAttachment->texture->target == GL_TEXTURE_2D_ARRAY)
            functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
    }
    else if (depthAttachment->isRenderbuffer())
    {
        functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    }

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());

    depthAttachment = nullptr;
}

void Framebuffer::attachColor(size_t idx, std::shared_ptr<Texture> texture, uint32_t layer)
{
    detachColor(idx);

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (texture->target == GL_TEXTURE_2D)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_2D, texture->id, 0);
    else if (texture->target == GL_TEXTURE_CUBE_MAP)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, texture->id, 0);
    else if (texture->target == GL_TEXTURE_2D_ARRAY)
        functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, texture->id, 0, static_cast<GLint>(layer));

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    colorAttachments[idx] = std::make_shared<RenderTarget>(texture);
}

void Framebuffer::attachColor(size_t idx, std::shared_ptr<Renderbuffer> renderbuffer)
{
    detachColor(idx);

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, GL_RENDERBUFFER, renderbuffer->id);
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    colorAttachments[idx] = std::make_shared<RenderTarget>(renderbuffer);
}

void Framebuffer::attachDepth(std::shared_ptr<Texture> texture, uint32_t layer)
{
    detachDepth();

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (texture->target == GL_TEXTURE_2D)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->id, 0);
    else if (texture->target == GL_TEXTURE_CUBE_MAP)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, texture->id, 0);
    else if (texture->target == GL_TEXTURE_2D_ARRAY)
        functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->id, 0, static_cast<GLint>(layer));

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    depthAttachment = std::make_shared<RenderTarget>(texture);
}

void Framebuffer::attachDepth(std::shared_ptr<Renderbuffer> renderbuffer)
{
    detachDepth();

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer->id);
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    depthAttachment = std::make_shared<RenderTarget>(renderbuffer);
}

void Framebuffer::drawBuffers(const std::vector<GLenum>& buffers)
{
    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glDrawBuffers(buffers.size(), buffers.data());
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
}

Renderer::Renderer(QOpenGLExtraFunctions& functions, GLuint defaultFbo)
    : m_functions(functions)
    , m_defaultFbo(defaultFbo)
    , m_resourceStorage(std::make_unique<ResourceStorage>())
    , m_drawData()
    , m_projMatrix(1.0f)
    , m_viewMatrix(1.0f)
{
}

void Renderer::initializeResources()
{
    m_functions.glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    m_functions.glEnable(GL_CULL_FACE);

    m_brdfLutMap = loadTexture(brdfLutTextureName);

//    const float L = 2500;
//    const float wrap = 14;
//    std::vector<glm::vec3> pos {glm::vec3(-L, 0.0f, -L), glm::vec3(-L, 0.0f, +L), glm::vec3(+L, 0.0f, -L), glm::vec3(+L, 0.0f, +L)};
//    std::vector<glm::vec3> t {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)};
//    std::vector<glm::vec3> n {glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f)};
//    std::vector<glm::vec2> tc {glm::vec2(0.f, 0.f), glm::vec2(0.f, wrap), glm::vec2(wrap, 0.f), glm::vec2(wrap, wrap)};
//    std::vector<uint32_t> indices {0, 1, 2, 3};

//    auto mesh = std::make_shared<Mesh>();
//    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(4, 3, &pos[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::Normal, std::make_shared<VertexBuffer>(4, 3, &n[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::Tangent, std::make_shared<VertexBuffer>(4, 3, &t[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::TexCoord, std::make_shared<VertexBuffer>(4, 2, &tc[0].x, GL_STATIC_DRAW));
//    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLE_STRIP, 4, indices.data(), GL_STATIC_DRAW));

//    auto mat = std::make_shared<Model::Material>();
//    mat->diffuseTexture = std::make_pair(std::string("textures/floor.jpg"), loadTexture("textures/floor.jpg"));

//    auto mdl = std::make_shared<Model>();
//    mdl->rootNode = std::make_shared<Model::Node>();
//    mdl->rootNode->meshes.push_back(std::make_shared<Model::Mesh>(mesh, mat));

//    std::ofstream f("floor.mdl", std::ios_base::binary);
//    push(f, mdl);
//    f.close();

//    std::vector<std::string> names {"liam", "stefani", "shae", "malcolm", "regina"};
//    for (auto& n : names)
//    {
//        auto mdl = loadModel(n+".dae");
//        mdl->animations.clear();
//        std::ofstream file(n+".mdl", std::ios_base::binary);
//        push(file, mdl);
//        file.close();
//    }

}

Renderer &Renderer::instance()
{
    return Core::instance().m().renderWidget->renderer();
}

QOpenGLExtraFunctions& Renderer::functions()
{
    return m_functions;
}

GLuint Renderer::defaultFbo() const
{
    return m_defaultFbo;
}

std::shared_ptr<RenderProgram> Renderer::loadRenderProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
    const std::string key = vertexFile+fragmentFile;
    auto object = std::dynamic_pointer_cast<RenderProgram>(m_resourceStorage->get(key));
    if (!object)
    {
        std::array<std::pair<GLenum, std::string>, 2> shaderFilenames {
            std::make_pair(GL_VERTEX_SHADER, vertexFile),
            std::make_pair(GL_FRAGMENT_SHADER, fragmentFile)
        };

        GLuint shaderIds[2];
        bool isOk = true;
        for (size_t i = 0; i < 2; ++i)
        {
            auto& shader = shaderFilenames[i];
            auto dir = utils::fileDir(shader.second);

            QFile file(QString::fromStdString(shader.second));

            if (!file.open(QFile::ReadOnly))
            {
                isOk = false;
                continue;
            }

            auto byteArray = file.readAll();
            auto errorString = precompileShader(QString::fromStdString(dir), byteArray);
            if (!errorString.empty())
            {
                std::cout << shader.second << ": " << errorString << std::endl;
                isOk = false;
                continue;
            }

            const char *data = byteArray.data();

            shaderIds[i] = m_functions.glCreateShader(shader.first);
            m_functions.glShaderSource(shaderIds[i], 1, &data, nullptr);
            m_functions.glCompileShader(shaderIds[i]);
            GLint compiled;
            m_functions.glGetShaderiv(shaderIds[i], GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                m_functions.glGetShaderiv(shaderIds[i], GL_INFO_LOG_LENGTH, &infoLen);
                if(infoLen > 1)
                {
                    char *infoLog = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(infoLen)));
                    m_functions.glGetShaderInfoLog(shaderIds[i], infoLen, nullptr, infoLog);
                    std::cout << shader.second << ": " << infoLog << std::endl;
                    free(infoLog);
                }
                m_functions.glDeleteShader(shaderIds[i]);
            }

            file.close();
        }

        GLuint programId = 0;
        if (isOk)
        {
            programId = m_functions.glCreateProgram();
            m_functions.glAttachShader(programId, shaderIds[0]);
            m_functions.glAttachShader(programId, shaderIds[1]);
            m_functions.glLinkProgram(programId);
            GLint linked;
            m_functions.glGetProgramiv(programId, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLint infoLen = 0;
                m_functions.glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
                if(infoLen > 1) {
                    char *infoLog = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(infoLen)));
                    m_functions.glGetProgramInfoLog(programId, infoLen, nullptr, infoLog);
                    std::cout << vertexFile << " " << fragmentFile << " link: " << infoLog << std::endl;
                    free(infoLog);
                }
                m_functions.glDeleteProgram(programId);
            }
        }

        object = std::make_shared<RenderProgram>(programId);
        m_resourceStorage->store(key, object);
    }

    return object;
}



std::shared_ptr<Model::Animation> Renderer::loadAnimation(const std::string& filename)
{
    auto object = std::dynamic_pointer_cast<Model::Animation>(m_resourceStorage->get(filename));
    if (!object)
    {
        std::shared_ptr<Model::Animation> anim;
        std::ifstream file(filename, std::ios_base::binary);
        pull(file, object);
        file.close();

        m_resourceStorage->store(filename, object);
    }
    return object;
}

std::shared_ptr<Font> Renderer::loadFont(const std::string& filename)
{
    auto object = std::dynamic_pointer_cast<Font>(m_resourceStorage->get(filename));
    if (!object)
    {
        QFile file(QString::fromStdString(filename));
        if (!file.open(QFile::ReadOnly))
            return nullptr;

        auto byteArray = file.readAll();

        rapidjson::Document document;
        document.Parse(byteArray);

        if (!document.HasMember("texture") ||
                !document.HasMember("size") ||
                !document.HasMember("width") ||
                !document.HasMember("height") ||
                !document.HasMember("characters"))
            return nullptr;

        const std::string dir = utils::fileDir(filename);

        object = std::make_shared<Font>();
        object->texture = loadTexture(dir + document["texture"].GetString());
        if (!object->texture)
            return nullptr;

        object->size = document["size"].GetUint();
        object->width = document["width"].GetUint();
        object->height = document["height"].GetUint();

        // chech that width and height are equal texture's size

        if (document.HasMember("name"))
            object->name = document["name"].GetString();

        if (document.HasMember("bold"))
            object->isBold = document["bold"].GetBool();

        if (document.HasMember("italic"))
            object->isItalic = document["italic"].GetBool();

        if (document.HasMember("characters"))
        {
            rapidjson::Document::ValueType& characters = document["characters"];
            for (auto it = characters.MemberBegin(); it != characters.MemberEnd(); ++it)
            {
                const std::string name = it->name.GetString();
                if (name.length() != 1)
                    return nullptr;

                auto& value = it->value;
                if (!value.HasMember("x") ||
                        !value.HasMember("y") ||
                        !value.HasMember("width") ||
                        !value.HasMember("height") ||
                        !value.HasMember("originX") ||
                        !value.HasMember("originY") ||
                        !value.HasMember("advance"))
                    return nullptr;

                object->characters[static_cast<size_t>(name.at(0))] = std::make_shared<Font::Character>(
                        value["x"].GetUint(),
                        value["y"].GetUint(),
                        value["width"].GetUint(),
                        value["height"].GetUint(),
                        value["originX"].GetInt(),
                        value["originY"].GetInt(),
                        value["advance"].GetUint());
            }
        }

        m_resourceStorage->store(filename, object);
    }

    return object;
}


void Renderer::bindTexture(std::shared_ptr<Texture> texture, GLint unit)
{
    GLuint id = texture ? texture->id : 0;
    GLenum target = texture ? texture->target : GL_TEXTURE_2D;
    m_functions.glActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
    m_functions.glBindTexture(target, id);
}

void Renderer::bindUniformBuffer(std::shared_ptr<Buffer> buffer, GLuint unit)
{
    GLuint id = buffer ? buffer->id : 0;
    m_functions.glBindBufferBase(GL_UNIFORM_BUFFER, unit, id);
}

void Renderer::draw(std::shared_ptr<Drawable> drawable, const utils::Transform& transform)
{
    m_drawData[castFromLayerId(drawable->layerId())].push_back(std::make_pair(drawable, transform));
}

void Renderer::render(std::shared_ptr<Framebuffer> framebuffer)
{
    using RenderMethod = void(Renderer::*)(DrawDataLayerContainer&);
    static const std::array<RenderMethod, numElementsLayerId()> renderMethods {
        &Renderer::renderSolidLayer, // render selection layer as solid
        &Renderer::renderShadowLayer, // render shadows layer as solid
        &Renderer::renderBackgroundLayer,
        &Renderer::renderSolidLayer,
        &Renderer::renderTransparentLayer
    };

    GLuint framebufferId = framebuffer ? framebuffer->id : m_defaultFbo;
    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    m_functions.glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    m_functions.glClearDepthf(m_clearDepth);
    m_functions.glClear(calcClearMask());

    m_functions.glViewport(m_viewport.x, m_viewport.y, m_viewport.z, m_viewport.w);

    for (uint32_t layerId = 0; layerId < numElementsLayerId(); ++layerId)
    {
        auto& dataLayer = m_drawData.at(layerId);
        (this->*renderMethods[layerId])(dataLayer);
        dataLayer.clear();
    }

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFbo);
}

void Renderer::readPixel(std::shared_ptr<Framebuffer> framebuffer, int xi, int yi, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a, float& depth) const
{
    GLuint framebufferId = framebuffer ? framebuffer->id : m_defaultFbo;
    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    uint8_t color[4];
    m_functions.glReadBuffer(GL_COLOR_ATTACHMENT0);
    m_functions.glReadPixels(xi, m_viewport.w - yi - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    r = color[0];
    g = color[1];
    b = color[2];
    a = color[3];

    m_functions.glReadBuffer(GL_DEPTH_ATTACHMENT);
    m_functions.glReadPixels(xi, m_viewport.w - yi - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFbo);
}

void Renderer::setViewport(const glm::ivec4& value)
{
    m_viewport = value;
}

void Renderer::setViewMatrix(const glm::mat4x4& value)
{
    m_viewMatrix = value;
    m_viewProjMatrix = m_projMatrix * m_viewMatrix;
    m_viewProjMatrixInverse = glm::inverse(m_viewProjMatrix);
    m_viewMatrixInverse = glm::inverse(m_viewMatrix);
    m_viewPosition = glm::vec3(m_viewMatrixInverse * glm::vec4(0.f, 0.f, 0.f, 1.f));
}

void Renderer::setProjectionMatrix(const glm::mat4x4& value)
{
    m_projMatrix = value;
    m_viewProjMatrix = m_projMatrix * m_viewMatrix;
    m_viewProjMatrixInverse = glm::inverse(m_viewProjMatrix);
}

void Renderer::setClearColor(bool state, const glm::vec4& value)
{
    m_clearColorBit = state;
    m_clearColor = value;
}

void Renderer::setClearDepth(bool state, float value)
{
    m_clearDepthBit = state;
    m_clearDepth = value;
}

void Renderer::setLightsBuffer(std::shared_ptr<Buffer> lb)
{
    m_lightsBuffer = lb;
}

void Renderer::setShadowMaps(std::shared_ptr<Texture> sm)
{
    m_shadowMaps = sm;
}

void Renderer::setIBLMaps(std::shared_ptr<Texture> dm, std::shared_ptr<Texture> sm)
{
    m_IBLDiffuseMap = dm;
    m_IBLSpecularMap = sm;

    m_numIBLSpecularMapsMipmaps = m_IBLSpecularMap ? m_IBLSpecularMap->numMipmapLevels() : 0;
}

void Renderer::renderShadowLayer(DrawDataLayerContainer& data)
{
    m_functions.glEnable(GL_DEPTH_TEST);
    setupAndRender(data, FaceRenderOrder::OnlyBack);
}

void Renderer::renderBackgroundLayer(DrawDataLayerContainer& data)
{
    m_functions.glDisable(GL_DEPTH_TEST);
    setupAndRender(data, FaceRenderOrder::OnlyFront);
}

void Renderer::renderSolidLayer(DrawDataLayerContainer& data)
{
    m_functions.glEnable(GL_DEPTH_TEST);
    setupAndRender(data, FaceRenderOrder::OnlyFront);
}

void Renderer::renderTransparentLayer(DrawDataLayerContainer& data)
{
    std::sort(data.begin(), data.end(), [this](const DrawDataLayerContainer::value_type& first, const DrawDataLayerContainer::value_type& second) -> bool
    {
        const glm::vec3 v1 = first.second.translation - m_viewPosition;
        const glm::vec3 v2 = second.second.translation - m_viewPosition;

        return glm::dot(v1,v1) > glm::dot(v2,v2);
    });

    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glEnable(GL_BLEND);
    m_functions.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    setupAndRender(data, FaceRenderOrder::IndirectOrder);
    m_functions.glDisable(GL_BLEND);
}

void Renderer::setupAndRender(DrawDataLayerContainer& data, FaceRenderOrder faceRenderOrder)
{
    for (const auto& drawData : data)
    {
        auto drawable = drawData.first;
        const auto& transform = drawData.second;

        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        glm::mat4x4 modelMatrix = transform.operator glm::mat4x4();
        glm::mat3x3 normalMatrix = glm::inverseTranspose(modelMatrix);
        glm::mat4x4 modelViewMatrix = m_viewMatrix * modelMatrix;
        glm::mat4x4 modelViewProjMatrix = m_viewProjMatrix * modelMatrix;

        auto projMatrixLoc = renderProgram->uniformLocation("u_projMatrix");
        if (projMatrixLoc != -1) renderProgram->setUniform(projMatrixLoc, m_projMatrix);

        auto viewMatrixLoc = renderProgram->uniformLocation("u_viewMatrix");
        if (viewMatrixLoc != -1) renderProgram->setUniform(viewMatrixLoc, m_viewMatrix);

        auto viewProjMatrixLoc = renderProgram->uniformLocation("u_viewProjMatrix");
        if (viewProjMatrixLoc != -1) renderProgram->setUniform(viewProjMatrixLoc, m_viewProjMatrix);

        auto viewProjMatrixInverseLoc = renderProgram->uniformLocation("u_viewProjMatrixInverse");
        if (viewProjMatrixInverseLoc != -1) renderProgram->setUniform(viewProjMatrixInverseLoc, m_viewProjMatrixInverse);

        auto modelMatrixLoc = renderProgram->uniformLocation("u_modelMatrix");
        if (modelMatrixLoc != -1) renderProgram->setUniform(modelMatrixLoc, modelMatrix);

        auto normalMatrixLoc = renderProgram->uniformLocation("u_normalMatrix");
        if (normalMatrixLoc != -1) renderProgram->setUniform(normalMatrixLoc, normalMatrix);

        auto modelViewMatrixLoc = renderProgram->uniformLocation("u_modelViewMatrix");
        if (modelViewMatrixLoc != -1) renderProgram->setUniform(modelViewMatrixLoc, modelViewMatrix);

        auto modelViewProjMatrixLoc = renderProgram->uniformLocation("u_modelViewProjMatrix");
        if (modelViewProjMatrixLoc != -1) renderProgram->setUniform(modelViewProjMatrixLoc, modelViewProjMatrix);

        auto viewPositionLoc = renderProgram->uniformLocation("u_viewPosition");
        if (viewPositionLoc != -1) renderProgram->setUniform(viewPositionLoc, m_viewPosition);

        auto iblDiffuseMapLoc = renderProgram->uniformLocation("u_diffuseIBLMap");
        if ((iblDiffuseMapLoc != -1) && m_IBLDiffuseMap) {
            renderProgram->setUniform(iblDiffuseMapLoc, castFromTextureUnit(TextureUnit::DiffuseIBL));
            bindTexture(m_IBLDiffuseMap, castFromTextureUnit(TextureUnit::DiffuseIBL));
        }

        auto iblSpecularMapLoc = renderProgram->uniformLocation("u_specularIBLMap");
        if ((iblSpecularMapLoc != -1) && m_IBLSpecularMap) {
            renderProgram->setUniform(iblSpecularMapLoc, castFromTextureUnit(TextureUnit::SpecularIBL));
            bindTexture(m_IBLSpecularMap, castFromTextureUnit(TextureUnit::SpecularIBL));
        }

        auto numIBLSpecularMapsMipmapsLoc = renderProgram->uniformLocation("u_numSpecularIBLMapLods");
        if (numIBLSpecularMapsMipmapsLoc != -1) renderProgram->setUniform(numIBLSpecularMapsMipmapsLoc, m_numIBLSpecularMapsMipmaps);

        auto brdfLutMapLoc = renderProgram->uniformLocation("u_brdfLUT");
        if ((brdfLutMapLoc != -1) && m_brdfLutMap) {
            renderProgram->setUniform(brdfLutMapLoc, castFromTextureUnit(TextureUnit::BrdfLUT));
            bindTexture(m_brdfLutMap, castFromTextureUnit(TextureUnit::BrdfLUT));
        }

        auto iblContributionLoc = renderProgram->uniformLocation("u_IBLContribution");
        if (iblContributionLoc != -1) renderProgram->setUniform(iblContributionLoc, m_IBLContribution);

        auto lightsBufferIndex = renderProgram->uniformBufferIndexByName("u_lightsBuffer");
        if ((lightsBufferIndex != static_cast<GLuint>(-1)) && m_lightsBuffer)
        {
            renderProgram->setUniformBufferBinding(lightsBufferIndex, 1);
            bindUniformBuffer(m_lightsBuffer, 1);
        }

        auto shadowMapsLoc = renderProgram->uniformLocation("u_shadowMaps");
        if ((shadowMapsLoc != -1) && m_shadowMaps) {
            renderProgram->setUniform(shadowMapsLoc, castFromTextureUnit(TextureUnit::ShadowMaps));
            bindTexture(m_shadowMaps, castFromTextureUnit(TextureUnit::ShadowMaps));
        }

        drawable->prerender();

        auto mesh = drawable->mesh();
        if (mesh)
        {
            m_functions.glBindVertexArray(mesh->id);
            switch (faceRenderOrder) {
            case FaceRenderOrder::OnlyBack:
                m_functions.glCullFace(GL_FRONT);
                for (auto ibo : mesh->indexBuffers)
                    m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
                break;
            case FaceRenderOrder::OnlyFront:
                m_functions.glCullFace(GL_BACK);
                for (auto ibo : mesh->indexBuffers)
                    m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
                break;
            case FaceRenderOrder::IndirectOrder:
                m_functions.glCullFace(GL_FRONT);
                for (auto ibo : mesh->indexBuffers)
                    m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
                m_functions.glCullFace(GL_BACK);
                for (auto ibo : mesh->indexBuffers)
                    m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
                break;
            default:
                break;
            }
            m_functions.glBindVertexArray(0);
        }

        drawable->postrender();
    }
}

GLbitfield Renderer::calcClearMask() const
{
    GLbitfield clearMask = 0;
    if (m_clearColorBit) clearMask |= GL_COLOR_BUFFER_BIT;
    if (m_clearDepthBit) clearMask |= GL_DEPTH_BUFFER_BIT;

    return clearMask;
}

std::string Renderer::precompileShader(const QString &dir, QByteArray &text)
{
    static const QString includeString = "#include<";
    static const QString closedBracket = ">";

    std::set<QByteArray> includedFiles;

    for (auto pos = text.indexOf(includeString); pos != -1; pos = text.indexOf(includeString, pos))
    {
        auto pos2 = text.indexOf(closedBracket, pos);
        if (pos2 == -1)
        {
            return "Wrong order '<' and '>' in '#include'";
        }

        auto pos1 = pos + includeString.length();
        auto includedFilename = text.mid(pos1, pos2-pos1);

        if (includedFiles.count(includedFilename))
        {
            text = text.replace(pos, pos2-pos+1, "");
            continue;
        }

        QFile includedfile(dir + includedFilename);
        if (!includedfile.open(QFile::ReadOnly))
        {
            return "Can't open included file \"" + includedFilename.toStdString() + "\"";
        }

        auto includedData = includedfile.readAll();
        includedfile.close();

        text = text.replace(pos, pos2-pos+1, includedData);
        includedFiles.insert(includedFilename);
    }

    return "";
}

} // namespace
} // namespace
