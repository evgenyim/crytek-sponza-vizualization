#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <cmath>
#include <Object.h>
#include <Renderer.h>
#include <Program.h>
#include <RenderSetuper.h>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>

std::string to_string(std::string_view str)
{
	return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message)
{
	throw std::runtime_error(to_string(message) + SDL_GetError());
}

void glew_fail(std::string_view message, GLenum error)
{
	throw std::runtime_error(to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
}

int main() try
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		sdl2_fail("SDL_Init: ");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window * window = SDL_CreateWindow("Graphics course homework 2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

	if (!window)
		sdl2_fail("SDL_CreateWindow: ");

	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (!gl_context)
		sdl2_fail("SDL_GL_CreateContext: ");

	if (auto result = glewInit(); result != GLEW_NO_ERROR)
		glew_fail("glewInit: ", result);

	if (!GLEW_VERSION_3_3)
		throw std::runtime_error("OpenGL 3.3 is not supported");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    Program p;
    p.setup_textures();
    p.setup_lights();

    ShadowProgram shadow_program;

    SceneRenderer scene_renderer(p, shadow_program, "/sponza/sponza.mtl", "/sponza/sponza.obj");
    ShrekRenderer shrek_renderer(p, shadow_program, "/shrek/shrek.mtl", "/shrek/shrek.obj");

    scene_renderer.setup_shadows_settings();

    RenderSetuper render_setuper(p, shadow_program);
    render_setuper.update_window_size(width, height);


    if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer error");
    }

	auto last_frame_start = std::chrono::high_resolution_clock::now();

	float time = 0.f;

	std::map<SDL_Keycode, bool> button_down;

    CameraParams camera_params;

	camera_params.view_azimuth = 0.f;
    camera_params.view_elevation = glm::radians(30.f);
    camera_params.camera_distance_x = 0.0f;
	camera_params.camera_distance_y = -0.5f;
	camera_params.camera_distance_z = 0.0f;
	bool running = true;
	bool paused = false;
	while (running)
	{
		for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		case SDL_WINDOWEVENT: switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				width = event.window.data1;
				height = event.window.data2;
                render_setuper.update_window_size(width, height);
                glViewport(0, 0, width, height);
				break;
			}
			break;
		case SDL_KEYDOWN:
			button_down[event.key.keysym.sym] = true;
			break;
		case SDL_KEYUP:
			button_down[event.key.keysym.sym] = false;
			break;
		}

		if (!running)
			break;

		auto now = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
		last_frame_start = now;
        if (!paused)
		    time += dt;

		if (button_down[SDLK_UP])
			camera_params.camera_distance_y -= 1.f * dt;
		if (button_down[SDLK_DOWN])
			camera_params.camera_distance_y += 1.f * dt;
        if (button_down[SDLK_w])
            camera_params.camera_distance_z += 1.f * dt;
        if (button_down[SDLK_s])
            camera_params.camera_distance_z -= 1.f * dt;
        if (button_down[SDLK_a])
            camera_params.camera_distance_x += 1.f * dt;
        if (button_down[SDLK_d])
            camera_params.camera_distance_x -= 1.f * dt;

		if (button_down[SDLK_LEFT])
			camera_params.view_azimuth -= 2.f * dt;
		if (button_down[SDLK_RIGHT])
			camera_params.view_azimuth += 2.f * dt;

        if (button_down[SDLK_SPACE]) {
            button_down[SDLK_SPACE] = false;
            paused = !paused;
        }

        if (button_down[SDLK_q])
            running = false;

		glClearColor(0.8f, 0.8f, 0.9f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shrek_renderer.change_time(time);
        scene_renderer.update_view(camera_params);
        scene_renderer.update_projection(width, height);

        render_setuper.setup_shadow_render();

        scene_renderer.render();
        shrek_renderer.render();

        render_setuper.setup_cubemap_render();
        scene_renderer.render_cubemap(shrek_renderer.translate, render_setuper.cubemap_texture);

        render_setuper.setup_render();

        scene_renderer.reset_params();
        scene_renderer.render();

        shrek_renderer.reset_params();
        shrek_renderer.render();

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
}
catch (std::exception const & e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
