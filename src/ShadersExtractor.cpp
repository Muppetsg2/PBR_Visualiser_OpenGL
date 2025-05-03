#include <ShadersExtractor.h>

std::unordered_map<std::string, std::string> ShadersExtractor::_shaders = std::unordered_map<std::string, std::string>();

bool ShadersExtractor::_init = false;

std::pair<bool, std::string> ShadersExtractor::ReadAndDecodeFile(std::string filePath, std::string key)
{
    if (key.empty()) {
        spdlog::error("Key cannot be empty.");
        return { false, {} };
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        spdlog::error("Error occurred while opening file at path '{}'.", filePath);
        return { false, {} };
    }

    try {
        // Check file size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            spdlog::error("File at path '{}' is empty or cannot determine size.", filePath);
            return { false, {} };
        }

        // Read file content
        std::vector<char> fileContent(static_cast<std::size_t>(size));
        if (!file.read(fileContent.data(), size)) {
            spdlog::error("Error occurred while reading file at path '{}'.", filePath);
            return { false, {} };
        }

        // Decrypt file content using xor
        std::string decodedData;
        decodedData.resize(fileContent.size());

        size_t keyLength = key.length();
        size_t diff = 0;
        for (size_t i = 0; i < fileContent.size(); ++i) {
            if (fileContent[i] == '\r' && i + 1 < fileContent.size() && fileContent[i + 1] == '\n') {
                diff += 1;
                continue;
            }
            decodedData[i - diff] = (char)(fileContent[i] ^ key[(i - diff) % keyLength]);
        }

        decodedData.resize(fileContent.size() - diff);

        return { true, decodedData };
    }
    catch (const std::ifstream::failure& e) {
        spdlog::error("Error occurred while processing file at path '{}': {}", filePath, e.what());
        return { false, {} };
    }
}

bool ShadersExtractor::AddShader(std::string shaderCode)
{
    // Regex to match FILE_NAME, and the rest of the content
    std::regex metadataRegex(R"(^FILE_NAME=([\w\.]+)\n([\s\S]*))");
    std::smatch match;

    // Perform regex search
    if (std::regex_search(shaderCode, match, metadataRegex)) {
        if (match.size() >= 3) {
            // Extract FILE_NAME and the rest of the content
            std::string fileName = match[1].str();
            std::string content = match[2].str();

            ShadersExtractor::_shaders[fileName] = content;
        }
    }
    else {
        spdlog::error("No valid FILE_NAME metadata found in the input, or it's not at the start.");
        return false;
    }

    return true;
}

void ShadersExtractor::Init(std::string dataPath)
{
    std::pair<bool, std::string> res = ShadersExtractor::ReadAndDecodeFile(dataPath, S_KEY);

    if (!res.first) {
        spdlog::error("Could not read or decode the contents of data file '{}'", dataPath);
        return;
    }

    const std::string beginMarker = "[BEGIN_SHADER]";
    const std::string endMarker = "[END_SHADER]";
    size_t startPos = 0;

    while (true) {
        // Find start of shader block
        startPos = res.second.find(beginMarker, startPos);
        if (startPos == std::string::npos) break;

        // Move position past begin marker
        startPos += beginMarker.length();

        // Find end of shader block
        size_t endPos = res.second.find(endMarker, startPos);
        if (endPos == std::string::npos) break;

        // Get content between markers
        std::string shaderCode = res.second.substr(startPos, endPos - startPos);

        // Delete additional white marks on begining and end
        shaderCode.erase(0, shaderCode.find_first_not_of("\n\r\t "));
        shaderCode.erase(shaderCode.find_last_not_of("\n\r\t ") + 1);

        // Pass shaderCode to AddShader function
        AddShader(shaderCode);

        // Move position past end marker
        startPos = endPos + endMarker.length();
    }

    ShadersExtractor::_init = true;
}

void ShadersExtractor::Deinit()
{
    if (ShadersExtractor::_init) {
        ShadersExtractor::_shaders.clear();
    }
}

std::string ShadersExtractor::GetShaderContent(std::string name)
{
    if (!ShadersExtractor::_init) {
        spdlog::error("Shader Extractor is not initialized");
        return "";
    }

    if (!ShadersExtractor::HasShader(name)) {
        spdlog::error("Given shader does not exist. Shader name: {}", name);
        return "";
    }

    return ShadersExtractor::_shaders[name];
}

bool ShadersExtractor::HasShader(std::string name)
{
    if (!ShadersExtractor::_init) {
        spdlog::error("Shader Extractor is not initialized");
        return false;
    }

    return ShadersExtractor::_shaders.contains(name); 
}
