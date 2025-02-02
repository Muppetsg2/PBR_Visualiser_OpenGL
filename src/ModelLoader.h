#pragma once
#include <Vertex.h>
#include <macros.h>

ENUM_CLASS_BASE_VALUE(ModelFormat, uint8_t, GLTF, 0, OBJ, 1, NONE, 2)

class ModelLoader {
private:
	static std::vector<Vertex> _vertices;
	static std::vector<unsigned int> _indices;

	static bool _init;
	static bool _hasIndices;
	static bool _updateEBO;

	static std::string _fileName;
	static ModelFormat _format;

	static GLuint _vbo;
	static GLuint _ebo;

	// ImGui
	static bool openFileDialog;
	static ImFileDialogInfo fileDialogInfo;

	static std::pair<glm::vec3, glm::vec3> CalcTangentBitangent(size_t t1, size_t t2, size_t t3);

	static bool LoadModelFromGLTF(std::string path);
	static bool LoadModelFromOBJ(std::string path);

	ModelLoader() = delete;

public:
	static bool LoadModel(std::string path);

	static GLuint GetVBO();
	static GLuint GetEBO();

	static bool IsInit();
	static bool HasIndices();

	static size_t GetVerticesCount();
	static size_t GetIndicesCount();

	static std::string GetModelName();
	static ModelFormat GetModelFormat();

	static unsigned int* GetIndices();

	static void Deinit();

	static void OpenImGuiFileDialog(std::string path);
	static bool ShowImGuiFileDialog();
};