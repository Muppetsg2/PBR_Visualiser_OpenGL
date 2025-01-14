#include <Config.h>

#if WINDOW_APP
bool Config::verbose = true;
#else
bool Config::verbose = false;
bool Config::interactive = false;
#endif

void Config::setVerbose(bool value)
{
	Config::verbose = value;
	spdlog::info("Verbose mode: {}", Config::verbose ? "true" : "false");
}

#if !WINDOW_APP
void Config::setInteractive(bool value)
{
	Config::interactive = value;
	spdlog::info("Interactive mode: {}", Config::interactive ? "true" : "false");
}
#endif

bool Config::isVerbose()
{
	return Config::verbose;
}

#if !WINDOW_APP
bool Config::isInteractive()
{
	return Config::interactive;
}
#endif