#pragma once

struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

static std::pair<glm::vec3, glm::vec3> CalcTangentBitangent(std::vector<Vertex>& vertices, unsigned int i1, unsigned int i2, unsigned int i3) {
    std::pair<glm::vec3, glm::vec3> TB;

    Vertex v0 = vertices[i1];
    Vertex v1 = vertices[i2];
    Vertex v2 = vertices[i3];

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

    float r = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);

    // Save the results
    TB.first = (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r;
    TB.second = (delta_pos2 * delta_uv1.x - delta_pos1 * delta_uv2.x) * r;

    return TB;
}