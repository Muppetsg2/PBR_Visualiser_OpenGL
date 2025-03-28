#include <ModelLoader.h>

std::vector<Vertex> ModelLoader::_vertices = std::vector<Vertex>();
std::vector<unsigned int> ModelLoader::_indices = std::vector<unsigned int>();

bool ModelLoader::_init = false;
bool ModelLoader::_hasIndices = false;
bool ModelLoader::_updateEBO = false;

std::string ModelLoader::_fileName = "";
ModelFormat ModelLoader::_format = ModelFormat::NONE;

GLuint ModelLoader::_vbo = 0;
GLuint ModelLoader::_ebo = 0;

std::pair<glm::vec3, glm::vec3> ModelLoader::CalcTangentBitangent(size_t t1, size_t t2, size_t t3)
{
    std::pair<glm::vec3, glm::vec3> TB;

    Vertex v0 = ModelLoader::_vertices[t1];
    Vertex v1 = ModelLoader::_vertices[t2];
    Vertex v2 = ModelLoader::_vertices[t3];

    glm::vec3 pos0 = v0.Position;
    glm::vec3 pos1 = v1.Position;
    glm::vec3 pos2 = v2.Position;

    glm::vec2 uv0 = v0.TexCoords;
    glm::vec2 uv1 = v1.TexCoords;
    glm::vec2 uv2 = v2.TexCoords;

    glm::vec3 delta_pos1 = pos1 - pos0;
    glm::vec3 delta_pos2 = pos2 - pos0;

    glm::vec2 delta_uv1 = uv1 - uv0;
    glm::vec2 delta_uv2 = uv2 - uv0;

    float d = delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x;
    float r = d != 0.0f ? 1.0f / d : 1.0f;

    // Save the results
    TB.first = (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r;
    TB.second = (delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x) * r;

    return TB;
}

bool ModelLoader::LoadModelFromGLTF(std::string path)
{
    cgltf_options options = {};
    cgltf_data* data = nullptr;

    // Parse the GLTF/GLB file
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);
    if (result != cgltf_result_success) {
        spdlog::error("Failed to parse GLTF/GLB file: {}", path);
        return false;
    }

    // Load the data into memory
    result = cgltf_load_buffers(&options, data, path.c_str());
    if (result != cgltf_result_success) {
        spdlog::error("Failed to load buffers for file: {}", path);
        cgltf_free(data);
        return false;
    }

    bool hasTangent = false;
    bool hasNormal = false;
    bool hasTexCoords = false;

    // Process meshes
    for (size_t i = 0; i < data->meshes_count; ++i) {
        const cgltf_mesh& mesh = data->meshes[i];

        // Process primitives
        for (size_t j = 0; j < mesh.primitives_count; ++j) {
            const cgltf_primitive& primitive = mesh.primitives[j];

            if (primitive.type != cgltf_primitive_type_triangles) {
                spdlog::error("Unsupported primitive type in GLTF mesh: only triangles are supported.");
                continue;
            }

            // Process attributes
            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<float> tangents;
            std::vector<float> texcoords;

            for (size_t k = 0; k < primitive.attributes_count; ++k) {
                const cgltf_attribute& attribute = primitive.attributes[k];
                const cgltf_accessor* accessor = attribute.data;

                switch (attribute.type) {
                    case cgltf_attribute_type_position: {
                        positions.resize(accessor->count * 3);
                        cgltf_accessor_unpack_floats(accessor, positions.data(), positions.size());
                        break;
                    }                        
                    case cgltf_attribute_type_normal: {
                        normals.resize(accessor->count * 3);
                        cgltf_accessor_unpack_floats(accessor, normals.data(), normals.size());
                        break;
                    }
                    case cgltf_attribute_type_texcoord: {
                        texcoords.resize(accessor->count * 2);
                        cgltf_accessor_unpack_floats(accessor, texcoords.data(), texcoords.size());
                        break;
                    }
                    case cgltf_attribute_type_tangent: {
                        tangents.resize(accessor->count * 4);
                        cgltf_accessor_unpack_floats(accessor, tangents.data(), tangents.size());
                        break;
                    }
                    default:
                        break;
                }
            }

            // Process indices
            if (primitive.indices) {
                const cgltf_accessor* index_accessor = primitive.indices;
                auto maxIterator = std::max_element(ModelLoader::_indices.begin(), ModelLoader::_indices.end());

                int32_t maxValue = maxIterator != ModelLoader::_indices.end() ? *maxIterator + 1 : 0;

                for (size_t k = 0; k < index_accessor->count; ++k) {
                    ModelLoader::_indices.push_back(maxValue + static_cast<uint32_t>(cgltf_accessor_read_index(index_accessor, k)));
                }
            }

            // Construct vertices
            size_t vertex_count = positions.size() / 3;
            for (size_t k = 0; k < vertex_count; ++k) {
                Vertex vertex;

                vertex.Position = glm::vec3(
                    positions[k * 3 + 0],
                    positions[k * 3 + 1],
                    positions[k * 3 + 2]
                );

                if (!normals.empty()) {
                    hasNormal = true;
                    vertex.Normal = glm::vec3(
                        normals[k * 3 + 0],
                        normals[k * 3 + 1],
                        normals[k * 3 + 2]
                    );
                }

                if (!texcoords.empty()) {
                    hasTexCoords = true;
                    vertex.TexCoords = glm::vec2(
                        texcoords[k * 2 + 0],
                        texcoords[k * 2 + 1]
                    );
                }

                if (!tangents.empty()) {
                    hasTangent = true;
                    vertex.Tangent = glm::vec3(
                        tangents[k * 4 + 0],
                        tangents[k * 4 + 1],
                        tangents[k * 4 + 2]
                    );

                    // Compute bitangent if needed
                    float bitangentSign = tangents[k * 4 + 3];
                    vertex.Tangent = glm::normalize(vertex.Tangent);
                    vertex.Bitangent = glm::normalize(glm::cross(vertex.Normal, vertex.Tangent)) * bitangentSign;
                }
                else {
                    vertex.Tangent = glm::vec3(0.0f);
                    vertex.Bitangent = glm::vec3(0.0f);
                }

                ModelLoader::_vertices.push_back(vertex);
            }
        }
    }

    if (ModelLoader::_vertices.empty()) {
        spdlog::error("Neither mesh had a triangular mesh or the loaded GLTF file did not have any vertices.");
        ModelLoader::_vertices.clear();
        ModelLoader::_indices.clear();
        cgltf_free(data);
        return false;
    }

    if (!hasNormal || !hasTexCoords) {
        spdlog::error("Loaded GLTF file did not have any information about Normal or TexCoords or both in vertices.");
        ModelLoader::_vertices.clear();
        ModelLoader::_indices.clear();
        cgltf_free(data);
        return false;
    }

    ModelLoader::_hasIndices = !ModelLoader::_indices.empty();
    ModelLoader::_updateEBO = true;

    if (ModelLoader::_hasIndices && !hasTangent) {

        std::vector<size_t> trisCount;
        trisCount.resize(ModelLoader::_vertices.size());

        for (size_t i = 0; i < ModelLoader::_indices.size(); i += 3) {

            size_t v1 = ModelLoader::_indices[i];
            size_t v2 = ModelLoader::_indices[i + 1];
            size_t v3 = ModelLoader::_indices[i + 2];

            std::pair<glm::vec3, glm::vec3> TB = ModelLoader::CalcTangentBitangent(v1, v2, v3);

            ModelLoader::_vertices[v1].Tangent += TB.first;
            ModelLoader::_vertices[v1].Bitangent += TB.second;
            ModelLoader::_vertices[v2].Tangent += TB.first;
            ModelLoader::_vertices[v2].Bitangent += TB.second;
            ModelLoader::_vertices[v3].Tangent += TB.first;
            ModelLoader::_vertices[v3].Bitangent += TB.second;

            trisCount[v1] += 1;
            trisCount[v2] += 1;
            trisCount[v3] += 1;
        }

        for (size_t i = 0; i < trisCount.size(); ++i) {

            if (ModelLoader::_vertices[i].Tangent == glm::vec3(0.0f)) {

                glm::vec3 U;
                if (glm::abs(ModelLoader::_vertices[i].Normal.x) > 0.9f) {
                    U = glm::vec3(0.f, 1.f, 0.f);
                }
                else if (glm::abs(ModelLoader::_vertices[i].Normal.y) > 0.9f) {
                    U = glm::vec3(0.f, 0.f, 1.f);
                }
                else {
                    U = glm::vec3(1.f, 0.f, 0.f);
                }

                ModelLoader::_vertices[i].Tangent = glm::normalize(glm::cross(U, ModelLoader::_vertices[i].Normal));
                ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
            }
            else if (ModelLoader::_vertices[i].Bitangent == glm::vec3(0.0f)) {
                ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
                ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
            }
            else {
                ModelLoader::_vertices[i].Tangent /= (float)trisCount[i];
                ModelLoader::_vertices[i].Bitangent /= (float)trisCount[i];

                ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
                ModelLoader::_vertices[i].Bitangent = glm::normalize(ModelLoader::_vertices[i].Bitangent);
            }
        }

        trisCount.clear();
    }
    else if (!ModelLoader::_hasIndices && !hasTangent) {
        for (size_t v = 0; v < ModelLoader::_vertices.size(); v += 3) {
            std::pair<glm::vec3, glm::vec3> TB = ModelLoader::CalcTangentBitangent(v, v + 1, v + 2);

            if (TB.first == glm::vec3(0.0f)) {

                for (size_t z = 0; z < 3; ++z) {
                    glm::vec3 U;
                    if (glm::abs(ModelLoader::_vertices[v + z].Normal.x) > 0.9f) {
                        U = glm::vec3(0.f, 1.f, 0.f);
                    }
                    else if (glm::abs(ModelLoader::_vertices[v + z].Normal.y) > 0.9f) {
                        U = glm::vec3(0.f, 0.f, 1.f);
                    }
                    else {
                        U = glm::vec3(1.f, 0.f, 0.f);
                    }

                    ModelLoader::_vertices[v + z].Tangent = glm::normalize(glm::cross(U, ModelLoader::_vertices[v + z].Normal));
                    ModelLoader::_vertices[v + z].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[v + z].Normal, ModelLoader::_vertices[v + z].Tangent));
                }
            }
            else if (TB.second == glm::vec3(0.0f)) {
                for (size_t z = 0; z < 3; ++z) {
                    ModelLoader::_vertices[v + z].Tangent = glm::normalize(ModelLoader::_vertices[v + z].Tangent);
                    ModelLoader::_vertices[v + z].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[v + z].Normal, ModelLoader::_vertices[v + z].Tangent));
                }
            }
            else {
                ModelLoader::_vertices[v].Tangent = glm::normalize(TB.first);
                ModelLoader::_vertices[v].Bitangent = glm::normalize(TB.second);
                ModelLoader::_vertices[v + 1].Tangent = glm::normalize(TB.first);
                ModelLoader::_vertices[v + 1].Bitangent = glm::normalize(TB.second);
                ModelLoader::_vertices[v + 2].Tangent = glm::normalize(TB.first);
                ModelLoader::_vertices[v + 2].Bitangent = glm::normalize(TB.second);
            }
        }
    }

    // Clean memory
    cgltf_free(data);
    return true;
}

bool ModelLoader::LoadModelFromOBJ(std::string path)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;

    config.triangulate = false;
    //config.triangulation_method = "earcut";

    if (!reader.ParseFromFile(path, config)) {
        if (!reader.Error().empty()) {
            spdlog::error("Error loading OBJ file: {}", reader.Error());
        }
        else {
            spdlog::error("Failed to load OBJ file: {}", path);
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        spdlog::warn("Warning when loading an OBJ file: {}", reader.Warning());
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

    std::vector<std::tuple<int, int, int>> vertIndexes;
    // Process shapes
    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        bool isShapeValid = true;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            if (shape.mesh.num_face_vertices[f] != 3) {
                spdlog::error("Unsupported number of vertices per face in OBJ mesh: only triangles are supported.");
                isShapeValid = false;
                break;
            }
        }

        if (!isShapeValid) continue;

        for (const auto& index : shape.mesh.indices) {

            std::tuple<int, int, int> tup = std::tuple<int, int, int>(index.vertex_index, index.normal_index, index.texcoord_index);

            if (!vertIndexes.empty()) {
                auto it = std::find_if(vertIndexes.begin(), vertIndexes.end(), 
                    [&tup](const std::tuple<int, int, int>& item) {
                        return std::get<0>(item) == std::get<0>(tup) && std::get<1>(item) == std::get<1>(tup) && std::get<2>(item) == std::get<2>(tup);
                    });
                if (it != vertIndexes.end()) {
                    size_t index = std::distance(vertIndexes.begin(), it);
                    _indices.push_back(index);
                    continue;
                }
            }

            vertIndexes.push_back(tup);

            ModelLoader::_indices.push_back(vertIndexes.size() - 1);
        }
    }

    bool hasNormal = false;
    bool hasTexCoords = false;

    // Make vertices
    for (const auto& index : vertIndexes) {
        Vertex v;

        // Position (x, y, z)
        int off = std::get<0>(index);
        v.Position = glm::vec3(
            attrib.vertices[3 * off + 0],
            attrib.vertices[3 * off + 1],
            attrib.vertices[3 * off + 2]
        );

        // Normal (if exist)
        off = std::get<1>(index);
        if (off >= 0) {
            hasNormal = true;
            v.Normal = glm::vec3(
                attrib.normals[3 * off + 0],
                attrib.normals[3 * off + 1],
                attrib.normals[3 * off + 2]
            );
        }

        // TexCoords (if exist)
        off = std::get<2>(index);
        if (off >= 0) {
            hasTexCoords = true;
            v.TexCoords = glm::vec2(
                attrib.texcoords[2 * off + 0],
                attrib.texcoords[2 * off + 1]
            );
        }

        v.Tangent = glm::vec3(0.0f);
        v.Bitangent = glm::vec3(0.0f);

        ModelLoader::_vertices.push_back(v);
    }

    vertIndexes.clear();

    if (ModelLoader::_vertices.empty()) {
        spdlog::error("Neither mesh had a triangular mesh or the loaded OBJ file did not have any vertices.");
        ModelLoader::_vertices.clear();
        ModelLoader::_indices.clear();
        return false;
    }

    if (!hasNormal || !hasTexCoords) {
        spdlog::error("Loaded OBJ file did not have any information about Normal or TexCoords or both in vertices.");
        ModelLoader::_vertices.clear();
        ModelLoader::_indices.clear();
        return false;
    }

    // Calculate Tangent and Bitangent (using indices)
    std::vector<size_t> trisCount(ModelLoader::_vertices.size(), 0);

    for (size_t i = 0; i < ModelLoader::_indices.size(); i += 3) {
        size_t v1 = ModelLoader::_indices[i];
        size_t v2 = ModelLoader::_indices[i + 1];
        size_t v3 = ModelLoader::_indices[i + 2];

        std::pair<glm::vec3, glm::vec3> TB = ModelLoader::CalcTangentBitangent(v1, v2, v3);

        ModelLoader::_vertices[v1].Tangent += TB.first;
        ModelLoader::_vertices[v1].Bitangent += TB.second;
        ModelLoader::_vertices[v2].Tangent += TB.first;
        ModelLoader::_vertices[v2].Bitangent += TB.second;
        ModelLoader::_vertices[v3].Tangent += TB.first;
        ModelLoader::_vertices[v3].Bitangent += TB.second;

        trisCount[v1]++;
        trisCount[v2]++;
        trisCount[v3]++;
    }

    for (size_t i = 0; i < trisCount.size(); ++i) {

        if (ModelLoader::_vertices[i].Tangent == glm::vec3(0.0f)) {

            glm::vec3 U;
            if (glm::abs(ModelLoader::_vertices[i].Normal.x) > 0.9f) {
                U = glm::vec3(0.f, 1.f, 0.f);
            }
            else if (glm::abs(ModelLoader::_vertices[i].Normal.y) > 0.9f) {
                U = glm::vec3(0.f, 0.f, 1.f);
            }
            else {
                U = glm::vec3(1.f, 0.f, 0.f);
            }

            ModelLoader::_vertices[i].Tangent = glm::normalize(glm::cross(U, ModelLoader::_vertices[i].Normal));
            ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
        }
        else if (ModelLoader::_vertices[i].Bitangent == glm::vec3(0.0f)) {
            ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
            ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
        }
        else {
            ModelLoader::_vertices[i].Tangent /= (float)trisCount[i];
            ModelLoader::_vertices[i].Bitangent /= (float)trisCount[i];

            ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
            ModelLoader::_vertices[i].Bitangent = glm::normalize(ModelLoader::_vertices[i].Bitangent);
        }
    }

    trisCount.clear();

    ModelLoader::_hasIndices = !ModelLoader::_indices.empty();
    ModelLoader::_updateEBO = true;

    return true;
}

bool ModelLoader::LoadModelFromFBX(std::string path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        spdlog::error("Failed to open FBX file: {}", path);
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size <= 0) {
        spdlog::error("FBX file is empty: {}", path);
        return false;
    }

    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        spdlog::error("Failed to read FBX file: {}", path);
        return false;
    }

    file.close();

    constexpr ofbx::LoadFlags flags =
        ofbx::LoadFlags::IGNORE_ANIMATIONS  |
        ofbx::LoadFlags::IGNORE_BONES       |
        ofbx::LoadFlags::IGNORE_CAMERAS     |
        ofbx::LoadFlags::IGNORE_LIGHTS      |
        ofbx::LoadFlags::IGNORE_LIMBS       |
        ofbx::LoadFlags::IGNORE_POSES       |
        ofbx::LoadFlags::IGNORE_SKIN        |
        ofbx::LoadFlags::IGNORE_PIVOTS      |
        ofbx::LoadFlags::IGNORE_VIDEOS;

    ofbx::IScene* scene = ofbx::load(buffer.data(), size, (ofbx::u16)flags);
    if (!scene) {
        spdlog::error("Failed to parse FBX file: {}", path);
        return false;
    }

    ModelLoader::_vertices.clear();
    ModelLoader::_indices.clear();

    for (size_t i = 0; i < scene->getMeshCount(); ++i) {
        const ofbx::Mesh* mesh = scene->getMesh(i);
        if (!mesh) continue;
        const ofbx::GeometryData& geom = mesh->getGeometryData();
        const ofbx::Vec3Attributes& positions = geom.getPositions();
        const ofbx::Vec3Attributes& normals = geom.getNormals();
        const ofbx::Vec2Attributes& uvs = geom.getUVs();

        if (!positions.values || !positions.indices || !normals.values || !uvs.values) {
            spdlog::warn("Skipping mesh due to missing necessary attributes.");
            continue;
        }

        size_t indices_offset = ModelLoader::_vertices.size();
        for (size_t partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
            const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);

            // partitions most likely have several polygons, they are not triangles necessarily, use ofbx::triangulate if you want triangles
            for (size_t polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
                const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];

                for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
                    ModelLoader::_vertices.push_back(Vertex{
                        .Position = glm::vec3(positions.get(i).x, positions.get(i).y, positions.get(i).z),
                        .TexCoords = glm::vec2(uvs.get(i).x, uvs.get(i).y),
                        .Normal = glm::vec3(normals.get(i).x, normals.get(i).y, normals.get(i).z)
                    });
                }
            }

            for (size_t polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
                const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];

                int* indices = new int[3 * (polygon.vertex_count - 2)];
                size_t indices_size = ofbx::triangulate(geom, polygon, indices);

                for (size_t i = 0; i < indices_size; ++i) {
                    ModelLoader::_indices.push_back(indices_offset + indices[i]);
                }

                delete[] indices;
            }
        }
    }
    scene->destroy();

    if (ModelLoader::_vertices.empty() || ModelLoader::_indices.empty()) {
        spdlog::error("Loaded FBX file did not contain needed mesh data.");
        return false;
    }

    // Calculate Tangent and Bitangent (using indices)
    std::vector<size_t> trisCount(ModelLoader::_vertices.size(), 0);

    for (size_t i = 0; i < ModelLoader::_indices.size(); i += 3) {
        size_t v1 = ModelLoader::_indices[i];
        size_t v2 = ModelLoader::_indices[i + 1];
        size_t v3 = ModelLoader::_indices[i + 2];

        std::pair<glm::vec3, glm::vec3> TB = ModelLoader::CalcTangentBitangent(v1, v2, v3);

        ModelLoader::_vertices[v1].Tangent += TB.first;
        ModelLoader::_vertices[v1].Bitangent += TB.second;
        ModelLoader::_vertices[v2].Tangent += TB.first;
        ModelLoader::_vertices[v2].Bitangent += TB.second;
        ModelLoader::_vertices[v3].Tangent += TB.first;
        ModelLoader::_vertices[v3].Bitangent += TB.second;

        trisCount[v1]++;
        trisCount[v2]++;
        trisCount[v3]++;
    }

    for (size_t i = 0; i < trisCount.size(); ++i) {

        if (ModelLoader::_vertices[i].Tangent == glm::vec3(0.0f)) {

            glm::vec3 U;
            if (glm::abs(ModelLoader::_vertices[i].Normal.x) > 0.9f) {
                U = glm::vec3(0.f, 1.f, 0.f);
            }
            else if (glm::abs(ModelLoader::_vertices[i].Normal.y) > 0.9f) {
                U = glm::vec3(0.f, 0.f, 1.f);
            }
            else {
                U = glm::vec3(1.f, 0.f, 0.f);
            }

            ModelLoader::_vertices[i].Tangent = glm::normalize(glm::cross(U, ModelLoader::_vertices[i].Normal));
            ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
        }
        else if (ModelLoader::_vertices[i].Bitangent == glm::vec3(0.0f)) {
            ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
            ModelLoader::_vertices[i].Bitangent = glm::normalize(glm::cross(ModelLoader::_vertices[i].Normal, ModelLoader::_vertices[i].Tangent));
        }
        else {
            ModelLoader::_vertices[i].Tangent /= (float)trisCount[i];
            ModelLoader::_vertices[i].Bitangent /= (float)trisCount[i];

            ModelLoader::_vertices[i].Tangent = glm::normalize(ModelLoader::_vertices[i].Tangent);
            ModelLoader::_vertices[i].Bitangent = glm::normalize(ModelLoader::_vertices[i].Bitangent);
        }
    }

    trisCount.clear();

    ModelLoader::_hasIndices = !ModelLoader::_indices.empty();
    ModelLoader::_updateEBO = true;

    return true;
}

bool ModelLoader::LoadModel(std::string path)
{
    std::string ext = std::filesystem::path(path).extension().string();
    if (ext != std::string(".gltf") && ext != std::string(".glb") && ext != std::string(".obj") && ext != std::string(".fbx")) {
        spdlog::error("ModelLoader does not support this file format.");
        return false;
    }

    ModelLoader::_vertices.clear();
    ModelLoader::_indices.clear();
    bool res = false;
    ModelFormat format = ModelFormat::NONE;

    if (ext == std::string(".gltf") || ext == std::string(".glb")) {
        res = ModelLoader::LoadModelFromGLTF(path);
        format = ModelFormat::GLTF;
    }
    else if (ext == std::string(".obj")) {
        res = ModelLoader::LoadModelFromOBJ(path);
        format = ModelFormat::OBJ;
    }
    else if (ext == std::string(".fbx")) {
        res = ModelLoader::LoadModelFromFBX(path);
        format = ModelFormat::FBX;
    }

    if (res) {
        if (!ModelLoader::IsInit()) {
            glGenBuffers(1, &ModelLoader::_vbo);
            glGenBuffers(1, &ModelLoader::_ebo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, ModelLoader::_vbo);
        glBufferData(GL_ARRAY_BUFFER, ModelLoader::_vertices.size() * sizeof(Vertex), ModelLoader::_vertices.data(), GL_STATIC_DRAW);

        if (ModelLoader::_updateEBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ModelLoader::_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ModelLoader::_indices.size() * sizeof(unsigned int), ModelLoader::_indices.data(), GL_STATIC_DRAW);
            ModelLoader::_updateEBO = false;
        }

        ModelLoader::_fileName = std::filesystem::path(path).filename().string();
        ModelLoader::_format = format;
    }

    ModelLoader::_init = res;

    return res;
}

GLuint ModelLoader::GetVBO()
{
    return ModelLoader::_vbo;
}

GLuint ModelLoader::GetEBO()
{
    return ModelLoader::_ebo;
}

bool ModelLoader::IsInit()
{
    return ModelLoader::_init;
}

bool ModelLoader::HasIndices()
{
    return ModelLoader::_hasIndices;
}

size_t ModelLoader::GetVerticesCount()
{
    return ModelLoader::_vertices.size();
}

size_t ModelLoader::GetIndicesCount()
{
    return ModelLoader::_indices.size();
}

std::string ModelLoader::GetModelName()
{
    return ModelLoader::_fileName;
}

ModelFormat ModelLoader::GetModelFormat()
{
    return ModelLoader::_format;
}

unsigned int* ModelLoader::GetIndices()
{
    return ModelLoader::_indices.data();
}

void ModelLoader::Deinit()
{
    if (ModelLoader::_init) {
        glDeleteBuffers(1, &ModelLoader::_vbo);
        glDeleteBuffers(1, &ModelLoader::_ebo);
        ModelLoader::_vertices.clear();
        ModelLoader::_indices.clear();

        ModelLoader::_init = false;
        ModelLoader::_hasIndices = false;
        ModelLoader::_format = ModelFormat::NONE;
        ModelLoader::_fileName.clear();
    }
}

bool ModelLoader::OpenFileDialog(std::string path)
{
    const char* filters[] = { "*.fbx", "*.obj", "*.glb", "*.gltf" };
    const char* filePath = tinyfd_openFileDialog(
        "Choose 3D Object File",
        path.c_str(),
        3, filters, "3D Objects (*.fbx, *.obj, *.glb, *.gltf)", 0);

    if (filePath)
    {
        if (ModelLoader::LoadModel(filePath)) {
            return true;
        }
        else {
            ImGui::OpenPopup("Error Loading Model");
        }
    }

    ImGui::ShowErrorPopup("Error Loading Model", "Failed to load 3D Model. Please check the file and try again.");

    return false;
}