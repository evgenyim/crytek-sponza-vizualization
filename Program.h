#ifndef SPONZA_SCENE_PROGRAM_H
#define SPONZA_SCENE_PROGRAM_H

#include "Shaders.h"


GLuint create_shader(GLenum type, const char * source)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Shader compilation failed: " + info_log);
    }
    return result;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint result = glCreateProgram();
    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragment_shader);
    glLinkProgram(result);

    GLint status;
    glGetProgramiv(result, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetProgramInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Program linkage failed: " + info_log);
    }

    return result;
}

class Program {
public:
    Program() {
        auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
        auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
        program = create_program(vertex_shader, fragment_shader);

        model_location = glGetUniformLocation(program, "model");
        view_location = glGetUniformLocation(program, "view");
        projection_location = glGetUniformLocation(program, "projection");
        has_diffuse_map_location = glGetUniformLocation(program, "has_diffuse_map");
        has_specular_map_location = glGetUniformLocation(program, "has_specular_map");
        has_normal_map_location = glGetUniformLocation(program, "has_normal_map");
        is_reflective_location = glGetUniformLocation(program, "is_reflective");
        texture_location = glGetUniformLocation(program, "tex");
        diffuse_map_location = glGetUniformLocation(program, "diffuse_map");
        specular_map_location = glGetUniformLocation(program, "specular_map");
        normal_map_location = glGetUniformLocation(program, "normal_map");
        cubemap_location = glGetUniformLocation(program, "cubemap");

        ambient_color_location = glGetUniformLocation(program, "ambient_color");
        diffuse_color_location = glGetUniformLocation(program, "diffuse_color");
        albedo_location = glGetUniformLocation(program, "albedo");
        camera_location = glGetUniformLocation(program, "camera_position");

        light_direction_location = glGetUniformLocation(program, "light_direction");
        light_color_location = glGetUniformLocation(program, "light_color");
        shadow_map_program_location = glGetUniformLocation(program, "shadow_map");
        shadow_transform_program_location = glGetUniformLocation(program, "shadow_transform");

        point_light_position_location0 = glGetUniformLocation(program, "point_light_position[0]");
        point_light_color_location0 = glGetUniformLocation(program, "point_light_color[0]");
        point_light_attenuation_location0 = glGetUniformLocation(program, "point_light_attenuation[0]");
        point_light_position_location1 = glGetUniformLocation(program, "point_light_position[1]");
        point_light_color_location1 = glGetUniformLocation(program, "point_light_color[1]");
        point_light_attenuation_location1 = glGetUniformLocation(program, "point_light_attenuation[1]");
        point_light_position_location2 = glGetUniformLocation(program, "point_light_position[2]");
        point_light_color_location2 = glGetUniformLocation(program, "point_light_color[2]");
        point_light_attenuation_location2 = glGetUniformLocation(program, "point_light_attenuation[2]");

    }

    void setup_textures() {
        glUseProgram(program);
        glUniform1i(shadow_map_program_location, 0);
        glUniform1i(texture_location, 1);
        glUniform1i(diffuse_map_location, 2);
        glUniform1i(specular_map_location, 3);
        glUniform1i(normal_map_location, 4);
        glUniform1i(cubemap_location, 5);
    }

    void setup_lights() {
        glUniform3f(point_light_color_location0, .3, .3, 1.);
        glUniform3f(point_light_attenuation_location0, 1.0, 13.0, 5.0);

        glUniform3f(point_light_color_location1, 1, .3, .3);
        glUniform3f(point_light_attenuation_location1, 1.0, 13.0, 5.0);

        glUniform3f(point_light_color_location2, .3, 1, .3);
        glUniform3f(point_light_attenuation_location2, 1.0, 13.0, 5.0);

        glUniform3f(albedo_location, .3f, .3f, .3f);

        glUniform3f(point_light_position_location0, -.65f, .2f, .3f);
        glUniform3f(point_light_position_location1, .65f, .2f, .3f);
        glUniform3f(point_light_position_location2, .65f, .2f, -.3f);

        glUniform3f(light_color_location, 0.8f, 0.8f, 0.8f);
    }

    GLint model_location, view_location, projection_location, has_diffuse_map_location, has_specular_map_location,
            has_normal_map_location, is_reflective_location, texture_location, diffuse_map_location,
            specular_map_location, normal_map_location, cubemap_location, ambient_color_location, diffuse_color_location,
            albedo_location, camera_location, light_direction_location, light_color_location, shadow_map_program_location,
            shadow_transform_program_location, point_light_position_location0, point_light_color_location0,
            point_light_attenuation_location0, point_light_position_location1, point_light_color_location1,
            point_light_attenuation_location1, point_light_position_location2, point_light_color_location2, point_light_attenuation_location2;
    GLuint program;
};

class ShadowProgram {
public:
    GLuint program;
    GLint transform_location, model_location;

    ShadowProgram() {
        auto new_vertex_shader = create_shader(GL_VERTEX_SHADER, new_vertex_shader_source);
        auto new_fragment_shader = create_shader(GL_FRAGMENT_SHADER, new_fragment_shader_source);
        program = create_program(new_vertex_shader, new_fragment_shader);

        transform_location = glGetUniformLocation(program, "shadow_transform");
        model_location = glGetUniformLocation(program, "model");
    }
};


#endif
