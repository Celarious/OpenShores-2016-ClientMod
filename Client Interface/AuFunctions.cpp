#include "pch.h"

#include "ClientInterface.h"
#include "AuFunctions.h"

// This file is used to centralize the game's existing exported Au functions

namespace Au {
    ReadEntryFn ReadEntry;
    WriteEntryFn WriteEntry;
    Log_t Log = nullptr;

    void Initialize() // Ensures that the program has loaded before resolving addresses
    {
        ReadEntry =
            *reinterpret_cast<ReadEntryFn*>(
                reinterpret_cast<uintptr_t>(hGame) + 0x447A60
            );

        /*WriteEntry =
            *reinterpret_cast<WriteEntryFn*>(
                reinterpret_cast<uintptr_t>(hGame) + 0x824318
            );

        Log =
            *reinterpret_cast<Log_t*>(
                reinterpret_cast<uintptr_t>(hGame) + 0x8240A0
            );*/
    }
}