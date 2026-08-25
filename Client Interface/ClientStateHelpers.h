#pragma once

#include <string>

std::string getTimestamp();
void logMessage(const std::string& message);
void stateLog(int state);
bool CheckLaunchArguments();
void InstallQtMessageHandler();