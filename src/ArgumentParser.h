#pragma once

class ArgumentParser {
public:
    using ArgHandler = std::function<void(const std::string&)>;
    using ArgHandlerWithIndex = std::function<void(int&, int, char**)>;

private:
    static std::unordered_map<std::string, ArgHandler> _handlers;
    static std::unordered_map<std::string, bool> _requiresValue;
    static std::unordered_map<std::string, ArgHandlerWithIndex> _indexHandlers;

public:
    static void AddOption(const std::string& flag, ArgHandler handler, bool requiresValue = false) {
        _handlers[flag] = handler;
        _requiresValue[flag] = requiresValue;
    }

    static void AddOptionWithIndex(const std::string& flag, ArgHandlerWithIndex handler) {
        _indexHandlers[flag] = handler;
    }

    static void Parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (_indexHandlers.find(arg) != _indexHandlers.end()) {
                _indexHandlers[arg](i, argc, argv);
            }
            else if (_handlers.find(arg) != _handlers.end()) {
                if (_requiresValue[arg]) {
                    if (i + 1 < argc) {
                        _handlers[arg](argv[++i]);
                    }
                    else {
                        spdlog::warn("Missing value for argument: {}", arg);
                    }
                }
                else {
                    _handlers[arg]("");
                }
            }
            else {
                spdlog::warn("Unknown argument: {}", arg);
            }
        }
    }

    static void Deinit() {
        _handlers.clear();
        _requiresValue.clear();
        _indexHandlers.clear();
    }
};

#ifdef ARGUMENT_PARSER_IMPLEMENTATION
std::unordered_map<std::string, ArgumentParser::ArgHandler> ArgumentParser::_handlers = {};
std::unordered_map<std::string, bool> ArgumentParser::_requiresValue = {};
std::unordered_map<std::string, ArgumentParser::ArgHandlerWithIndex> ArgumentParser::_indexHandlers = {};
#endif