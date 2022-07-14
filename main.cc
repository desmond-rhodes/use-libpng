#include <functional>

class cleanup {
public:
	cleanup(std::function<void()> x) : f {x} {}
	~cleanup() { f(); }
private:
	std::function<void()> f;
};

class optional_cleanup {
public:
	void disable() { r = false; }
	optional_cleanup(std::function<void()> x) : f {x} {}
	~optional_cleanup() { if (r) f(); }
private:
	bool r {true};
	std::function<void()> f;
};

#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <png.h>
#include <zlib.h>
#include <cstdio>

GLuint shader_create(size_t, GLenum const[], char const* const[]);

int main() {
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	std::cout << "Compiled with libpng " << PNG_LIBPNG_VER_STRING  << "; using libpng " << png_libpng_ver << ".\n";
	std::cout << "Compiled with zlib " << ZLIB_VERSION << "; using zlib " << zlib_version << ".\n";

	png_uint_32 width;
	png_uint_32 height;
	unsigned char* image_data;
	{
		std::cout << "Reading image... " << std::flush;

		FILE* file {fopen("brick-wall.png", "rb+")};
		if (!file)
			return -1;
		cleanup c_file {[&]{ fclose(file); }};

		unsigned char sig[8];
		fread(sig, 1, 8, file);
		if (!png_check_sig(sig, 8))
			return -1;

		png_structp png_ptr {png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)};
		if (!png_ptr)
			return -1;

		png_infop info_ptr {png_create_info_struct(png_ptr)};
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, nullptr, nullptr);
			return -1;
		}

		cleanup c_png {[&]{ png_destroy_read_struct(&png_ptr, &info_ptr, nullptr); }};

		if (setjmp(png_jmpbuf(png_ptr)))
			return -1;

		png_init_io(png_ptr, file);
		png_set_sig_bytes(png_ptr, 8);
		png_read_info(png_ptr, info_ptr);

		int bit_depth;
		int color_type;

		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

		double const LUT_exponent {1.0};
		double const CRT_exponent {2.2};
		double const default_display_exponent {LUT_exponent * CRT_exponent};

		double const display_exponent {default_display_exponent};

		if (setjmp(png_jmpbuf(png_ptr)))
			return -1;

		if (color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand(png_ptr);
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_expand(png_ptr);

		if (bit_depth == 16)
			png_set_strip_16(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png_ptr);

		if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_BEFORE);

		double gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &gamma))
			png_set_gamma(png_ptr, display_exponent, gamma);

		png_read_update_info(png_ptr, info_ptr);

		png_uint_32 rowbytes {static_cast<png_uint_32>(png_get_rowbytes(png_ptr, info_ptr))};
		int channels {static_cast<int>(png_get_channels(png_ptr, info_ptr))};

		auto row_pointers {new png_bytep[height]};
		if (!row_pointers)
			return -1;
		cleanup c_row_pointers {[&]{ delete[] row_pointers; }};

		image_data = new unsigned char[rowbytes*height];
		if (!image_data)
			return -1;

		for (size_t i {0}; i < height; ++i)
			row_pointers[i] = image_data + i * rowbytes;

		png_read_image(png_ptr, row_pointers);

		std::cout << "done.\n" << std::flush;
		std::cout << "Available channels " << channels << '\n';
	}

	if (!glfwInit())
		return -1;
	cleanup c_glfw {[&]{ glfwTerminate(); }};

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
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
		// glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image_data);
		glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, image_data);

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
	optional_cleanup c_pro {[&]{ glDeleteProgram(pro); }};
	auto det {new GLuint[n]};
	cleanup c_det {[&]{ delete[] det; }};
	for (size_t i {0}; i < n; ++i) {
		auto const obj {glCreateShader(type[i])};
		if (!obj)
			return 0;
		cleanup c_obj {[&]{ glDeleteShader(obj); }};
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
		det[i] = obj;
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
		glDetachShader(pro, det[i]);
	c_pro.disable();
	return pro;
}
