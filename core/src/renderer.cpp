#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include <glm/gtc/type_ptr.hpp>

#include <core/core.h>

#include "coreprivate.h"
#include "renderwidget.h"
#include "renderer.h"
#include "drawables.h"
#include "model.inl"

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

    if (count >= 0)
    {
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
{
    auto& functions = Renderer::instance().functions();
    functions.glGenBuffers(1, &id);
    functions.glBindBuffer(GL_ARRAY_BUFFER, id);
    functions.glBufferData(GL_ARRAY_BUFFER, size, data, usage);
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

VertexBuffer::VertexBuffer(GLsizei nv, GLsizei nc, float *data, GLenum usage)
    : Buffer(static_cast<GLsizeiptr>(nv*nc*sizeof(float)), data, usage)
    , numVertices(nv)
    , numComponents(nc)
{
}

void VertexBuffer::declareAttribute(VertexAttribute attrib, GLsizei offset)
{
    declaration.insert({attrib, offset});
}

IndexBuffer::IndexBuffer(GLenum primitiveType_, GLsizei numIndices_, uint32_t *data, GLenum usage)
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

void Mesh::attachVertexBuffer(std::shared_ptr<VertexBuffer> b)
{
    std::unordered_set<VertexAttribute> attribs;
    attribs.reserve(b->declaration.size());
    for (const auto& attrib : b->declaration)
        attribs.insert(attrib.first);

    attachVertexBuffer(b, attribs);
}

void Mesh::attachVertexBuffer(std::shared_ptr<VertexBuffer> b, const std::unordered_set<VertexAttribute>& attribs)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindVertexArray(id);
    functions.glBindBuffer(GL_ARRAY_BUFFER, b->id);

    GLsizei stride = static_cast<GLsizei>(b->numComponents * sizeof(float));

    for (const auto& attrib : attribs)
    {
        functions.glVertexAttribPointer(castFromVertexAttribute(attrib),
                                        static_cast<GLint>(numAttributeComponents(attrib)),
                                        GL_FLOAT,
                                        GL_FALSE,
                                        stride,
                                        reinterpret_cast<const GLvoid*>(b->declaration[attrib] * sizeof(float)));
        functions.glEnableVertexAttribArray(castFromVertexAttribute(attrib));
    }

    functions.glBindVertexArray(0);

    vertexBuffers.insert(b);
}

void Mesh::attachIndexBuffer(std::shared_ptr<IndexBuffer> b)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindVertexArray(id);
    functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->id);
    functions.glBindVertexArray(0);

    indexBuffers.insert(b);
}

void Mesh::tmp(std::shared_ptr<VertexBuffer> b)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindVertexArray(id);
    functions.glBindBuffer(GL_ARRAY_BUFFER, b->id);

    GLsizei stride = static_cast<GLsizei>(b->numComponents * sizeof(float));

    for (const auto& attrib : b->declaration)
    {
        functions.glVertexAttribPointer(castFromVertexAttribute(attrib.first),
                                        static_cast<GLint>(numAttributeComponents(attrib.first)),
                                        GL_FLOAT,
                                        GL_FALSE,
                                        stride,
                                        reinterpret_cast<const GLvoid*>(attrib.second * sizeof(float)));
        functions.glEnableVertexAttribArray(castFromVertexAttribute(attrib.first));
    }

    functions.glBindVertexArray(0);

}

Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
    , m_projMatrix(1.0f)
    , m_viewMatrix(1.0f)
{
}

void Renderer::initializeResources()
{
    m_standardLayer = std::make_shared<Layer>();

    m_standardTexture = loadTexture(":/res/chess.png");
    m_standardTexture->generateMipmaps();

    m_coloredRenderProgram = loadRenderProgram(":/res/color.vert", ":/res/color.frag");

    m_skeletalAnimationProgram = loadRenderProgram(":/res/skeletal_animation.vert", ":/res/skeletal_animation.frag");
    m_skeletalAnimationProgram->setupTransformFeedback({"v_position"}, GL_INTERLEAVED_ATTRIBS);
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
    auto object = ResourceStorage::instance().get(key);
    if (!object)
    {
        std::array<std::pair<GLenum, QString>, 2> shaderFilenames {
            std::make_pair(GL_VERTEX_SHADER, QString::fromStdString(vertexFile)),
            std::make_pair(GL_FRAGMENT_SHADER, QString::fromStdString(fragmentFile))
        };

        GLuint shaderIds[2];
        bool isOk = true;
        for (int32_t i = 0; i < 2; ++i)
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
        ResourceStorage::instance().store(key, object);
    }

    return std::static_pointer_cast<RenderProgram>(object);
}

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filename)
{
    auto object = ResourceStorage::instance().get(filename);
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
        ResourceStorage::instance().store(filename, object);
    }

    return std::static_pointer_cast<Texture>(object);
}

std::shared_ptr<Layer> Renderer::getOrCreateLayer(int32_t layerId)
{
    std::shared_ptr<Layer> layer;

    auto it = m_layers.find(layerId);
    if (it != m_layers.end())
        layer = it->second;
    else
        m_layers.insert({layerId, layer});

    return layer;
}

void Renderer::beginTransformFeedback(GLenum mode)
{
    auto& functions = Renderer::instance().functions();

    functions.glEnable(GL_RASTERIZER_DISCARD);
    functions.glBeginTransformFeedback(mode);
}

void Renderer::endTransformFeedback()
{
    auto& functions = Renderer::instance().functions();

    functions.glEndTransformFeedback();
    functions.glDisable(GL_RASTERIZER_DISCARD);
}

void Renderer::bindTexture(std::shared_ptr<Texture> texture, GLint unit)
{
    auto& functions = Renderer::instance().functions();
    functions.glActiveTexture(static_cast<GLenum>(GL_TEXTURE0+unit));
    functions.glBindTexture(texture->target, texture->id);
}

void Renderer::bindUniformBuffer(std::shared_ptr<Buffer> buffer, GLuint unit)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBufferBase(GL_UNIFORM_BUFFER, unit, buffer->id);
}

void Renderer::bindTransformFeedbackBuffer(std::shared_ptr<Buffer> buffer, GLuint unit)
{
    auto& functions = Renderer::instance().functions();
    functions.glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, unit, buffer->id);
}

std::shared_ptr<MeshDrawable> Renderer::createMeshDrawable(std::shared_ptr<RenderProgram> rp, std::shared_ptr<Model::Mesh> m, std::shared_ptr<VertexBuffer> ab) const
{
    return std::make_shared<MeshDrawable>(rp, m, ab);
}

std::shared_ptr<SphereDrawable> Renderer::createSphereDrawable(uint32_t segs, const BoundingSphere &sphere, const glm::vec4& color) const
{
    return std::make_shared<SphereDrawable>(m_coloredRenderProgram, segs, sphere, color);
}

std::shared_ptr<FrustumDrawable> Renderer::createFrustumDrawable(const Frustum& frustum, const glm::vec4& color) const
{
    return std::make_shared<FrustumDrawable>(m_coloredRenderProgram, frustum, color);
}

void Renderer::draw(int32_t layerId, std::shared_ptr<Drawable> drawable, const Transform& transform)
{
    m_drawData.insert({layerId, std::make_pair(drawable, transform)});
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

const glm::mat4x4 &Renderer::projectionMatrix() const
{
    return m_projMatrix;
}

const glm::ivec4 Renderer::viewport() const
{
    return m_viewport;
}

void Renderer::drawMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<RenderProgram> p)
{
    m_functions.glUseProgram(p->id);

    m_functions.glBindVertexArray(mesh->id);
    for (auto ibo : mesh->indexBuffers)
        m_functions.glDrawElements(ibo->primitiveType, ibo->numIndices, GL_UNSIGNED_INT, nullptr);
    m_functions.glBindVertexArray(0);
}

void Renderer::resize(int width, int height)
{
    m_viewport = glm::ivec4(0, 0, width, height);
}

void Renderer::render()
{
    m_functions.glEnable(GL_DEPTH_TEST);

    m_functions.glClearColor(.5f, .5f, 1.f, 1.f);
    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_functions.glViewport(m_viewport.x, m_viewport.y, m_viewport.z, m_viewport.w);
    m_projMatrix = glm::perspective(m_fov, static_cast<float>(m_viewport.z)/static_cast<float>(m_viewport.w), m_zNear, m_zFar);

    for (DrawDataType::iterator begin = m_drawData.begin(); begin != m_drawData.end(); )
    {
        DrawDataType::iterator end = m_drawData.upper_bound(begin->first);
        renderLayer(begin, end);
        begin = end;
    }
    m_drawData.clear();
}

void Renderer::renderLayer(DrawDataType::iterator begin, DrawDataType::iterator end)
{
    auto layer = m_standardLayer;

    auto it = m_layers.find(begin->first);
    if (it != m_layers.end())
        layer = it->second;

    for (; begin != end; ++begin)
    {
        auto drawable = begin->second.first;
        const auto& transform = begin->second.second;

        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        renderProgram->setUniform(renderProgram->uniformLocation("u_projMatrix"), m_projMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_viewMatrix"), m_viewMatrix);
        renderProgram->setUniform(renderProgram->uniformLocation("u_modelMatrix"), transform.operator glm::mat4x4());

        drawable->setup();

        auto mesh = drawable->mesh();
        if (mesh)
            drawMesh(mesh, renderProgram);
    }
}
