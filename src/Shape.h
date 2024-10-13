#pragma once
#include <Vertex.h>

class Shape {
private:
	// Vertices
	static Vertex _cubeVertices[24];
	static Vertex _quadVertices[4];

	// Indices
	static unsigned int _cubeIndices[36];
	static unsigned int _quadIndices[6];

	// Initialization
	static bool _cubeInitialized;
	static bool _quadInitialized;

	// VBO
	static GLuint _cubeVBO;
	static GLuint _quadVBO;

	// EBO
	static GLuint _cubeEBO;
	static GLuint _quadEBO;
public:
	Shape() = default;
	~Shape() = default;

	// Get VBO
	static GLuint GetCubeVBO() {
		if (!_cubeInitialized) {
			glGenBuffers(1, &_cubeVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(Vertex), &_cubeVertices, GL_STATIC_DRAW);

			glGenBuffers(1, &_cubeEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cubeEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), &_cubeIndices, GL_STATIC_DRAW);

			_cubeInitialized = true;

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		return _cubeVBO;
	}

	static GLuint GetQuadVBO() {
		if (!_quadInitialized) {
			glGenBuffers(1, &_quadVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
			glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), &_quadVertices, GL_STATIC_DRAW);

			glGenBuffers(1, &_quadEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &_quadIndices, GL_STATIC_DRAW);

			_quadInitialized = true;

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		return _quadVBO;
	}

	// Get EBO
	static GLuint GetCubeEBO() {
		if (!_cubeInitialized) {
			glGenBuffers(1, &_cubeVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(Vertex), &_cubeVertices, GL_STATIC_DRAW);

			glGenBuffers(1, &_cubeEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cubeEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), &_cubeIndices, GL_STATIC_DRAW);

			_cubeInitialized = true;

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		return _cubeEBO;
	}

	static GLuint GetQuadEBO() {
		if (!_quadInitialized) {
			glGenBuffers(1, &_quadVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
			glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), &_quadVertices, GL_STATIC_DRAW);

			glGenBuffers(1, &_quadEBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &_quadIndices, GL_STATIC_DRAW);

			_quadInitialized = true;

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		return _quadEBO;
	}

	// Get Vertices Count
	static size_t GetCubeVerticesCount() {
		return 24;
	}

	static size_t GetQuadVerticesCount() {
		return 4;
	}

	// Get Indices Count
	static size_t GetCubeIndicesCount() {
		return 36;
	}

	static size_t GetQuadIndicesCount() {
		return 6;
	}
	
	// Get Indices
	static unsigned int* GetCubeIndices() {
		return _cubeIndices;
	}

	static unsigned int* GetQuadIndices() {
		return _quadIndices;
	}
};

#ifdef SHAPE_IMPLEMENTATION

// Initialization
bool Shape::_cubeInitialized = false;
bool Shape::_quadInitialized = false;

// VBO
GLuint Shape::_cubeVBO = 0;
GLuint Shape::_quadVBO = 0;

// EBO
GLuint Shape::_cubeEBO = 0;
GLuint Shape::_quadEBO = 0;

// Vertices
Vertex Shape::_cubeVertices[24] = {
	{	.Position = glm::vec3(-0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(0.0f, 0.0f, 1.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(0.0f, 0.0f, 1.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(0.0f, 0.0f, -1.0f), .Tangent = glm::vec3(-1.0f, 0.0f, 0.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(-0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(0.0f, 0.0f, -1.0f), .Tangent = glm::vec3(-1.0f, 0.0f, 0.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},

	{	.Position = glm::vec3(-0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(0.0f, 0.0f, 1.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(0.0f, 0.0f, 1.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, -0.5f),	.TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(0.0f, 0.0f, -1.0f), .Tangent = glm::vec3(-1.0f, 0.0f, 0.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(-0.5f, -0.5f, -0.5f), .TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(0.0f, 0.0f, -1.0f), .Tangent = glm::vec3(-1.0f, 0.0f, 0.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},

	{	.Position = glm::vec3(-0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(-1.0f, 0.0f, 0.0f), .Tangent = glm::vec3(0.0f, 0.0f, 1.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(1.0f, 0.0f, 0.0f),	.Tangent = glm::vec3(0.0f, 0.0f, -1.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(1.0f, 0.0f, 0.0f),	.Tangent = glm::vec3(0.0f, 0.0f, -1.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(-0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(-1.0f, 0.0f, 0.0f), .Tangent = glm::vec3(0.0f, 0.0f, 1.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},

	{	.Position = glm::vec3(-0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(-1.0f, 0.0f, 0.0f), .Tangent = glm::vec3(0.0f, 0.0f, 1.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(1.0f, 0.0f, 0.0f),	.Tangent = glm::vec3(0.0f, 0.0f, -1.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, -0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(1.0f, 0.0f, 0.0f),	.Tangent = glm::vec3(0.0f, 0.0f, -1.0f),	.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},
	{	.Position = glm::vec3(-0.5f, -0.5f, -0.5f), .TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(-1.0f, 0.0f, 0.0f), .Tangent = glm::vec3(0.0f, 0.0f, 1.0f),		.Bitangent = glm::vec3(0.0f, -1.0f, 0.0f)	},

	{	.Position = glm::vec3(-0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)	},
	{	.Position = glm::vec3(0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)	},
	{	.Position = glm::vec3(-0.5f, 0.5f, -0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f),	.Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)	},

	{	.Position = glm::vec3(-0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(0.0f, -1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, 0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(0.0f, -1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f)	},
	{	.Position = glm::vec3(0.5f, -0.5f, -0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(0.0f, -1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f)	},
	{	.Position = glm::vec3(-0.5f, -0.5f, -0.5f), .TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(0.0f, -1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f),		.Bitangent = glm::vec3(0.0f, 0.0f, -1.0f)	}
};

Vertex Shape::_quadVertices[4] = {
	{	.Position = glm::vec3(-0.5f, 0.0f, -0.5f),	.TexCoords = glm::vec2(0.0f, 0.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f), .Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)		},
	{	.Position = glm::vec3(0.5f, 0.0f, -0.5f),	.TexCoords = glm::vec2(1.0f, 0.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f), .Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)		},

	{	.Position = glm::vec3(-0.5f, 0.0f, 0.5f),	.TexCoords = glm::vec2(0.0f, 1.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f), .Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)		},
	{	.Position = glm::vec3(0.5f, 0.0f, 0.5f),	.TexCoords = glm::vec2(1.0f, 1.0f), .Normal = glm::vec3(0.0f, 1.0f, 0.0f), .Tangent = glm::vec3(1.0f, 0.0f, 0.0f), .Bitangent = glm::vec3(0.0f, 0.0f, 1.0f)		}
};

// Indices
unsigned int Shape::_cubeIndices[36] = {
	0, 4, 5,
	0, 5, 1,

	2, 6, 7,
	2, 7, 3,

	9, 13, 14,
	9, 14, 10,

	11, 15, 12,
	11, 12, 8,

	19, 16, 17,
	19, 17, 18,

	20, 23, 22,
	20, 22, 21
};

unsigned int Shape::_quadIndices[6] = {
	2, 1, 0,
	2, 3, 1
};

#endif