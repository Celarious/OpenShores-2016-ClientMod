#include "pch.h"

#include <windows.h>
#include <mutex>
#include <fstream>

#include "ClientInterface.h"
#include "ClientStateProcessing.h"

static int g_state = 0; // Global state variable
static std::mutex g_mutex; // Mutex for safely modifying the variable
HMODULE hGame = GetModuleHandleW(nullptr);

extern "C" __declspec(dllexport)
void SetState(int state, void* context = nullptr, void* aux = nullptr) // 2nd and 3rd parameters are optional, used whenever a state needs extra objects
{
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_state = state;
    }
    ProcessState(state, context, aux); // This might be temporary (there is nothing more permanent than a temporary software solution), used to migrate the core asm functionality to C++
}

extern "C" __declspec(dllexport)
int GetState() // Could be used in the future by other C++ or even Lua modules to fetch the current state
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_state;
}

const QString& SetAuLogFilename()
{
    static const QString filename = QStringLiteral("logs/ClientInterface.log");
    return filename;
}