#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <core/core.h>

#include "coreprivate.h"
#include "renderwidget.h"
#include "renderer.h"
#include "resourcestorage.h"
#include "drawables.h"
#include "resources.h"
#include "model.inl"
#include "importexport.h"

#include <iostream>

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

    functions.glBindTexture(GL_TEXTURE_2D, id);
    functions.glGenerateMipmap(GL_TEXTURE_2D);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Buffer::Buffer(GLsizeiptr size, GLvoid *data, GLenum usage)
    : id(0)
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
}

void *Buffer::map(GLintptr offset, GLsizeiptr size, GLbitfield access)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBuffer(GL_ARRAY_BUFFER, id);
    return functions.glMapBufferRange(GL_ARRAY_BUFFER, offset, size, access);
}

void Buffer::unmap()
{
    Renderer::instance().functions().glUnmapBuffer(GL_ARRAY_BUFFER);
}

VertexBuffer::VertexBuffer(uint32_t nv, uint32_t nc, float *data, GLenum usage)
    : Buffer(static_cast<GLsizeiptr>(nv*nc*sizeof(float)), data, usage)
    , numVertices(nv)
    , numComponents(nc)
{
}

IndexBuffer::IndexBuffer(GLenum primitiveType_, uint32_t numIndices_, uint32_t *data, GLenum usage)
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
        if (vb->numComponents == 2)
            boundingSphere = BoundingSphere(static_cast<glm::vec2*>(p), vb->numVertices);
        if (vb->numComponents == 3)
            boundingSphere = BoundingSphere(static_cast<glm::vec3*>(p), vb->numVertices);
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

Framebuffer::Framebuffer(GLint internalFormat)
    : colorTexture(0)
    , depthTexture(0)
    , colorTextureInternalFormat(internalFormat)
{
    auto& functions = Renderer::instance().functions();

    functions.glGenFramebuffers(1, &id);
}

Framebuffer::~Framebuffer()
{
    auto& functions = Renderer::instance().functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    QOpenGLFramebufferObject::bindDefault();

    if (depthTexture)
        functions.glDeleteTextures(1, &depthTexture);

    if (colorTexture)
        functions.glDeleteTextures(1, &colorTexture);

    functions.glDeleteFramebuffers(1, &id);
}

void Framebuffer::resize(int width, int height)
{
    auto& functions = Renderer::instance().functions();

    functions.glBindFramebuffer(GL_FRAMEBUFFER, id);
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

    if (depthTexture)
        functions.glDeleteTextures(1, &depthTexture);

    if (colorTexture)
        functions.glDeleteTextures(1, &colorTexture);

    functions.glGenTextures(1, &depthTexture);
    functions.glBindTexture(GL_TEXTURE_2D, depthTexture);
    functions.glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    functions.glBindTexture(GL_TEXTURE_2D, 0);

    functions.glGenTextures(1, &colorTexture);
    functions.glBindTexture(GL_TEXTURE_2D, colorTexture);
    functions.glTexImage2D(GL_TEXTURE_2D, 0, colorTextureInternalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    functions.glBindTexture(GL_TEXTURE_2D, 0);

    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    functions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    QOpenGLFramebufferObject::bindDefault();
}

Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
    , m_resourceStorage(std::make_unique<ResourceStorage>())
    , m_projMatrix(1.0f)
    , m_viewMatrix(1.0f)
{
}

void Renderer::initializeResources()
{
    m_selectionFramebuffer = std::make_shared<Framebuffer>(GL_RGBA8);

    auto standardTexture = loadTexture(standardDiffuseTextureName);
    standardTexture->generateMipmaps();

//    const float L = 1250;
//    const float wrap = 7;
//    std::vector<glm::vec3> pos {glm::vec3(-L, 0.0f, -L), glm::vec3(-L, 0.0f, +L), glm::vec3(+L, 0.0f, -L), glm::vec3(+L, 0.0f, +L)};
//    std::vector<glm::vec3> n {glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f), glm::vec3(0.f, 1.0f, 0.f)};
//    std::vector<glm::vec2> tc {glm::vec2(0.f, 0.f), glm::vec2(0.f, wrap), glm::vec2(wrap, 0.f), glm::vec2(wrap, wrap)};
//    std::vector<uint32_t> indices {0, 1, 2, 3};

//    auto mesh = std::make_shared<Mesh>();
//    mesh->declareVertexAttribute(VertexAttribute::Position, std::make_shared<VertexBuffer>(4, 3, &pos[0].x, GL_STATIC_DRAW));
//    mesh->declareVertexAttribute(VertexAttribute::Normal, std::make_shared<VertexBuffer>(4, 3, &n[0].x, GL_STATIC_DRAW));
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

    std::vector<std::string> names {"liam", "stefani", "shae", "malcolm", "regina"};
    for (auto& n : names)
    {
        auto mdl = loadModel(n+".dae");
        std::ofstream file(n+".mdl", std::ios_base::binary);
        push(file, mdl);
        file.close();
    }
}

Renderer &Renderer::instance()
{
    return Core::instance().m().renderWidget->renderer();
}

QOpenGLExtraFunctions& Renderer::functions()
{
    return m_functions;
}

std::shared_ptr<RenderProgram> Renderer::loadRenderProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
    const std::string key = vertexFile+fragmentFile;
    auto object = std::dynamic_pointer_cast<RenderProgram>(m_resourceStorage->get(key));
    if (!object)
    {
        std::array<std::pair<GLenum, QString>, 2> shaderFilenames {
            std::make_pair(GL_VERTEX_SHADER, QString::fromStdString(vertexFile)),
            std::make_pair(GL_FRAGMENT_SHADER, QString::fromStdString(fragmentFile))
        };

        GLuint shaderIds[2];
        bool isOk = true;
        for (size_t i = 0; i < 2; ++i)
        {
            auto& shader = shaderFilenames[i];
            QFile file(shader.second);

            if (!file.open(QFile::ReadOnly))
            {
                isOk = false;
                continue;
            }

            auto byteArray = file.readAll();
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
                    std::cout << shader.second.toStdString() << ": " << infoLog << std::endl;
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

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filename)
{
    auto object = std::dynamic_pointer_cast<Texture>(m_resourceStorage->get(filename));
    if (!object)
    {
        QImage image;
        if (!image.load(QString::fromStdString(filename)))
            return nullptr;
        image = image.convertToFormat(QImage::Format_RGBA8888);

        GLuint id;
        m_functions.glGenTextures(1, &id);
        m_functions.glBindTexture(GL_TEXTURE_2D, id);
        m_functions.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        object = std::make_shared<Texture>(id);
        object->generateMipmaps();
        m_resourceStorage->store(filename, object);
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

void Renderer::bindTexture(std::shared_ptr<Texture> texture, GLint unit)
{
    m_functions.glActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
    m_functions.glBindTexture(texture->target, texture->id);
}

void Renderer::bindUniformBuffer(std::shared_ptr<Buffer> buffer, GLuint unit)
{
    GLuint id = buffer ? buffer->id : 0;
    m_functions.glBindBufferBase(GL_UNIFORM_BUFFER, unit, id);
}

void Renderer::draw(std::shared_ptr<Drawable> drawable, const Transform& transform)
{
    m_drawData.insert(std::make_pair(drawable, transform));
}

void Renderer::pick(int xi, int yi, const glm::vec4& backgroundColor, uint32_t& pickColor, float& pickDepth)
{
    static const GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};

    m_functions.glBindFramebuffer(GL_FRAMEBUFFER, m_selectionFramebuffer->id);
    m_functions.glDrawBuffers(1, drawBuffers);

    m_functions.glEnable(GL_DEPTH_TEST);
    m_functions.glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    m_functions.glClearDepthf(1.0f);
    m_functions.glViewport(0, 0, m_viewport.z, m_viewport.w);

    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto begin = m_drawData.lower_bound(LayerId::Selection);
    auto end = m_drawData.upper_bound(*begin);

    for (; begin != end; ++begin)
    {
        auto drawable = begin->first;
        const auto& transform = begin->second;

        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        renderProgram->setUniform(renderProgram->uniformLocation("u_projMatrix"), m_projMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_viewMatrix"), m_viewMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_modelMatrix"), transform.operator glm::mat4x4());

        drawable->prerender();

        auto mesh = drawable->mesh();
        if (mesh)
        {
            m_functions.glBindVertexArray(mesh->id);
            for (auto ibo : mesh->indexBuffers)
                m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
            m_functions.glBindVertexArray(0);
        }

        drawable->postrender();
    }

    uint8_t color[4];
    m_functions.glReadBuffer(GL_COLOR_ATTACHMENT0);
    m_functions.glReadPixels(xi, m_viewport.w - yi - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    m_functions.glReadBuffer(GL_DEPTH_ATTACHMENT);
    m_functions.glReadPixels(xi, m_viewport.w - yi - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pickDepth);

//    std::vector<uint8_t> color(m_viewport.z * m_viewport.w * 4);
//    m_functions.glReadBuffer(GL_COLOR_ATTACHMENT0);
//    m_functions.glReadPixels(0, 0, m_viewport.z, m_viewport.w, GL_RGBA, GL_UNSIGNED_BYTE, color.data());
//    QImage img(color.data(), m_viewport.z, m_viewport.w, QImage::Format_RGBA8888);
//    img.save("qwe.png");

    QOpenGLFramebufferObject::bindDefault();

    pickColor = SelectionMeshDrawable::colorToId(color[0], color[1], color[2], color[3]);
}

void Renderer::setViewMatrix(const glm::mat4x4& value)
{
    m_viewMatrix = value;
}

void Renderer::setProjectionMatrix(float fov, float znear, float zfar)
{
    m_fov = fov;
    m_zNear = znear;
    m_zFar = zfar;
}

const glm::mat4x4& Renderer::projectionMatrix() const
{
    return m_projMatrix;
}

const glm::ivec4& Renderer::viewport() const
{
    return m_viewport;
}

void Renderer::resize(int width, int height)
{
    m_viewport = glm::ivec4(0, 0, width, height);
    m_selectionFramebuffer->resize(m_viewport.z, m_viewport.w);
}

void Renderer::render()
{
    using RenderMethod = void(Renderer::*)(DrawDataContainer::iterator, DrawDataContainer::iterator);
    static const std::array<RenderMethod, numElementsLayerId()> renderMethods {
        &Renderer::renderSelectionLayer,
        &Renderer::renderSolidLayer,
        &Renderer::renderTransparentLayer
    };

    m_functions.glClearColor(.4f, .4f, .85f, 1.f);
    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_functions.glViewport(m_viewport.x, m_viewport.y, m_viewport.z, m_viewport.w);
    m_projMatrix = glm::perspective(m_fov, static_cast<float>(m_viewport.z)/static_cast<float>(m_viewport.w), m_zNear, m_zFar);

    for (auto begin = m_drawData.begin(); begin != m_drawData.end(); )
    {
        auto layerId = begin->first->layerId();

        auto end = m_drawData.upper_bound(*begin);
        (this->*renderMethods[castFromLayerId(layerId)])(begin, end);
        begin = end;
    }
    m_drawData.clear();
}

void Renderer::renderSelectionLayer(DrawDataContainer::iterator, DrawDataContainer::iterator)
{
}

void Renderer::renderSolidLayer(DrawDataContainer::iterator begin, DrawDataContainer::iterator end)
{
    m_functions.glEnable(GL_DEPTH_TEST);

    for (; begin != end; ++begin)
    {
        auto drawable = begin->first;
        const auto& transform = begin->second;

        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        glm::mat4x4 modelMatrix = transform.operator glm::mat4x4();
        glm::mat3x3 normalMatrix = glm::inverseTranspose(modelMatrix);

        renderProgram->setUniform(renderProgram->uniformLocation("u_projMatrix"), m_projMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_viewMatrix"), m_viewMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_modelMatrix"), modelMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_normalMatrix"), normalMatrix);

        drawable->prerender();

        auto mesh = drawable->mesh();
        if (mesh)
        {
            m_functions.glBindVertexArray(mesh->id);
            for (auto ibo : mesh->indexBuffers)
                m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
            m_functions.glBindVertexArray(0);
        }

        drawable->postrender();
    }
}

void Renderer::renderTransparentLayer(DrawDataContainer::iterator, DrawDataContainer::iterator)
{

}

bool Renderer::DrawDataComarator::operator ()(const Renderer::DrawDataType& left, const Renderer::DrawDataType& right) const
{
    return left.first->layerId() < right.first->layerId();
}

bool Renderer::DrawDataComarator::operator ()(const Renderer::DrawDataType& left, LayerId id) const
{
    return left.first->layerId() < id;
}
