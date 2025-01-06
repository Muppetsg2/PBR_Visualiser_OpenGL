#include <Config.h>

#if _DEBUG
bool Config::verbose = true;
#else
bool Config::verbose = false;
#endif

void Config::setVerbose(bool value)
{
	Config::verbose = value;
	spdlog::info("Verbose mode: {}", verbose ? "true" : "false");
}

bool Config::isVerbose()
{
	return Config::verbose;
}
