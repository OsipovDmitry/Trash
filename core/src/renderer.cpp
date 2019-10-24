#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "model.inl"

#include <iostream>

Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
{
    m_loadShader = [this](const std::string& filename, GLenum type, std::string& log) -> GLuint {
        QFile file(QString::fromStdString(filename));

        if (!file.open(QFile::ReadOnly))
            return 0;

        auto byteArray = file.readAll();
        const char *data = byteArray.data();

        GLuint id = m_functions.glCreateShader(type);
        if (!id)
            return 0;
        m_functions.glShaderSource(id, 1, &data, nullptr);
        m_functions.glCompileShader(id);
        GLint compiled;
        m_functions.glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            m_functions.glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen > 1)
            {
                char *infoLog = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(infoLen)));
                m_functions.glGetShaderInfoLog(id, infoLen, nullptr, infoLog);
                log = infoLog;
                free(infoLog);
            }
            m_functions.glDeleteShader(id);
            return 0;
        }

        file.close();
        return id;
    };

    m_deleteShader = [this](GLuint id) {
        m_functions.glDeleteShader(id);
    };

    m_loadProgram = [this](GLuint vertexId, GLuint fragmentId, std::string& log) -> GLuint
    {
        if (!vertexId || !fragmentId)
            return 0;
        GLuint id = m_functions.glCreateProgram();
        if (!id)
            return 0;
        m_functions.glAttachShader(id, vertexId);
        m_functions.glAttachShader(id, fragmentId);
        m_functions.glLinkProgram(id);
        GLint linked;
        m_functions.glGetProgramiv(id, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint infoLen = 0;
            m_functions.glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);
            if(infoLen > 1) {
                char *infoLog = static_cast<char*>(malloc(sizeof(char) * static_cast<unsigned int>(infoLen)));
                m_functions.glGetProgramInfoLog(id, infoLen, nullptr, infoLog);
                log = infoLog;
                free(infoLog);
            }
            m_functions.glDeleteProgram(id);
            return 0;
        }
        return id;
    };

    m_deleteProgram = [this](GLuint id) {
        if (!id)
            return;
        GLuint shaders[2];
        GLsizei count = 0;
        m_functions.glGetAttachedShaders(id, 2, &count, shaders);
        m_functions.glDetachShader(id, shaders[0]);
        m_deleteShader(shaders[0]);
        m_functions.glDetachShader(id, shaders[1]);
        m_deleteShader(shaders[1]);
        m_functions.glDeleteProgram(id);
    };

    m_renderProgramDeleter = [this](RenderProgram* p) {
        m_deleteProgram(p->id);
        delete p;
    };

    m_textureDeleter = [this](Texture* p) {
        m_functions.glDeleteTextures(1, &(p->id));
        delete p;
    };

    m_meshDeleter = [this](Model::Mesh* p) {
        m_functions.glDeleteVertexArrays(1, &p->vao);
        m_functions.glDeleteBuffers(1, &p->vbo);
        m_functions.glDeleteBuffers(1, &p->ibo);
        delete p;
    };

    m_standardTexture = loadTexture(":/res/chess.png");
    generateMipmaps(m_standardTexture);

    m_functions.glClearColor(.5f, .5f, 1.f, 1.f);
    m_functions.glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer()
{
}

std::shared_ptr<RenderProgram> Renderer::loadRenderProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
    const std::string key = vertexFile+fragmentFile;
    auto object = ResourceStorage::instance().get(key);
    if (!object)
    {
        std::string vertexLog, fragmentLog, programLog;
        GLuint vertexId = m_loadShader(vertexFile, GL_VERTEX_SHADER, vertexLog);
        GLuint fragmentId = m_loadShader(fragmentFile, GL_FRAGMENT_SHADER, fragmentLog);
        GLuint programId = m_loadProgram(vertexId, fragmentId, programLog);
        if (!vertexId || !fragmentId || !programId)
        {
            m_deleteShader(vertexId);
            m_deleteShader(fragmentId);
            std::cout <<
                         "Vertex: " << vertexLog <<
                         "Fragment: " << fragmentLog <<
                         "Program: " << programLog << std::endl;
        }
        object = std::shared_ptr<RenderProgram>(new RenderProgram(programId), m_renderProgramDeleter);
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

        object = std::shared_ptr<Texture>(new Texture(id), m_textureDeleter);
        ResourceStorage::instance().store(filename, object);
    }

    return std::static_pointer_cast<Texture>(object);
}

void Renderer::generateMipmaps(std::shared_ptr<Texture> texture)
{
    m_functions.glBindTexture(GL_TEXTURE_2D, texture->id);
    m_functions.glGenerateMipmap(GL_TEXTURE_2D);
    m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_functions.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Renderer::draw(uint32_t layerId, std::shared_ptr<Drawable> drawable, const Transform& transform)
{
    m_drawData.insert({layerId, std::make_pair(drawable, transform)});
}

void Renderer::resize(int width, int height)
{
    m_functions.glViewport(0, 0, width, height);
    m_projMatrix = glm::perspective(glm::pi<float>() * 0.25f, (float)width/(float)height, 0.5f, 2000.0f);
}

void Renderer::render()
{
    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    //uint32_t layerId = begin->first;

    for (; begin != end; ++begin)
    {
        auto drawable = begin->second.first;
        const auto& transform = begin->second.second;

        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        m_functions.glUniformMatrix4fv(m_functions.glGetUniformLocation(renderProgram->id, "u_projMatrix"), 1, false, glm::value_ptr(m_projMatrix));
        m_functions.glUniformMatrix4fv(m_functions.glGetUniformLocation(renderProgram->id, "u_modelMatrix"), 1, false, glm::value_ptr(transform.operator glm::mat4x4()));

        m_functions.glUniform1i(m_functions.glGetUniformLocation(renderProgram->id, "u_diffuseMap"), 0);
        m_functions.glActiveTexture(GL_TEXTURE0);
        m_functions.glBindTexture(GL_TEXTURE_2D, drawable->diffuseTexture() ? drawable->diffuseTexture()->id : m_standardTexture->id);

        m_functions.glBindVertexArray(drawable->vao());
        m_functions.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawable->numIndices()), GL_UNSIGNED_INT, nullptr);
    }
}
