#ifndef SPONZA_SCENE_SHADERS_H
#define SPONZA_SCENE_SHADERS_H

const char vertex_shader_source[] =
        R"(#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out vec3 position;
out vec3 raw_pos;
out vec3 normal;
out vec2 texcoord;

void main()
{
	gl_Position = projection * view * model * vec4(in_position, 1.0);
	position = (model * vec4(in_position, 1.0)).xyz;
	normal = normalize((model * vec4(in_normal, 0.0)).xyz);
    texcoord = vec2(in_texcoord.x, -in_texcoord.y);
    raw_pos = in_position;
}
)";

const char fragment_shader_source[] =
        R"(#version 330 core

uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 albedo;
uniform vec3 camera_position;
uniform vec3 point_light_position[3];
uniform vec3 point_light_color[3];
uniform vec3 point_light_attenuation[3];

uniform bool has_specular_map;
uniform bool has_diffuse_map;
uniform bool has_normal_map;
uniform bool is_reflective;

uniform sampler2D tex;
uniform sampler2D specular_map;
uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform samplerCube cubemap;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform mat4 shadow_transform;
uniform mat4 model;
uniform mat4 view;
uniform sampler2D shadow_map;

in vec3 position;
in vec3 raw_pos;
in vec3 normal;
in vec2 texcoord;

layout (location = 0) out vec4 out_color;

vec3 get_color(int idx, vec3 normal_) {

    vec3 light_vector = point_light_position[idx] - position;
    vec3 point_light_direction = normalize(light_vector);
    float cosine = dot(normal_, point_light_direction);
    float light_factor = max(0.0, cosine);

    vec3 reflected =  2.0 * normal_ * dot(normal_, point_light_direction) - point_light_direction;
    vec4 roughness = vec4(0.0);
    if (has_specular_map) {
        roughness = texture(specular_map, texcoord);
    }
    float specular = pow(max(0.0, dot(reflected, normalize(camera_position - position))), 64.0) * (roughness[0] * roughness[3]);

    float light_distance = length(light_vector);
    float light_intensity = 1.0 / dot(point_light_attenuation[idx], vec3(1.0, light_distance, light_distance * light_distance));

    vec3 color = light_factor * light_intensity * point_light_color[idx] +
                 specular * point_light_color[idx] * vec3(0.5, 0.5, 0.5);
    return color;
}

void main()
{
    vec3 normal_ = normal;
    if (has_normal_map) {
        normal_ = (normalize(texture(normal_map, texcoord) * 2.f - 1.f)).xyz;
        normal_ = normalize((model * vec4(normal_, 0.0)).xyz);
    }

    vec4 shadow_coord = shadow_transform * vec4(position, 1.0);
    shadow_coord /= shadow_coord.w; // perspective divide
    shadow_coord = shadow_coord * 0.5 + vec4(0.5);
    bool in_shadow = texture(shadow_map, shadow_coord.xy).r < shadow_coord.z;

    vec3 reflected = 2.0 * normal_ * dot(normal_, light_direction) - light_direction;
    vec4 roughness = vec4(0.0);
    if (has_specular_map) {
        roughness = texture(specular_map, texcoord);
    }
    float specular_light = pow(max(0.0, dot(reflected, normalize(camera_position - position))), 4.0) * (roughness[0] * roughness[3]);

    vec3 ambient = ambient_color * albedo * texture(tex, texcoord).xyz;
    vec3 diffuse = diffuse_color * light_color * max(0.0, dot(normal_ , light_direction)) * texture(diffuse_map, texcoord).xyz;
    vec3 specular = specular_light * texture(tex, texcoord).xyz;
    vec3 point_light_colors = (get_color(0, normal_) + get_color(1, normal_) + get_color(2, normal_)) * texture(tex, texcoord).xyz;
	vec3 color = ambient;
    if (has_diffuse_map) {
        color = color + diffuse;
    }
    if (has_specular_map) {
        color = color + specular;
    }

    if (in_shadow) {
        color = ambient;
    }

    color = color + point_light_colors;

    out_color = vec4(color, 1.0);
    out_color.a = texture(tex, texcoord).a;

    if (is_reflective) {

        vec3 I      = normalize(camera_position - position);
        vec3 reflect_normal = (model * vec4(normal, 0.0)).xyz;
        vec3 reflection  = -reflect(I, reflect_normal);
        vec3 coords = normalize(reflection);
        color   = texture(cubemap, coords).xyz;
        out_color = vec4(color, 1.0);
    }
}
)";

const char new_vertex_shader_source[] =
        R"(#version 330 core

uniform mat4 model;
uniform mat4 shadow_transform;

layout (location = 0) in vec3 in_position;

void main()
{
	gl_Position = shadow_transform * model * vec4(in_position, 1.0);
}
)";

const char new_fragment_shader_source[] =
        R"(#version 330 core

void main()
{
}
)";


#endif
