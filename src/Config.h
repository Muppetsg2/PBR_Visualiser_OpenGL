#pragma once

class Config {
private:
    static bool _verbose;
#if !WINDOW_APP
    static bool _interactive;
#endif
    Config() = delete;

public:
    static void SetVerbose(bool value);
#if !WINDOW_APP
    static void SetInteractive(bool value);
#endif

    static bool IsVerbose();
#if !WINDOW_APP
    static bool IsInteractive();
#endif
};