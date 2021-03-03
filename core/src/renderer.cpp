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
#include "utils.h"
#include "model.inl"
#include "texture.inl"

#include <iostream>

namespace trash
{
namespace core
{

AbstractUniform::~AbstractUniform()
{
}

RenderProgram::RenderProgram(GLuint id_)
    : id(id_)
{
    auto& functions = Renderer::instance().functions();

    GLint numActiveUniforms, uniformMaxLength;
    functions.glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
    functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLength);

    GLint numActiveUniformBlocks, uniformBlockMaxLength;
    functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks);
    functions.glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniformBlockMaxLength);

    char *name = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(glm::max(uniformMaxLength, uniformBlockMaxLength) + 1)));
    GLint size;
    GLenum type;

    for (size_t i = 0; i < numActiveUniforms; ++i)
    {
        functions.glGetActiveUniform(id, i, uniformMaxLength, nullptr, &size, &type, name);
        auto uniformId = uniformIdByName(name);
        if (uniformId == UniformId::Undefined)
            continue;
        GLint loc = functions.glGetUniformLocation(id, name);
        if (loc == -1)
            continue;
        uniforms.insert({uniformId, loc});
    }

    for (size_t i = 0; i < numActiveUniformBlocks; ++i)
    {
        functions.glGetActiveUniformBlockName(id, i, uniformBlockMaxLength, nullptr, name);
        auto uniformId = uniformIdByName(name);
        if (uniformId == UniformId::Undefined)
            continue;
        uniforms.insert({uniformId, static_cast<GLint>(i)});
    }

    free(name);
}

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

UniformId RenderProgram::uniformIdByName(const std::string& name)
{
    static const std::map<std::string, UniformId> s_names {
        { "u_id", UniformId::NodeId },
        { "u_modelMatrix", UniformId::ModelMatrix },
        { "u_normalMatrix", UniformId::NormalMatrix },
        { "u_viewMatrix", UniformId::ViewMatrix },
        { "u_viewMatrixInverse", UniformId::ViewMatrixInverse },
        { "u_projMatrix", UniformId::ProjMatrix },
        { "u_viewProjMatrix", UniformId::ViewProjMatrix },
        { "u_viewProjMatrixInverse", UniformId::ViewProjMatrixInverse },
        { "u_modelViewMatrix", UniformId::ModelViewMatrix },
        { "u_normalViewMatrix", UniformId::NormalViewMatrix },
        { "u_modelViewProjMatrix", UniformId::ModelViewProjMatrix },
        { "u_viewPosition", UniformId::ViewPosition },
        { "u_viewportSize", UniformId::ViewportSize },
        { "u_diffuseIBLMap", UniformId::IBLDiffuseMap },
        { "u_specularIBLMap", UniformId::IBLSpecularMap },
        { "u_maxSpecularIBLMapMipmapLevel", UniformId::IBLSpecularMapMaxMipmapLevel },
        { "u_brdfLUT", UniformId::BrdfLutMap },
        { "u_IBLContribution", UniformId::IBLContribution },
        { "u_shadowMaps", UniformId::ShadowMaps },
        { "u_bonesBuffer", UniformId::BonesBuffer },
        { "u_lightsBuffer", UniformId::LightsBuffer },
        { "u_color", UniformId::Color },
        { "u_metallicRoughness", UniformId::MetallicRoughness },
        { "u_baseColorMap", UniformId::BaseColorMap },
        { "u_opacityMap", UniformId::OpacityMap },
        { "u_normalMap", UniformId::NormalMap },
        { "u_metallicMap", UniformId::MetallicMap },
        { "u_roughnessMap", UniformId::RoughnessMap },
        { "u_lightIndices[0]", UniformId::LightIndicesList },
        { "u_gBufferMap0", UniformId::GBufferMap0 },
        { "u_gBufferMap1", UniformId::GBufferMap1 },
        { "u_gBufferMap2", UniformId::GBufferMap2 },
        { "u_hdrMap", UniformId::HDRMap },
    };

    auto it = s_names.find(name);
    return (it == s_names.end()) ? UniformId::Undefined : it->second;
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

Texture::~Texture()
{
    Renderer::instance().functions().glDeleteTextures(1, &id);
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

int32_t Texture::maxMipmapLevel() const
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
    detachDepthStencil();

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

void Framebuffer::detachDepthStencil()
{
    if (!depthStencilAttachment)
        return;

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (depthStencilAttachment->isTexture())
    {
        if (depthStencilAttachment->texture->target == GL_TEXTURE_2D || depthStencilAttachment->texture->target == GL_TEXTURE_CUBE_MAP)
            functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
        else if (depthStencilAttachment->texture->target == GL_TEXTURE_2D_ARRAY)
            functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 0, 0, 0);
    }
    else if (depthStencilAttachment->isRenderbuffer())
    {
        functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    }

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());

    depthStencilAttachment = nullptr;
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
    detachDepthStencil();

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
    detachDepthStencil();

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer->id);
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    depthAttachment = std::make_shared<RenderTarget>(renderbuffer);
}

void Framebuffer::attachDepthStencil(std::shared_ptr<Texture> texture, uint32_t layer)
{
    detachDepth();
    detachDepthStencil();

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();
    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);

    if (texture->target == GL_TEXTURE_2D)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->id, 0);
    else if (texture->target == GL_TEXTURE_CUBE_MAP)
        functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer, texture->id, 0);
    else if (texture->target == GL_TEXTURE_2D_ARRAY)
        functions.glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture->id, 0, static_cast<GLint>(layer));

    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    depthStencilAttachment = std::make_shared<RenderTarget>(texture);
}

void Framebuffer::attachDepthStencil(std::shared_ptr<Renderbuffer> renderbuffer)
{
    detachDepth();
    detachDepthStencil();

    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer->id);
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
    depthStencilAttachment = std::make_shared<RenderTarget>(renderbuffer);
}

void Framebuffer::drawBuffers(const std::vector<GLenum>& buffers)
{
    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glDrawBuffers(buffers.size(), buffers.data());
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());
}

bool Framebuffer::isComplete() const
{
    auto& renderer = Renderer::instance();
    auto& functions = renderer.functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    auto res = functions.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    functions.glBindFramebuffer(GL_FRAMEBUFFER, renderer.defaultFbo());

    return res = GL_FRAMEBUFFER_COMPLETE;
}

Renderer::Renderer(QOpenGLExtraFunctions& functions, GLuint defaultFbo)
    : m_functions(functions)
    , m_defaultFbo(defaultFbo)
    , m_resourceStorage(std::make_unique<ResourceStorage>())
    , m_drawData()
{
}

void Renderer::initializeResources()
{
    m_functions.glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_hdrFramebuffer = std::make_shared<Framebuffer>();
    m_hdrFramebufferSize = glm::uvec2(0u, 0u);

    m_gFramebuffer = std::make_shared<Framebuffer>();
    m_gFramebufferSize = glm::uvec2(0u, 0u);

//    const float L = 2500;
//    const float wrap = 14;
//    std::vector<glm::vec3> pos {glm::vec3(-L, 0.0f, -L), glm::vec3(-L, 0.0f, +L), glm::vec3(+L, 0.0f, -L), glm::vec3(+L, 0.0f, +L)};
//    std::vector<glm::vec3> n {glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f)};
//    std::vector<glm::vec2> tc {glm::vec2(0.f, 0.f), glm::vec2(0.f, wrap), glm::vec2(wrap, 0.f), glm::vec2(wrap, wrap)};
//    std::vector<uint32_t> indices {0, 1, 2, 1, 3, 2};

//    auto mesh = std::make_shared<Mesh>();
//    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(4, 3, &pos[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::Normal, std::make_shared<VertexBuffer>(4, 3, &n[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::TexCoord, std::make_shared<VertexBuffer>(4, 2, &tc[0].x, GL_STATIC_DRAW));
//    mesh->attachIndexBuffer(std::make_shared<IndexBuffer>(GL_TRIANGLES, 6, indices.data(), GL_STATIC_DRAW));

//    auto mat = std::make_shared<Model::Material>();
//    mat->baseColorMap = std::make_pair(std::string("textures/floor.jpg"), loadTexture("textures/floor.jpg"));

//    auto mdl = std::make_shared<Model>();
//    mdl->rootNode = std::make_shared<Model::Node>();
//    mdl->rootNode->meshes.push_back(std::make_shared<Model::Mesh>(mesh, mat));

//    std::ofstream f("floor2.mdl", std::ios_base::binary);
//    push(f, mdl);
//    f.close();

//    std::vector<std::string> names {"liam2", "stefani2", "shae2", "malcolm2", "regina2"};
//    for (auto& n : names)
//    {
//        auto mdl = loadModel(n+".mdl");
//        mdl->animations.clear();
//        std::ofstream file(n+"2.mdl", std::ios_base::binary);
//        push(file, mdl);
//        file.close();
//    }



//    auto model = loadModel("teeth.dae");
//    model->rootNode->children().at(1)->meshes.at(0)->material->baseColorMap.first = "textures/teeth_basecolor.png";
//    model->rootNode->children().at(1)->meshes.at(0)->material->normalMap.first = "textures/teeth_normal.png";
//    model->rootNode->children().at(1)->meshes.at(0)->material->metallicMap.first = "textures/teeth_metallic.png";
//    model->rootNode->children().at(1)->meshes.at(0)->material->roughnessMap.first = "textures/teeth_roughness.png";

//    model->rootNode->children().at(2)->meshes.at(0)->material->baseColorMap.first = "textures/teeth_basecolor.png";
//    model->rootNode->children().at(2)->meshes.at(0)->material->normalMap.first = "textures/teeth_normal.png";
//    model->rootNode->children().at(2)->meshes.at(0)->material->metallicMap.first = "textures/teeth_metallic.png";
//    model->rootNode->children().at(2)->meshes.at(0)->material->roughnessMap.first = "textures/teeth_roughness.png";

//    std::ofstream f("teeth.mdl", std::ios_base::binary);
//    push(f, model);
//    f.close();

//    QImage diffuse("Mrm_Albedo_LP_merged.1001.png");
//    QImage specular("Mrm_Specular_LP_merged.1001.png");
//    QImage glossiness("Mrm_Gloss_LP_merged.1001.png");
//    QImage baseColor, metallic, roughness;

//    diffuseSpecularGlossinessToBaseColorMetallicRoughness(diffuse, specular, glossiness, QImage(), baseColor, metallic, roughness);

//    baseColor.save("teeth_basecolor.png");
//    metallic.save("teeth_metallic.png");
//    roughness.save("teeth_roughness.png");

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

std::shared_ptr<RenderProgram> Renderer::loadRenderProgram(const std::string &vertexFile, const std::string &fragmentFile, const std::set<std::string>& defines)
{
    std::string key = vertexFile+fragmentFile;
    for (const auto& define : defines)
        key += define;

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
            auto errorString = precompileShader(QString::fromStdString(dir), byteArray, defines);
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

void Renderer::draw(std::shared_ptr<Drawable> drawable, const utils::Transform& transform, uint32_t id)
{
    m_drawData[castFromLayerId(drawable->layerId())].push_back(std::tuple<std::shared_ptr<Drawable>, utils::Transform, uint32_t>(drawable, transform, id));
}

void Renderer::clear()
{
    for (auto& layer: m_drawData)
        layer.clear();
}

void Renderer::renderDeffered(const RenderInfo& renderInfo)
{
    if (renderInfo.viewportSize() != m_gFramebufferSize)
    {
        m_gFramebufferSize = renderInfo.viewportSize();
        resizeGFramebuffer(m_gFramebufferSize);
    }

    if (renderInfo.viewportSize() != m_hdrFramebufferSize)
    {
        m_hdrFramebufferSize = renderInfo.viewportSize();
        resizeHDRFramebuffer(m_hdrFramebufferSize);
    }

    m_functions.glViewport(0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y);

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_gFramebuffer->id);

    static const GLfloat gcolor[4] = {0.5f, 0.5f, 0.5f, 0.0f};
    static const GLfloat gdepth[1] = {1.0f};
    static const GLint gstencil[1] = {0};
    m_functions.glClearBufferfv(GL_COLOR, 0, gcolor);
    m_functions.glClearBufferfv(GL_COLOR, 1, gcolor);
    m_functions.glClearBufferfv(GL_DEPTH, 0, gdepth);
    m_functions.glClearBufferiv(GL_STENCIL, 0, gstencil);

    m_functions.glDisable(GL_BLEND);
    m_functions.glEnable(GL_STENCIL_TEST);
    m_functions.glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    m_functions.glStencilFunc(GL_ALWAYS, 1, 0xFF);
    m_functions.glStencilMask(0xFF);
    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);

    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::OpaqueGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::DeferredGeometryPass, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrFramebuffer->id);
    m_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gFramebuffer->id);
    m_functions.glBlitFramebuffer(0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y,
                                  0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    m_functions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    m_functions.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFramebuffer->id);

    static const GLfloat color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_functions.glClearBufferfv(GL_COLOR, 0, color);

    m_functions.glDisable(GL_BLEND);
    m_functions.glEnable(GL_STENCIL_TEST);
    m_functions.glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    m_functions.glStencilMask(0x00);
    m_functions.glDisable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);

    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::Background)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glEnable(GL_STENCIL_TEST);
    m_functions.glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    m_functions.glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
    m_functions.glStencilMask(0xFF);
    m_functions.glDepthMask(GL_FALSE);
    m_functions.glBlendFunc(GL_ONE, GL_ONE);
    m_functions.glCullFace(GL_FRONT);

    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::Lights)])
    {
        auto mesh = std::get<0>(drawData)->mesh();

        m_functions.glDisable(GL_BLEND);
        m_functions.glEnable(GL_DEPTH_TEST);
        m_functions.glDisable(GL_CULL_FACE);
        m_functions.glStencilFunc(GL_ALWAYS, 0, 0xFF);

        m_functions.glClear(GL_STENCIL_BUFFER_BIT);

        static GLenum none[] = {GL_NONE};
        m_functions.glDrawBuffers(1, none);

        setupUniforms(drawData, DrawableRenderProgramId::DeferredStencilPass, renderInfo);
        renderMesh(mesh);

        m_functions.glEnable(GL_BLEND);
        m_functions.glDisable(GL_DEPTH_TEST);
        m_functions.glEnable(GL_CULL_FACE);
        m_functions.glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

        static GLenum colorAttachment[] = {GL_COLOR_ATTACHMENT0};
        m_functions.glDrawBuffers(1, colorAttachment);

        setupUniforms(drawData, DrawableRenderProgramId::DeferredLightPass, renderInfo);
        renderMesh(mesh);
    }

    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glDisable(GL_BLEND);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::NotLightedGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    DrawDataLayerContainer& transparentLayer = m_drawData[castFromLayerId(LayerId::TransparentGeometry)];
    std::sort(transparentLayer.begin(), transparentLayer.end(), [&renderInfo](const DrawDataLayerContainer::value_type& first, const DrawDataLayerContainer::value_type& second) -> bool
    {
        const glm::vec3 v1 = std::get<1>(first).translation - renderInfo.viewPosition();
        const glm::vec3 v2 = std::get<1>(second).translation - renderInfo.viewPosition();
        return glm::dot(v1,v1) > glm::dot(v2,v2);
    });

    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_BLEND);
    m_functions.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::TransparentGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        m_functions.glCullFace(GL_FRONT);
        renderMesh(std::get<0>(drawData)->mesh());
        m_functions.glCullFace(GL_BACK);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFbo);

    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glDisable(GL_DEPTH_TEST);
    m_functions.glDisable(GL_BLEND);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::PostEffect)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::PostEffect, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glBindVertexArray(0);
}

void Renderer::renderForward(const RenderInfo& renderInfo)
{
    if (renderInfo.viewportSize() != m_hdrFramebufferSize)
    {
        m_hdrFramebufferSize = renderInfo.viewportSize();
        resizeHDRFramebuffer(m_hdrFramebufferSize);
    }

    m_functions.glViewport(0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y);

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFramebuffer->id);

    static const GLfloat color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    static const GLfloat depth[1] = {1.0f};

    m_functions.glClearBufferfv(GL_COLOR, 0, color);
    m_functions.glClearBufferfv(GL_DEPTH, 0, depth);

    m_functions.glDisable(GL_BLEND);
    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glDisable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);

    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::Background)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glEnable(GL_DEPTH_TEST);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::OpaqueGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::NotLightedGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    DrawDataLayerContainer& transparentLayer = m_drawData[castFromLayerId(LayerId::TransparentGeometry)];
    std::sort(transparentLayer.begin(), transparentLayer.end(), [&renderInfo](const DrawDataLayerContainer::value_type& first, const DrawDataLayerContainer::value_type& second) -> bool
    {
        const glm::vec3 v1 = std::get<1>(first).translation - renderInfo.viewPosition();
        const glm::vec3 v2 = std::get<1>(second).translation - renderInfo.viewPosition();
        return glm::dot(v1,v1) > glm::dot(v2,v2);
    });

    m_functions.glEnable(GL_BLEND);
    m_functions.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::TransparentGeometry)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::ForwardRender, renderInfo);
        m_functions.glCullFace(GL_FRONT);
        renderMesh(std::get<0>(drawData)->mesh());
        m_functions.glCullFace(GL_BACK);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFbo);

    m_functions.glDisable(GL_DEPTH_TEST);
    m_functions.glDisable(GL_BLEND);
    for (const auto& drawData : m_drawData[castFromLayerId(LayerId::PostEffect)])
    {
        setupUniforms(drawData, DrawableRenderProgramId::PostEffect, renderInfo);
        renderMesh(std::get<0>(drawData)->mesh());
    }

    m_functions.glBindVertexArray(0);
}

void Renderer::renderShadows(const RenderInfo& renderInfo, std::shared_ptr<Framebuffer> framebuffer)
{
    GLuint framebufferId = framebuffer ? framebuffer->id : m_defaultFbo;
    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    m_functions.glViewport(0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y);

    static const GLfloat depth[1] = {1.0f};
    m_functions.glClearBufferfv(GL_DEPTH, 0, depth);

    m_functions.glDisable(GL_BLEND);
    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_FRONT);

    for (auto layer : {LayerId::OpaqueGeometry, LayerId::TransparentGeometry})
        for (const auto& drawData : m_drawData.at(castFromLayerId(layer)))
        {
            setupUniforms(drawData, DrawableRenderProgramId::Shadow, renderInfo);
            renderMesh(std::get<0>(drawData)->mesh());
        }

    m_functions.glBindVertexArray(0);
}

void Renderer::renderIds(const RenderInfo& renderInfo, std::shared_ptr<Framebuffer> framebuffer)
{
    GLuint framebufferId = framebuffer ? framebuffer->id : m_defaultFbo;
    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    m_functions.glViewport(0, 0, renderInfo.viewportSize().x, renderInfo.viewportSize().y);

    static const GLuint color[4] = {0u, 0u, 0u, 0u};
    static const GLfloat depth[1] = {1.0f};

    m_functions.glClearBufferuiv(GL_COLOR, 0, color);
    m_functions.glClearBufferfv(GL_DEPTH, 0, depth);

    m_functions.glDisable(GL_BLEND);
    m_functions.glDisable(GL_STENCIL_TEST);
    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glDepthMask(GL_TRUE);
    m_functions.glEnable(GL_CULL_FACE);
    m_functions.glCullFace(GL_BACK);

    for (auto layer : {LayerId::OpaqueGeometry, LayerId::TransparentGeometry})
    {
        for (const auto& drawData : m_drawData.at(castFromLayerId(layer)))
        {
            setupUniforms(drawData, DrawableRenderProgramId::Selection, renderInfo);
            renderMesh(std::get<0>(drawData)->mesh());
        }
    }

    m_functions.glBindVertexArray(0);
}

void Renderer::readPixel(const RenderInfo& renderInfo, std::shared_ptr<Framebuffer> framebuffer, GLenum attachment, int xi, int yi, GLenum format, GLenum type, GLvoid *data) const
{
    GLuint framebufferId = framebuffer ? framebuffer->id : m_defaultFbo;
    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);

    m_functions.glReadBuffer(attachment);
    m_functions.glReadPixels(xi, renderInfo.viewportSize().y - yi - 1, 1, 1, format, type, data);
}

void Renderer::setupUniforms(const DrawDataType& data, DrawableRenderProgramId programId, const RenderInfo& renderInfo)
{
    std::shared_ptr<Drawable> drawable = std::get<0>(data);
    const utils::Transform& modelTransform = std::get<1>(data);
    glm::mat4x4 modelMatrix = modelTransform.operator glm::mat4x4();
    uint32_t id = std::get<2>(data);

    auto renderProgram = drawable->renderProgram(programId);
    m_functions.glUseProgram(renderProgram->id);

    for (const auto& uniform : renderProgram->uniforms)
    {
        switch (uniform.first)
        {
        case UniformId::NodeId:
        {
            m_functions.glUniform1ui(uniform.second, id);
            break;
        }
        case UniformId::ModelMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            break;
        }
        case UniformId::NormalMatrix:
        {
            m_functions.glUniformMatrix3fv(uniform.second, 1, GL_FALSE, glm::value_ptr(glm::mat3x3(glm::inverseTranspose(modelMatrix))));
            break;
        }
        case UniformId::ViewMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewMatrix()));
            break;
        }
        case UniformId::ViewMatrixInverse:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewMatrixInverse()));
            break;
        }
        case UniformId::ProjMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.projMatrix()));
            break;
        }
        case UniformId::ViewProjMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewProjMatrix()));
            break;
        }
        case UniformId::ViewProjMatrixInverse:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewProjMatrixInverse()));
            break;
        }
        case UniformId::ModelViewMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewMatrix() * modelMatrix));
            break;
        }
        case UniformId::NormalViewMatrix:
        {
            m_functions.glUniformMatrix3fv(uniform.second, 1, GL_FALSE, glm::value_ptr(glm::mat3x3(glm::inverseTranspose(renderInfo.viewMatrix() * modelMatrix))));
            break;
        }
        case UniformId::ModelViewProjMatrix:
        {
            m_functions.glUniformMatrix4fv(uniform.second, 1, GL_FALSE, glm::value_ptr(renderInfo.viewProjMatrix() * modelMatrix));
            break;
        }
        case UniformId::ViewPosition:
        {
            m_functions.glUniform3fv(uniform.second, 1, glm::value_ptr(renderInfo.viewPosition()));
            break;
        }
        case UniformId::ViewportSize:
        {
            m_functions.glUniform2uiv(uniform.second, 1, glm::value_ptr(renderInfo.viewportSize()));
            break;
        }
        case UniformId::IBLDiffuseMap:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::DiffuseIBL));
            bindTexture(renderInfo.IBLDiffuseMap(), castFromTextureUnit(TextureUnit::DiffuseIBL));
            break;
        }
        case UniformId::IBLSpecularMap:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::SpecularIBL));
            bindTexture(renderInfo.IBLSpecularMap(), castFromTextureUnit(TextureUnit::SpecularIBL));
            break;
        }
        case UniformId::IBLSpecularMapMaxMipmapLevel:
        {
            m_functions.glUniform1i(uniform.second, renderInfo.maxIBLSpecularMapMipmapLevel());
            break;
        }
        case UniformId::BrdfLutMap:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::BrdfLUT));
            bindTexture(renderInfo.brdfLutMap(), castFromTextureUnit(TextureUnit::BrdfLUT));
            break;
        }
        case UniformId::IBLContribution:
        {
            m_functions.glUniform1f(uniform.second, renderInfo.IBLContribution());
            break;
        }
        case UniformId::ShadowMaps:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::ShadowMaps));
            bindTexture(renderInfo.shadowMaps(), castFromTextureUnit(TextureUnit::ShadowMaps));
            break;
        }
        case UniformId::BonesBuffer:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Buffer>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniformBlockBinding(renderProgram->id, uniform.second, castFromUniformBufferUnit(UniformBufferUnit::Bones));
                bindUniformBuffer(uniformValue->get(), castFromUniformBufferUnit(UniformBufferUnit::Bones));
            }
            break;
        }
        case UniformId::LightsBuffer:
        {
            m_functions.glUniformBlockBinding(renderProgram->id, uniform.second, castFromUniformBufferUnit(UniformBufferUnit::Lights));
            bindUniformBuffer(renderInfo.lightsBuffer(), castFromUniformBufferUnit(UniformBufferUnit::Lights));
            break;
        }
        case UniformId::Color:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<glm::vec4>>(drawable->uniform(uniform.first));
            if (uniformValue)
                m_functions.glUniform4fv(uniform.second, 1, glm::value_ptr(uniformValue->get()));
            break;
        }
        case UniformId::MetallicRoughness:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<glm::vec2>>(drawable->uniform(uniform.first));
            if (uniformValue)
                m_functions.glUniform2fv(uniform.second, 1, glm::value_ptr(uniformValue->get()));
            break;
        }
        case UniformId::BaseColorMap:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Texture>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::BaseColor));
                bindTexture(uniformValue->get(), castFromTextureUnit(TextureUnit::BaseColor));
            }
            break;
        }
        case UniformId::OpacityMap:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Texture>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::Opacity));
                bindTexture(uniformValue->get(), castFromTextureUnit(TextureUnit::Opacity));
            }
            break;
        }
        case UniformId::NormalMap:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Texture>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::Normal));
                bindTexture(uniformValue->get(), castFromTextureUnit(TextureUnit::Normal));
            }
            break;
        }
        case UniformId::MetallicMap:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Texture>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::Metallic));
                bindTexture(uniformValue->get(), castFromTextureUnit(TextureUnit::Metallic));
            }
            break;
        }
        case UniformId::RoughnessMap:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<Texture>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::Roughness));
                bindTexture(uniformValue->get(), castFromTextureUnit(TextureUnit::Roughness));
            }
            break;
        }
        case UniformId::LightIndicesList:
        {
            auto uniformValue = std::dynamic_pointer_cast<Uniform<std::shared_ptr<LightIndicesList>>>(drawable->uniform(uniform.first));
            if (uniformValue)
            {
                std::shared_ptr<LightIndicesList> lightIndices = uniformValue->get();
                for (size_t i = 0; i < lightIndices->size(); ++i)
                    m_functions.glUniform1i(uniform.second + i, lightIndices->at(i));
            }
            break;
        }
        case UniformId::GBufferMap0:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::GBuffer0));
            bindTexture(m_gFramebuffer->depthStencilAttachment->texture, castFromTextureUnit(TextureUnit::GBuffer0));
            break;
        }
        case UniformId::GBufferMap1:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::GBuffer1));
            bindTexture(m_gFramebuffer->colorAttachments[0]->texture, castFromTextureUnit(TextureUnit::GBuffer1));
            break;
        }
        case UniformId::GBufferMap2:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::GBuffer2));
            bindTexture(m_gFramebuffer->colorAttachments[1]->texture, castFromTextureUnit(TextureUnit::GBuffer2));
            break;
        }
        case UniformId::HDRMap:
        {
            m_functions.glUniform1i(uniform.second, castFromTextureUnit(TextureUnit::BaseColor));
            bindTexture(m_hdrFramebuffer->colorAttachments[0]->texture, castFromTextureUnit(TextureUnit::BaseColor));
            break;
        }
        }
    }
}

void Renderer::renderMesh(std::shared_ptr<Mesh> mesh)
{
    m_functions.glBindVertexArray(mesh->id);
    for (auto ibo : mesh->indexBuffers)
        m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
}

void Renderer::resizeHDRFramebuffer(const glm::uvec2& sz)
{
    m_hdrFramebuffer = std::make_shared<Framebuffer>();
    m_hdrFramebuffer->attachDepthStencil(std::make_shared<Renderbuffer>(GL_DEPTH24_STENCIL8, sz.x, sz.y));
    m_hdrFramebuffer->attachColor(0, createTexture2D(GL_RGBA16F, sz.x, sz.y, 0, 0, nullptr, false));
}

void Renderer::resizeGFramebuffer(const glm::uvec2& sz)
{
    m_gFramebuffer = std::make_shared<Framebuffer>();
    m_gFramebuffer->attachDepthStencil(createTexture2D(GL_DEPTH24_STENCIL8, sz.x, sz.y, 0, 0, nullptr, false));
    m_gFramebuffer->attachColor(0, createTexture2D(GL_RGBA8, sz.x, sz.y, 0, 0, nullptr, false));
    m_gFramebuffer->attachColor(1, createTexture2D(GL_RGB10_A2, sz.x, sz.y, 0, 0, nullptr, false));
    m_gFramebuffer->drawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
}

std::string Renderer::precompileShader(const QString &dir, QByteArray &text, const std::set<std::string>& defines)
{
    static const QString versionString = "#version 330 core";
    static const QString includeString = "#include<";
    static const QString closedBracket = ">";

    // add included files
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

    // add defines
    for (const auto& define : defines)
        text.prepend(QByteArray::fromStdString("#define " + define + "\n"));

    // add version string
    text.prepend((versionString + "\n").toUtf8());


    return "";
}

RenderInfo::RenderInfo(const glm::mat4x4& vm, const glm::mat4x4& pm, const glm::uvec2& vs)
    : m_viewMatrix(vm)
    , m_projMatrix(pm)
    , m_viewportSize(vs)
    , m_lightsBuffer(nullptr)
    , m_shadowMaps(nullptr)
    , m_IBLDiffuseMap(nullptr)
    , m_IBLSpecularMap(nullptr)
    , m_maxIBLSpecularMapMipmapLevel(0)
    , m_IBLContribution(0.2f)
{
    m_viewMatrixInverse = glm::inverse(m_viewMatrix);
    m_viewProjMatrix = m_projMatrix * m_viewMatrix;
    m_viewProjMatrixInverse = glm::inverse(m_viewProjMatrix);
    m_viewPosition = glm::vec3(m_viewMatrixInverse * glm::vec4(0.f, 0.f, 0.f, 1.f));
}

} // namespace
} // namespace
