// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CLIENTINTERFACE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CLIENTINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CLIENTINTERFACE_EXPORTS
#define CLIENTINTERFACE_API __declspec(dllexport)
#else
#define CLIENTINTERFACE_API __declspec(dllimport)
#endif

#pragma once

#include <windows.h>
#include <QString>

extern HMODULE hGame;
extern "C" __declspec(dllexport)
const QString& SetAuLogFilename();