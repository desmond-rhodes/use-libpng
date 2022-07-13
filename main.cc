#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <png.h>
#include <zlib.h>

GLuint shader_create(size_t, GLenum const[], char const* const[]);

int main() {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	std::cout << "Compiled with libpng " << PNG_LIBPNG_VER_STRING  << "; using libpng " << png_libpng_ver << ".\n";
	std::cout << "Compiled with zlib " << ZLIB_VERSION << "; using zlib " << zlib_version << ".\n";

	if (!glfwInit())
		return -1;
	struct glfwTerminate_t {
		~glfwTerminate_t() { glfwTerminate(); }
	} glfwTerminate_h;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 960, "Hello World", nullptr, nullptr);

	if (!window)
		return -1;

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height){
		glViewport(0, 0, width, height);
	});

	glfwMakeContextCurrent(window);

	if (gl3wInit())
		return -1;

	std::cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	GLuint tex;
	GLuint sam;
	GLuint sha;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	{
		GLuint const image[] {
			0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
			0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
			0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
			0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
			0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
			0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
			0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
			0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff
		};

		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		glTextureStorage2D(tex, 1, GL_RGBA8, 8, 8);
		glTextureSubImage2D(tex, 0, 0, 0, 8, 8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);

		glCreateSamplers(1, &sam);
		glSamplerParameteri(sam, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sam, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		GLenum const type[] {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

		char const* const src[] {
			"#version 450 core\n"
			"in vec4 vPosition;\n"
			"in vec2 vTexCoord;\n"
			"out vec2 fTexCoord;\n"
			"void main() {\n"
			"	gl_Position = vPosition;\n"
			"	fTexCoord = vTexCoord;\n"
			"}\n"
		,
			"#version 450 core\n"
			"uniform sampler2D tex;\n"
			"in vec2 fTexCoord;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	color = texture(tex, fTexCoord);\n"
			"}\n"
		};

		sha = shader_create(2, type, src);
		if (!sha)
			return -1;
		glUseProgram(sha);

		GLint const uniform_tex {glGetUniformLocation(sha, "tex")};
		glBindSampler(uniform_tex, sam);
		glBindTextureUnit(uniform_tex, tex);

		GLuint const bindingindex {0};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		auto const attrib_position {glGetAttribLocation(sha, "vPosition")};
		glEnableVertexAttribArray(attrib_position);
		glVertexAttribFormat(attrib_position, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(attrib_position, bindingindex);

		auto const attrib_texcoord {glGetAttribLocation(sha, "vTexCoord")};
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribFormat(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat));
		glVertexAttribBinding(attrib_texcoord, bindingindex);

		GLfloat vertex[] {-0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f};
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(vertex), vertex, 0);
		glBindVertexBuffer(bindingindex, vbo, 0, 4*sizeof(GLfloat));

		GLuint index[] {0, 1, 2, 0, 3, 2};
		glCreateBuffers(1, &ebo);
		glNamedBufferStorage(ebo, sizeof(index), index, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	GLfloat const fill[] {0.0f, 0.0f, 0.0f, 0.0f};

	std::cout << std::flush;

	while (!glfwWindowShouldClose(window)) {
		glClearBufferfv(GL_COLOR, 0, fill);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	return 0;
}

GLuint shader_create(size_t n, GLenum const type[], char const* const src[]) {
	GLint status;
	GLint length;

	auto const pro {glCreateProgram()};
	if (!pro)
		return 0;
	struct programDelete_t {
		bool r {true}; void cancel() { r = false; }
		GLuint pro;
		programDelete_t(GLuint p) : pro {p} {}
		~programDelete_t() { if (r) glDeleteProgram(pro); }
	} programDelete_h(pro);

	auto detach {new GLuint[n]};
	struct detachDelete_t {
		GLuint* detach;
		detachDelete_t(GLuint* d) : detach {d} {}
		~detachDelete_t() { delete[] detach; }
	} detachDelete_h(detach);

	for (size_t i {0}; i < n; ++i) {
		auto const obj {glCreateShader(type[i])};
		if (!obj)
			return 0;
		struct objectDelete_t {
			GLuint obj;
			objectDelete_t(GLuint o) : obj {o} {}
			~objectDelete_t() { glDeleteShader(obj); }
		} objectDelete_h(obj);

		glShaderSource(obj, 1, src+i, nullptr);
		glCompileShader(obj);
		glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
			auto log {new char[length]};
			glGetShaderInfoLog(obj, length, nullptr, log);
			std::cerr << log << '\n';
			delete[] log;
			return 0;
		}
		glAttachShader(pro, obj);
		detach[i] = obj;
	}

	glLinkProgram(pro);
	glGetProgramiv(pro, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &length);
		auto log {new char[length]};
		glGetProgramInfoLog(pro, length, nullptr, log);
		std::cerr << log << '\n';
		delete[] log;
		return 0;
	}
	for (size_t i {0}; i < n; ++i)
		glDetachShader(pro, detach[i]);

	programDelete_h.cancel();
	return pro;
}
