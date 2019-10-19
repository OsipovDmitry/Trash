#include <QtGui/QOpenGLExtraFunctions>
#include <QtGui/QImage>
#include <QtCore/QFile>

#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"

#include <iostream>

Renderer::Renderer(QOpenGLExtraFunctions& functions)
    : m_functions(functions)
{
    m_functions.glClearColor(.5f, .5f, 1.f, 1.f);
    m_functions.glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer()
{
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

unsigned int Renderer::generateTriangle()
{
    static const float vertices[] {
                -0.2f, -0.2f, -1.0f, 0.0f, 1.0f,
                +0.2f, -0.2f, -1.0f, 1.0f, 1.0f,
                +0.0f, +0.2f, -1.0f, 0.5f, 0.0f
    };

    static const unsigned int indices[] {0, 1, 2};

    GLuint vao, vbo, ibo;

    m_functions.glGenVertexArrays(1, &vao);
    m_functions.glBindVertexArray(vao);

    m_functions.glGenBuffers(1, &vbo);
    m_functions.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    m_functions.glBufferData(GL_ARRAY_BUFFER, 5 * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
    m_functions.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    m_functions.glEnableVertexAttribArray(0);
    m_functions.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<const void*>(3 * sizeof(float)));
    m_functions.glEnableVertexAttribArray(1);

    m_functions.glGenBuffers(1, &ibo);
    m_functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    m_functions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    m_functions.glBindVertexArray(vao);

    return vao;
}

void Renderer::draw(std::shared_ptr<Drawable> drawable)
{
    m_drawData.insert(drawable);
}

void Renderer::resize(int width, int height)
{
    m_functions.glViewport(0, 0, width, height);
}

void Renderer::render()
{
    m_functions.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto drawable : m_drawData)
    {
        auto renderProgram = drawable->renderProgram();
        m_functions.glUseProgram(renderProgram->id);

        m_functions.glUniformMatrix4fv(m_functions.glGetUniformLocation(renderProgram->id, "u_modelMatrix"), 1, false, glm::value_ptr(drawable->modelMatrix()));

        m_functions.glUniform1i(m_functions.glGetUniformLocation(renderProgram->id, "u_diffuseMap"), 0);
        m_functions.glActiveTexture(GL_TEXTURE0);
        m_functions.glBindTexture(GL_TEXTURE_2D, drawable->diffuseTexture()->id);

        m_functions.glBindVertexArray(drawable->vao());
        m_functions.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(drawable->numIndices()), GL_UNSIGNED_INT, nullptr);

    }
    m_drawData.clear();
}
