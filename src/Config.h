#pragma once

class Config {
private:
    static bool verbose;

public:
    static void setVerbose(bool value);

    static bool isVerbose();
};