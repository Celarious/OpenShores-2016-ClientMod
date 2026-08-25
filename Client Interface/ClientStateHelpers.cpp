#include "pch.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <QCoreApplication>
#include <QStringList>
#include <QtGlobal>
#include <QFile>
#include <QTextStream>
#include <QMutex>

#include "ClientStateHelpers.h"

std::string getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm;
    localtime_s(&tm, &time);

    std::ostringstream ss;
    ss << std::put_time(&tm, "[%H:%M:%S] ");
    return ss.str();
}

void logMessage(const std::string& message)
{
    static bool initialized = false; // So the log only wipes itself once at startup

    if (!initialized) { // So the log only wipes itself once at startup
        std::ofstream("logs/ClientInterface.log", std::ios::trunc).close(); // Wipes itself on every program start so it doesn't fill forever
        std::ofstream("logs/GameDebug.log", std::ios::trunc).close();
        std::ofstream("logs/ClientInterface.log", std::ios::app) << "==== Logging started! ====\n";
        std::ofstream("logs/ClientInterface.log", std::ios::app) << "Note: AuGlobal::Log() calls have been redirected here, our added logs are marked with [CI]\n\n";
        initialized = true;
    }
    std::ofstream log("logs/ClientInterface.log", std::ios::app);
    log << "[CI]" << getTimestamp() << message << '\n';
}

void stateLog(int state) // Temporary log function for checking if the states are called
{
    logMessage("SetState(" + std::to_string(state) + ") received!");
}

bool CheckLaunchArguments() // Function that handles the OS launcher's custom args
{
    const QStringList args = QCoreApplication::arguments();
    bool noLogin = args.contains(QStringLiteral("-nologin")); // Checks if -nologin was passed
    logMessage(std::string("-nologin = ") + (noLogin ? "true" : "false"));
    return noLogin;
}

static void qtMessageHandler(QtMsgType type,
    const QMessageLogContext& context,
    const QString& msg)
{
    QFile file("logs/GameDebug.log");

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        switch (type) {
        case QtDebugMsg:    out << "[DEBUG] "; break;
        case QtInfoMsg:     out << "[INFO] "; break;
        case QtWarningMsg:  out << "[WARNING] "; break;
        case QtCriticalMsg: out << "[CRITICAL] "; break;
        case QtFatalMsg:    out << "[FATAL] "; break;
        }

        out << msg << '\n';
    }
}

void InstallQtMessageHandler()
{
    qInstallMessageHandler(qtMessageHandler);
}