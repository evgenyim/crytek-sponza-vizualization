#ifndef SPONZA_SCENE_PARSER_H
#define SPONZA_SCENE_PARSER_H


#include <set>

class Parser {
public:
    static std::pair<std::map<std::string, mtl_object>, std::map<std::string, texture>> load_mtl(std::istream &input) {
        mtl_object obj;
        obj.name = "__";

        std::map<std::string, mtl_object> m;
        std::set<std::string> paths;

        for (std::string line; std::getline(input, line);) {
            std::istringstream line_stream(line);

            std::string type;
            line_stream >> type;

            if (type == "newmtl") {
                if (obj.name != "__")
                    m.insert({obj.name, obj});
                obj.clear();
                line_stream >> obj.name;
                continue;
            }

            if (type == "Ns") {
                line_stream >> obj.Ns;
                continue;
            }

            if (type == "Ni") {
                line_stream >> obj.Ni;
                continue;
            }

            if (type == "d") {
                line_stream >> obj.d;
                continue;
            }

            if (type == "Tr") {
                line_stream >> obj.Tr;
                continue;
            }

            if (type == "Tf") {
                line_stream >> obj.Tf.x >> obj.Tf.y >> obj.Tf.z;
                continue;
            }

            if (type == "illum") {
                line_stream >> obj.illum;
                continue;
            }

            if (type == "Ka") {
                line_stream >> obj.Ka.x >> obj.Ka.y >> obj.Ka.z;
                continue;
            }

            if (type == "Kd") {
                line_stream >> obj.Kd.x >> obj.Kd.y >> obj.Kd.z;
                continue;
            }

            if (type == "Ks") {
                line_stream >> obj.Ks.x >> obj.Ks.y >> obj.Ks.z;
                continue;
            }

            if (type == "Tf") {
                line_stream >> obj.Ke.x >> obj.Ke.y >> obj.Ke.z;
                continue;
            }

            if (type == "map_Ka") {
                line_stream >> obj.map_Ka;
                paths.insert(obj.map_Ka);
                continue;
            }

            if (type == "map_Kd") {
                line_stream >> obj.map_Kd;
                paths.insert(obj.map_Kd);
                continue;
            }

            if (type == "map_Ks") {
                line_stream >> obj.map_Ks;
                paths.insert(obj.map_Ks);
                continue;
            }

            if (type == "norm") {
                line_stream >> obj.norm;
                paths.insert(obj.norm);
                continue;
            }

            if (type == "map_bump" || type == "map_d") {
                continue;
            }
        }

        std::map<std::string, texture> textures;
        for (auto &path: paths) {
            std::string filename = (PRACTICE_SOURCE_DIRECTORY "/sponza/" + std::regex_replace(path, std::regex("\\\\"), "/"));
            texture t;
            unsigned char *pixels = stbi_load(filename.c_str(),
                                              &t.width, &t.height, &t.channels, 0);
            t.data.resize(t.width * t.height * t.channels);
            t.data.assign(pixels, pixels + t.width * t.height * t.channels);
            textures.insert({path, t});
        }
        m.insert({obj.name, obj});

        return {m, textures};
    };

    static std::vector<Object> load_obj(std::istream & input, std::map<std::string, mtl_object> &m, float scale_factor = 1500)
    {
        std::vector<vertex> vertices;
        std::vector<vertex> vertices_normals;
        std::vector<vertex> vertices_texture_coords;
        std::vector<std::uint32_t> indices;
        std::vector<std::uint32_t> indices_normals;
        std::vector<std::uint32_t> indices_texture_coords;
        std::vector<Object> objects;
        mtl_object cur_mtl;

        for (std::string line; std::getline(input, line);)
        {
            std::istringstream line_stream(line);

            std::string type;
            line_stream >> type;

            if (type == "#")
                continue;

            if (type == "v")
            {
                vertex v;
                line_stream >> v.position.x >> v.position.y >> v.position.z;
                v.position.x /= scale_factor;
                v.position.y /= scale_factor;
                v.position.z /= scale_factor;
                vertices.push_back(v);
                continue;
            }

            if (type == "vn"){
                vertex v;
                line_stream >> v.position.x >> v.position.y >> v.position.z;
                vertices_normals.push_back(v);
                continue;
            }

            if (type == "vt"){
                vertex v;
                line_stream >> v.position.x >> v.position.y >> v.position.z;
                vertices_texture_coords.push_back(v);
                continue;
            }

            if (type == "s" || type.empty() || type == "g" || type == "mtllib" || type == "o" || type == "l")
                continue;

            if (type == "usemtl") {
                std::string mtl_name;
                line_stream >> mtl_name;
                if (indices.empty()) {
                    cur_mtl = m[mtl_name];
                    continue;
                }
                std::vector<vertex> cur_vertices;
                std::vector<vertex> cur_vertices_normals;
                std::vector<vertex> cur_vertices_texture_coords;
                std::vector<std::uint32_t> cur_indices;
                std::vector<std::uint32_t> cur_indices_normals;
                std::vector<std::uint32_t> cur_indices_texture_coords;

                std::uint32_t cur = 0;
                std::map<std::tuple<int, int, int>, std::uint32_t> check_map;

                for (int i = 0; i < indices.size(); i++) {
                    if (check_map.contains({indices[i], indices_normals[i], indices_texture_coords[i]}))
                        cur_indices.push_back(check_map[{indices[i], indices_normals[i], indices_texture_coords[i]}]);
                    else {
                        glm::vec2 texcoords = {0.0, 0.0};
                        if (indices_texture_coords[i] != -1)
                            texcoords = {vertices_texture_coords[indices_texture_coords[i]].position.x,
                                         vertices_texture_coords[indices_texture_coords[i]].position.y};
                        cur_vertices.push_back({vertices[indices[i]].position,
                                                vertices_normals[indices_normals[i]].position,
                                                {texcoords.x, texcoords.y}
                                               });
                        check_map[{indices[i], indices_normals[i], indices_texture_coords[i]}] = cur;
                        cur_indices.push_back(cur++);
                    }
                }

                Object o(cur_vertices, cur_indices, cur_mtl);
                objects.push_back(o);
                cur_mtl = m[mtl_name];
                indices.clear();
                indices_normals.clear();
                indices_texture_coords.clear();
                continue;
            }

            if (type == "f")
            {
                std::uint32_t i0, i1, i2;
                std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>> cur_indices;
                char c;
                std::vector<std::uint32_t> is;
                std::string s;
                while(line_stream >> s) {
                    std::stringstream ss(s);
                    if (s.find("//") != std::string::npos) {
                        ss >> i0 >> c >> c >> i2;
                        i1 = 0;
                    }
                    else
                        ss >> i0 >> c >> i1 >> c >> i2;
                    cur_indices.push_back({--i0, --i1, --i2});
                }

                for (int i = 1; i < cur_indices.size() - 1; i++) {
                    indices.push_back(std::get<0>(cur_indices[0]));
                    indices.push_back(std::get<0>(cur_indices[i]));
                    indices.push_back(std::get<0>(cur_indices[i + 1]));
                    indices_texture_coords.push_back(std::get<1>(cur_indices[0]));
                    indices_texture_coords.push_back(std::get<1>(cur_indices[i]));
                    indices_texture_coords.push_back(std::get<1>(cur_indices[i + 1]));
                    indices_normals.push_back(std::get<2>(cur_indices[0]));
                    indices_normals.push_back(std::get<2>(cur_indices[i]));
                    indices_normals.push_back(std::get<2>(cur_indices[i + 1]));
                }
                continue;
            }

            throw std::runtime_error("Unknown OBJ row type: " + type);
        }
        std::vector<vertex> cur_vertices;
        std::vector<vertex> cur_vertices_normals;
        std::vector<vertex> cur_vertices_texture_coords;
        std::vector<std::uint32_t> cur_indices;
        std::vector<std::uint32_t> cur_indices_normals;
        std::vector<std::uint32_t> cur_indices_texture_coords;

        std::uint32_t cur = 0;

        for (int i = 0; i < indices.size(); i++) {
            cur_vertices.push_back({vertices[indices[i]].position,
                                    vertices_normals[indices_normals[i]].position,
                                    {vertices_texture_coords[indices_texture_coords[i]].position.x,
                                     vertices_texture_coords[indices_texture_coords[i]].position.y}});
            cur_indices.push_back(cur++);
        }

        Object o(cur_vertices, cur_indices, cur_mtl);
        objects.push_back(o);

        std::cout << "Objects: " << objects.size() << std::endl;
        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Normals: " << vertices_normals.size() << std::endl;
        std::cout << "Texture Coords: " << vertices_texture_coords.size() << std::endl;

        return objects;
    }

};


#endif
