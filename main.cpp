#include "olcConsoleGameEngine.h"

class OneLoneCoder_Example : public olcConsoleGameEngine {

public:
    OneLoneCoder_Example() : olcConsoleGameEngine() {
        m_sAppName = L"Example";
    }

    bool OnUserCreate() override {
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        FillTriangle(2, 2, 86, 15, 8, 39, PIXEL_SOLID, FG_CYAN);
        if (GetKey(kVK_Escape)) return false;
        return true;
    }
};

int main() {
    OneLoneCoder_Example game;
    game.ConstructConsole(160, 40, 8);
    game.Start();
    return 0;
}
