#ifndef SPONZA_SCENE_RENDERER_H
#define SPONZA_SCENE_RENDERER_H

#include <fstream>
#include <GL/glew.h>
#include "Program.h"
#include "Parser.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/string_cast.hpp>


struct CameraParams {
    float camera_distance_x, camera_distance_y, camera_distance_z;
    float view_elevation, view_azimuth;
};

class Renderer {
protected:
    std::map<std::string, mtl_object> mtl;
    std::map<std::string, texture> textures;
    std::vector<Object> objects;
    Program program;
    ShadowProgram shadow_program;

public:
    virtual void render() = 0;
};

class SceneRenderer: Renderer {
private:
    glm::mat4 view, projection, model;
    float near, far;
public:
    SceneRenderer(Program program, ShadowProgram shadow_program, std::string mtl_path, std::string obj_path) {
        this->program = program;
        this->shadow_program = shadow_program;
        std::ifstream mtl_file(PRACTICE_SOURCE_DIRECTORY + mtl_path);
        std::tie(mtl, textures) = Parser::load_mtl(mtl_file);
        std::ifstream obj_file(PRACTICE_SOURCE_DIRECTORY + obj_path);
        objects = Parser::load_obj(obj_file, mtl);

        for (Object &object: objects)
            object.load_textures(textures);

        std::vector<Object> objects1, objects2;
        for (Object &obj: objects) {
            if (obj.map_Ka == nullptr || obj.map_Ka->channels != 4)
                objects1.push_back(obj);
            else
                objects2.push_back(obj);
        }
        objects.clear();

        for (Object &obj: objects1)
            objects.push_back(obj);

        for (Object &obj: objects2)
            objects.push_back(obj);

        model = glm::mat4(1.f);

        near = 0.01f;
        far = 10.f;
    }

    void render() override {
        glUniformMatrix4fv(shadow_program.model_location, 1, GL_FALSE, reinterpret_cast<float *>(&model));

        for (Object &object : objects) {
            glUniform3f(program.ambient_color_location, object.mtl.Ka.x, object.mtl.Ka.y, object.mtl.Ka.z);
            glUniform3f(program.diffuse_color_location, object.mtl.Kd.x, object.mtl.Kd.y, object.mtl.Kd.z);

            glUniform1i(program.has_diffuse_map_location, object.has_diffuse_map);
            glUniform1i(program.has_specular_map_location, object.has_specular_map);
            glUniform1i(program.has_normal_map_location, object.has_normal_map);
            glUniform1i(program.is_reflective_location, false);

            object.render();
        }
    }

    void render_cubemap(glm::vec3 translation, GLuint cubemap_texture) {
        glm::mat4 cubemap_perspective = glm::perspective(glm::pi<float>() / 2.f, 1.f, near, far);
        glUniformMatrix4fv(program.projection_location, 1, GL_FALSE, reinterpret_cast<float *>(&cubemap_perspective));

        glm::mat4 shrek_view(1.f);
        std::vector<glm::mat4> shrek_views = {
                get_view(2, 1, 0, -translation),
                get_view(2, 3, 0, -translation),
                get_view(-1, 0, 0, -translation),
                get_view(1, 0, 0, -translation),
                get_view(0, 2, 2, -translation),
                get_view(0, 0, 2, -translation),
        };

        for (int i = 0; i < 6; i++) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap_texture, 0);

            glUniformMatrix4fv(program.view_location, 1, GL_FALSE, reinterpret_cast<float *>(&shrek_views[i]));
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            render();
        }
    }

    glm::mat4 get_view(float x, float y, float z, glm::vec3 translation) {
        glm::mat4 v(1.f);
        v = glm::rotate(v, x * glm::pi<float>() / 2.f, {1.f, 0.f, 0.f});
        v = glm::rotate(v, y * glm::pi<float>() / 2.f, {0.f, 1.f, 0.f});
        v = glm::rotate(v, z * glm::pi<float>() / 2.f, {0.f, 0.f, 1.f});
        v = glm::translate(v, translation);
        return v;
    }

    void reset_params() {
        glUniformMatrix4fv(program.model_location, 1, GL_FALSE, reinterpret_cast<float *>(&model));
        glUniformMatrix4fv(program.view_location, 1, GL_FALSE, reinterpret_cast<float *>(&view));
        glUniformMatrix4fv(program.projection_location, 1, GL_FALSE, reinterpret_cast<float *>(&projection));
    }

    void update_view(CameraParams params) {
        view = glm::mat4(1.f);
        view = glm::translate(view, {params.camera_distance_x, params.camera_distance_y, params.camera_distance_z});
        view = glm::rotate(view, params.view_elevation, {1.f, 0.f, 0.f});
        view = glm::rotate(view, params.view_azimuth, {0.f, 1.f, 0.f});

        glm::vec3 camera_position = (glm::inverse(view) * glm::vec4(0.0, 0.0, 0.0, 1.0));
        glUniform3f(program.camera_location, camera_position.x, camera_position.y, camera_position.z);
    }

    void update_projection(float width, float height) {
        projection = glm::perspective(glm::pi<float>() / 2.f, (1.f * width) / height, near, far);
    }

    void setup_shadows_settings() {
        glm::vec3 light_direction = glm::vec3(0.05f, .7f, 0.05f);

        auto light_Z = -light_direction;
        auto light_X = glm::vec3(light_Z.y, -light_Z.x, 0.0);
        auto light_Y = glm::cross(light_X, light_Z);

        auto shadow_transform = glm::mat4(0.f);
        shadow_transform[0][0] = light_X[0];
        shadow_transform[1][0] = light_X[1];
        shadow_transform[2][0] = light_X[2];
        shadow_transform[0][1] = light_Y[0];
        shadow_transform[1][1] = light_Y[1];
        shadow_transform[2][1] = light_Y[2];
        shadow_transform[0][2] = light_Z[0];
        shadow_transform[1][2] = light_Z[1];
        shadow_transform[2][2] = light_Z[2];
        shadow_transform[3][3] = 1.f;

        glUseProgram(program.program);
        glUniformMatrix4fv(program.shadow_transform_program_location, 1, GL_FALSE, reinterpret_cast<float *>(&shadow_transform));
        glUniform3fv(program.light_direction_location, 1, reinterpret_cast<float *>(&light_direction));
        glUseProgram(shadow_program.program);
        glUniformMatrix4fv(shadow_program.transform_location, 1, GL_FALSE, reinterpret_cast<float *>(&shadow_transform));
    }
};

class ShrekRenderer: Renderer {
private:
    glm::mat4 model;
public:
    glm::vec3 translate;

    ShrekRenderer(Program program, ShadowProgram shadow_program, std::string mtl_path, std::string obj_path) {
        this->program = program;
        this->shadow_program = shadow_program;
        std::ifstream mtl_file(PRACTICE_SOURCE_DIRECTORY + mtl_path);
        std::tie(mtl, textures) = Parser::load_mtl(mtl_file);
        std::ifstream obj_file(PRACTICE_SOURCE_DIRECTORY + obj_path);
        objects = Parser::load_obj(obj_file, mtl, 100);
    }

    void render() override {
        glUniformMatrix4fv(shadow_program.model_location, 1, GL_FALSE, reinterpret_cast<float *>(&model));
        for (Object &object : objects) {
            glUniform1i(program.is_reflective_location, true);
            object.render();
        }
    }

    void reset_params() {
        glUniformMatrix4fv(program.model_location, 1, GL_FALSE, reinterpret_cast<float *>(&model));
    }

    void change_time(float time) {
        model = glm::mat3(1.f);
        translate = {sin(time) * 0.05, 0.1 + cos(time + 2) * 0.02, cos(time + 4) * 0.03};
        model = glm::translate(model, translate);
    }

};



#endif //PRACTICE8_RENDERER_H
