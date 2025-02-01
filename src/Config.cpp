#include <Config.h>

#if WINDOW_APP
bool Config::_verbose = true;
#else
bool Config::_verbose = false;
bool Config::_interactive = false;
#endif

void Config::SetVerbose(bool value)
{
	Config::_verbose = value;
	spdlog::info("Verbose mode: {}", Config::_verbose ? "true" : "false");
}

#if !WINDOW_APP
void Config::SetInteractive(bool value)
{
	Config::_interactive = value;
	spdlog::info("Interactive mode: {}", Config::_interactive ? "true" : "false");
}
#endif

bool Config::IsVerbose()
{
	return Config::_verbose;
}

#if !WINDOW_APP
bool Config::IsInteractive()
{
	return Config::_interactive;
}
#endif