#ifndef SPONZA_SCENE_RENDERSETUPER_H
#define SPONZA_SCENE_RENDERSETUPER_H


class RenderSetuper {
private:
    Program program;
    ShadowProgram shadow_program;
    int shadow_map_res, cubemap_res, width, height;
    GLuint shadow_texture, cubemap_framebuffer, frame_buffer;

public:
    GLuint cubemap_texture;

    RenderSetuper(Program program, ShadowProgram shadow_program) {
        this->program = program;
        this->shadow_program = shadow_program;
        shadow_map_res = 4096;
        glGenTextures(1, &shadow_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_map_res, shadow_map_res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture, 0);

        cubemap_res = 1024;

        glGenTextures(1, &cubemap_texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        for(int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                         cubemap_res, cubemap_res, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        glGenFramebuffers(1, &cubemap_framebuffer);
        GLuint cubemap_renderbuffer;
        glGenRenderbuffers(1, &cubemap_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, cubemap_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemap_res, cubemap_res);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubemap_framebuffer);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cubemap_renderbuffer);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 5);
    }

    void setup_shadow_render() {
        glUseProgram(shadow_program.program);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        glViewport(0, 0, shadow_map_res, shadow_map_res);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
    }

    void setup_cubemap_render() {
        glUseProgram(program.program);
        glCullFace(GL_BACK);

        glViewport(0, 0, cubemap_res, cubemap_res);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_texture);
        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubemap_framebuffer);
    }

    void setup_render() {

        glViewport(0, 0, width, height);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_texture);

        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    }

    void update_window_size(int width, int height) {
        this->width = width;
        this->height = height;
    }
};


#endif
