#include <vector>
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <regex>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/vec2.hpp>


#ifndef SPONZA_SCENE_OBJECT_H
#define SPONZA_SCENE_OBJECT_H

struct vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct mtl_object {
    float Ns, Ni, d, Tr;
    int illum;
    glm::vec3 Ka, Kd, Ks, Ke, Tf;
    std::string name, map_Ka, map_Kd, map_Ks, norm;

    void clear() {
        Ns = Ni = d = Tr = illum = 0;
        map_Ka = map_Ks = norm = "";
        Tf.x = Tf.y = Tf.z = 0;
        Ks.x = Ks.y = Ks.z = 0;
        Ke.x = Ke.y = Ke.z = 0;
        Ka.x = Ka.y = Ka.z = 0;
        Kd.x = Kd.y = Kd.z = 0;    }
};

struct texture {
    int width = 0, height = 0, channels = 0;
    std::vector<unsigned char> data;
};

class Object {
public:
    std::vector<vertex> vertices;
    std::vector<std::uint32_t> indices;
    mtl_object mtl;
    texture *map_Ka, *map_Ks, *map_Kd, *norm;
    GLuint vao, vbo, ebo, tex, specular_map, diffuse_map, normal_map;
    bool has_specular_map = false;
    bool has_diffuse_map = false;
    bool has_normal_map = false;
    bool has_texture = false;

    Object(std::vector<vertex> vertices, std::vector<std::uint32_t> indices, mtl_object mtl) {
        this->vertices = vertices;
        this->indices = indices;
        this->mtl = mtl;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(vertex), this->vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(this->indices[0]), this->indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(12));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(24));

    }

    void load_textures(std::map<std::string, texture> &textures) {
        if (mtl.map_Ka != "") {
            this->map_Ka = &textures[mtl.map_Ka];
            has_texture = true;
        }
        if (mtl.map_Ks != "") {
            this->map_Ks = &textures[mtl.map_Ks];
            has_specular_map = true;
        }
        if (mtl.map_Kd != "") {
            this->map_Kd = &textures[mtl.map_Kd];
            has_diffuse_map = true;
        }
        if (mtl.norm != "") {
            this->norm = &textures[mtl.norm];
            has_normal_map = true;
        }
        if (has_texture)
            load_texture(tex, map_Ka);
        else
            load_texture(tex, reinterpret_cast<texture *&>(textures.begin()->second));
        if (has_specular_map)
            load_texture(specular_map, map_Ks);
        if (has_diffuse_map)
            load_texture(diffuse_map, map_Kd);
        if (has_normal_map)
            load_texture(normal_map, norm);
    }

    void load_texture(GLuint &t, texture* &tex_src) {
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        GLenum format;
        GLint internal_format;
        if (tex_src->channels == 4) {
            format = GL_RGBA;
            internal_format = GL_RGBA8;
        }
        if (tex_src->channels == 3) {
            format = GL_RGB;
            internal_format = GL_RGB8;
        }
        if (tex_src->channels == 2) {
            format = GL_RG;
            internal_format = GL_RG8;
        }
        if (tex_src->channels == 1) {
            format = GL_DEPTH_COMPONENT;
            internal_format = GL_DEPTH_COMPONENT24;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, tex_src->width, tex_src->height, 0,
                     format, GL_UNSIGNED_BYTE, tex_src->data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void render() {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, tex);

        if (has_diffuse_map) {
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, diffuse_map);
        }
        if (has_specular_map) {
            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_2D, specular_map);
        }
        if (has_normal_map) {
            glActiveTexture(GL_TEXTURE0 + 4);
            glBindTexture(GL_TEXTURE_2D, normal_map);
        }

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    }
};


#endif
