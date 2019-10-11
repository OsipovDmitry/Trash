#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include "renderer.h"

#include <iostream>

Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
{
    m_functions.glClearColor(.5f, .5f, 1.f, 1.f);
}

Renderer::~Renderer()
{
    m_functions.glDeleteBuffers(1, &m_vbo);
    m_functions.glDeleteVertexArrays(1, &m_vao);
}

std::shared_ptr<RenderProgram> Renderer::loadRenderProgram(const std::string &vertexFile, const std::string &fragmentFile)
{
    static auto loadShader = [this](const std::string& filename, GLenum type, std::string& log) -> GLuint {
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

    static auto deleteShader = [this](GLuint id) {
        if (!id)
            return;
        m_functions.glDeleteShader(id);
    };

    static auto loadProgram = [this](GLuint vertexId, GLuint fragmentId, std::string& log) -> GLuint
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

    static auto deleteProgram = [this](GLuint id) {
        if (!id)
            return;
        GLuint shaders[2];
        GLsizei count = 0;
        m_functions.glGetAttachedShaders(id, 2, &count, shaders);
        m_functions.glDetachShader(id, shaders[0]);
        deleteShader(shaders[0]);
        m_functions.glDetachShader(id, shaders[1]);
        deleteShader(shaders[1]);
        m_functions.glDeleteProgram(id);
    };

    static auto deleter = [](RenderProgram* p) {
        deleteProgram(p->id);
        delete p;
    };

    const std::string key = vertexFile+fragmentFile;
    auto object = ResourceStorage::instance().get(key);
    if (!object)
    {
        std::string vertexLog, fragmentLog, programLog;
        GLuint vertexId = loadShader(vertexFile, GL_VERTEX_SHADER, vertexLog);
        GLuint fragmentId = loadShader(fragmentFile, GL_FRAGMENT_SHADER, fragmentLog);
        GLuint programId = loadProgram(vertexId, fragmentId, programLog);
        if (!vertexId || !fragmentId || !programId)
        {
            deleteShader(vertexId);
            deleteShader(fragmentId);
            std::cout <<
                         "Vertex: " << vertexLog <<
                         "Fragment: " << fragmentLog <<
                         "Program: " << programLog << std::endl;
        }
        object = std::shared_ptr<RenderProgram>(new RenderProgram(programId), deleter);
        ResourceStorage::instance().store(key, object);
    }

    return std::static_pointer_cast<RenderProgram>(object);
}

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& filename)
{
    static auto deleter = [this](Texture* p) {
        m_functions.glDeleteTextures(1, &(p->id));
        delete p;
    };

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

        object = std::shared_ptr<Texture>(new Texture(id), deleter);
        ResourceStorage::instance().store(filename, object);
    }

    return std::static_pointer_cast<Texture>(object);
}

void Renderer::resize(int width, int height)
{
    m_functions.glViewport(0, 0, width, height);
}

void Renderer::render()
{
    if (!m_renderProgram)
        m_renderProgram = loadRenderProgram(":/resources/shader.vert", ":/resources/shader.frag");

    if (!m_texture)
        m_texture = loadTexture(":/resources/brick.jpg");

    if (!m_vao)
    {
        static const float vertices[] {
            -0.7f, -0.7f, -1.0f, 0.0f, 1.0f,
            +0.7f, -0.7f, -1.0f, 1.0f, 1.0f,
            +0.0f, +0.7f, -1.0f, 0.5f, 0.0f,
        };

        m_functions.glGenVertexArrays(1, &m_vao);
        m_functions.glBindVertexArray(m_vao);

        m_functions.glGenBuffers(1, &m_vbo);
        m_functions.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_functions.glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(float), vertices, GL_STATIC_DRAW);

        m_functions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const GLvoid*>(0 * sizeof(float)));
        m_functions.glEnableVertexAttribArray(0);

        m_functions.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const GLvoid*>(3 * sizeof(float)));
        m_functions.glEnableVertexAttribArray(1);

        m_functions.glBindVertexArray(0);
    }

    m_functions.glClear(GL_COLOR_BUFFER_BIT);

    m_functions.glUseProgram(m_renderProgram->id);
    m_functions.glBindVertexArray(m_vao);

    m_functions.glUniform1i(m_functions.glGetUniformLocation(m_renderProgram->id, "u_diffuseMap"), 0);
    m_functions.glActiveTexture(GL_TEXTURE0);
    m_functions.glBindTexture(GL_TEXTURE_2D, m_texture->id);

    m_functions.glDrawArrays(GL_TRIANGLES, 0, 3);

    m_functions.glUseProgram(0);
    m_functions.glBindVertexArray(0);

}
