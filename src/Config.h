#pragma once

class Config {
private:
    static bool verbose;
#if !WINDOW_APP
    static bool interactive;
#endif

public:
    static void setVerbose(bool value);
#if !WINDOW_APP
    static void setInteractive(bool value);
#endif

    static bool isVerbose();
#if !WINDOW_APP
    static bool isInteractive();
#endif
};