#include "../../src/OSDBKG.HPP"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>
#endif

const struct desktop_consts_t {
	const char* title = "OSDBKG";
	const char* tag_main = "MAIN";
	const char* tag_test = "oglx_test";
	const char* logwrite_tag_fmt = "%s/%s";
	const char* logwrite_print_fmt = "%s %s\n";
	const char* arg_disable_cgl = "--no-cgl";
	const char* arg_disable_cgl_warning = "[--no-cgl] program OpenGL error checking is disabled, for use with NSight / PerfStudio / RenderDoc";
	const char* arg_slow_fb_swap = "--slow-fb";
	const char* arg_slow_fb_swap_warning = "[--slow-fb] Running with Framebuffer swapping being waited";
	const char* arg_parent_hwnd = "-parentHWND";
	const char* arg_parent_hwnd_warning = "[-parentHWND] So... I'm adopted by \"%s\"? OK :'(";
	const char* arg_embed_keep_console = "--keep-console";
	const char* arg_embed_keep_console_warning = "[--keep-console] Console will be kept when embedding, hence this console anyway";
	const char* test_decompress_complete = "R.tex_image_tex decompressed %i -> %zi";
	const char* test_decompress_deleted = "R.tex_image_tex decompressed file deleted from memory";
} desk_consts;

#define FMT_BUFSZ 256
#define TAG_BUFSZ 16

struct desktop_state_t {
	GLFWwindow* window;
	char fmt_buffer[FMT_BUFSZ], tag_buffer[TAG_BUFSZ];
	GLuint test_w;
#ifdef WIN32
	HWND win32_embed_parent = 0;
	bool win32_embed = false;
	bool win32_embed_keep_console = true;
#endif
} desk_state;

void osd_desktop_resize(GLFWwindow* _, int w, int h) {
	glViewport(0, 0, w, h);
	states.screen.w = w;
	states.screen.h = h;
	states.screen.update = true;
}

void osd_log_write(const char* t, const char* tag, const char* fmt, va_list va) {
	memset(desk_state.tag_buffer, ' ', TAG_BUFSZ);
	desk_state.tag_buffer[TAG_BUFSZ - 1] = 0;
	int s = snprintf(desk_state.tag_buffer, TAG_BUFSZ, desk_consts.logwrite_tag_fmt, t,tag);
	if(s < TAG_BUFSZ) desk_state.tag_buffer[s] = ' ';
	s = vsnprintf(desk_state.fmt_buffer, FMT_BUFSZ, fmt, va);
	if (s < FMT_BUFSZ) desk_state.fmt_buffer[s] = 0;

	printf(desk_consts.logwrite_print_fmt, desk_state.tag_buffer, desk_state.fmt_buffer);
}

void check_arguments(int argc, const char** argv) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], desk_consts.arg_disable_cgl) == 0) {
			osd_log_W(desk_consts.tag_main, desk_consts.arg_disable_cgl_warning);
			states.disable_cgl = true;
		}
#if WIN32
		if (strcmp(argv[i], desk_consts.arg_parent_hwnd) == 0) {
			i++;
			desk_state.win32_embed_parent = (HWND)_strtoi64(argv[i], nullptr, 10);
			if (desk_state.win32_embed_parent != 0) {
				desk_state.win32_embed = true;
				size_t sz = GetWindowText(desk_state.win32_embed_parent, states.mdl_ldr_buf, 256);
				if (sz < 256) states.mdl_ldr_buf[sz] = 0;
				osd_log_W(desk_consts.tag_main, desk_consts.arg_parent_hwnd_warning, states.mdl_ldr_buf);
			}
		}
		if (strcmp(argv[i], desk_consts.arg_embed_keep_console) == 0) {
			osd_log_D(desk_consts.tag_main, desk_consts.arg_embed_keep_console_warning);
			desk_state.win32_embed_keep_console = true;
		}
#endif
		if (strcmp(argv[i], desk_consts.arg_slow_fb_swap)) {
			states.slow_fb_swap = true; 
			osd_log_W(desk_consts.tag_main, desk_consts.arg_slow_fb_swap_warning);
		}
	}
}

int main(int argc, const char** argv) {
	check_arguments(argc, argv);
	
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);

	states.screen.w = 1280;
	states.screen.h = 720;

#ifdef WIN32
	if (desk_state.win32_embed) {
		glfwWindowHint(GLFW_VISIBLE, 0);
		glfwWindowHint(GLFW_DECORATED, 0);
		RECT parent_rect;
		if (GetClientRect(desk_state.win32_embed_parent, &parent_rect)) {
			int w = parent_rect.right - parent_rect.left,
				h = parent_rect.bottom - parent_rect.top;
			states.screen.w = w;
			states.screen.h = h;
			osd_log_D(tags.entry, "Parent have size : %ix%i",w,h);
		}
	}
#endif

	desk_state.window = glfwCreateWindow(states.screen.w, states.screen.h, desk_consts.title, nullptr, nullptr);

#ifdef WIN32
	// Windows Only Section : Wallpaper Engine Binding
	if (desk_state.win32_embed) {
		HWND win32_window = glfwGetWin32Window(desk_state.window);
		SetParent(win32_window, desk_state.win32_embed_parent);
		LONG gwl = GetWindowLong(win32_window, GWL_STYLE);
		gwl = (gwl & ~WS_POPUP) | WS_CHILDWINDOW;
		SetWindowLong(win32_window, GWL_STYLE, gwl);
		glfwShowWindow(desk_state.window);
		glfwSetWindowPos(desk_state.window, 0, 0);
		// Hide Console when is bound to Wallpaper Engine and should not keep it
		if (!desk_state.win32_embed_keep_console) {
			FreeConsole();
		}
	}
#endif

	glfwMakeContextCurrent(desk_state.window);
	glewInit();

	glfwSetWindowSizeCallback(desk_state.window, osd_desktop_resize);
	glfwGetWindowSize(desk_state.window, &states.screen.w, &states.screen.h);
	osd_desktop_resize(desk_state.window, states.screen.w, states.screen.h);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	osd_init();

	double lTime = glfwGetTime(), cTime = lTime, dTime = 0.0;

	while (!glfwWindowShouldClose(desk_state.window)) {

#ifdef WIN32
		if (desk_state.win32_embed) { // Always update to 0,0 when is embedded into Wallpaper Engine
			glfwSetWindowPos(desk_state.window, 0, 0);
		}
#endif

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CGL();
		glBindTexture(GL_TEXTURE_2D, desk_state.test_w); CGL();

		cTime = glfwGetTime();
		dTime = cTime - lTime;
		lTime = cTime;
		osd_draw(dTime);

		glfwSwapBuffers(desk_state.window);
		states.screen.update = false;
		glfwPollEvents();
	}
	return 0;
}