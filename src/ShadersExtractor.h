#pragma once

class ShadersExtractor {
private:
	static std::unordered_map<std::string, std::string> _shaders;
	static bool _init;

	static std::pair<bool, std::string> ReadAndDecodeFile(std::string filePath, std::string key);
	static bool AddShader(std::string shaderCode);

	ShadersExtractor() = delete;

public:
	static void Init(std::string dataPath);
	static void Deinit();

	static std::string GetShaderContent(std::string name);

	static bool HasShader(std::string name);
};