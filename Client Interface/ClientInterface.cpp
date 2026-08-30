#include "pch.h"

#include <windows.h>
#include <mutex>
#include <fstream>

#include "ClientInterface.h"
#include "ClientStateProcessing.h"

#include <QtGui/QPen>
#include <QtCore/QRect>

static int g_state = 0; // Global state variable
static std::mutex g_mutex; // Mutex for safely modifying the variable
static HTHEME g_editTheme = nullptr;
static QStyle* g_fallbackStyle = nullptr;
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

extern "C" __declspec(dllexport) void DrawHoverBorder(QPainter* painter, int x1, int y1, int x2, int y2) // Needed as part of fixing the stupid flashbang on hover bug
{
    QRect rect;
    rect.setCoords(x1, y1, x2, y2);
    QPen pen(QColor(255, 255, 255));
    pen.setWidth(1);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}