#include "display.hh"
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <png.h>
#include <zlib.h>

using namespace std;

GLuint shader_create(vector<GLenum> const&, vector<string> const&);

int display(vector<string> const&) {
	cout
		<< png_get_copyright(nullptr) << '\n'
		<< " library (" << static_cast<unsigned long long>(png_access_version_number()) << "):" << png_get_header_version(nullptr)
		<< " build   (" << static_cast<unsigned long long>(PNG_LIBPNG_VER) << "):" << PNG_HEADER_VERSION_STRING << '\n'
		<< "Compiled with libpng " << PNG_LIBPNG_VER_STRING  << "; using libpng " << png_libpng_ver << ".\n"
		<< "Compiled with zlib " << ZLIB_VERSION << "; using zlib " << zlib_version << ".\n\n";

	struct glfwHandle {
		glfwHandle() {
			if (!glfwInit())
				throw runtime_error("glfwHandle");
		}
		~glfwHandle() {
			glfwTerminate();
		}
	}
	use_glfw;

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

	cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n' << flush;

	GLuint tex;
	GLuint sam;
	GLuint sha;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	{
		vector<GLuint> image {
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
		glTextureSubImage2D(tex, 0, 0, 0, 8, 8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image.data());

		glCreateSamplers(1, &sam);
		glSamplerParameteri(sam, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sam, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		string const vert_src {
			"#version 450 core\n"
			"in vec4 vPosition;\n"
			"in vec2 vTexCoord;\n"
			"out vec2 fTexCoord;\n"
			"void main() {\n"
			"	gl_Position = vPosition;\n"
			"	fTexCoord = vTexCoord;\n"
			"}\n"
		};

		string const frag_src {
			"#version 450 core\n"
			"uniform sampler2D tex;\n"
			"in vec2 fTexCoord;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	color = texture(tex, fTexCoord);\n"
			"}\n"
		};

		sha = shader_create({GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, {vert_src, frag_src});
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

	vector<GLfloat> const fill {0.0f, 0.0f, 0.0f, 0.0f};

	while (!glfwWindowShouldClose(window)) {
		glClearBufferfv(GL_COLOR, 0, fill.data());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

GLuint shader_create(vector<GLenum> const& type, vector<string> const& src) {
	GLint status;
	auto const pro {glCreateProgram()};
	if (!pro)
		throw runtime_error("shader_link");
	vector<GLuint> detach;
	for (size_t i {0}; i < type.size(); ++i) {
		auto const obj {glCreateShader(type[i])};
		if (!obj)
			throw runtime_error("shader_compile");
		GLchar const* v_src[] {src[i].data()};
		glShaderSource(obj, 1, v_src, nullptr);
		glCompileShader(obj);
		glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint length;
			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
			string log(length, '\0');
			glGetShaderInfoLog(obj, length, nullptr, log.data());
			cerr << log << '\n';
			glDeleteShader(obj);
			glDeleteProgram(pro);
			throw runtime_error("shader_compile");
		}
		glAttachShader(pro, obj);
		glDeleteShader(obj);
		detach.push_back(obj);
	}
	glLinkProgram(pro);
	glGetProgramiv(pro, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &length);
		string log(length, '\0');
		glGetProgramInfoLog(pro, length, nullptr, log.data());
		cerr << log << '\n';
		glDeleteProgram(pro);
		throw runtime_error("shader_link");
	}
	for (auto const& obj : detach)
		glDetachShader(pro, obj);
	return pro;
}
