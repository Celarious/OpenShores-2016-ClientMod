#pragma once

#include <QString>

using ReadEntryFn = void(__cdecl*)(
    void* settings,
    QString& result,
    const QString& key,
    QString* resultStorage
);

using WriteEntryFn = void(__cdecl*)(
    void* settings,
    const QString& key,
    const QString& value
);

using Log_t = void(__cdecl*)(
    const QString&, 
    const QString&
);

namespace Au {
    extern ReadEntryFn ReadEntry;
    extern WriteEntryFn WriteEntry;
    extern Log_t Log;

    void Initialize();
}