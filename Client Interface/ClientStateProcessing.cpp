#include "pch.h"
#include <sstream>

#include "ClientInterface.h"
#include "ClientStateHelpers.h"
#include "ClientStateProcessing.h"
#include "AuFunctions.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QStackedLayout>
#include <QtCore/QMetaObject>
#include <QtWidgets/QApplication>
#include <QtGui/QIcon>

static QLineEdit* g_ipEdit = nullptr; // Our inserted IP input field
static bool g_state8Fired = false; // Prevents state 8 from firing repeatedly during loop
static uintptr_t g_auGlobal = 0; // Storage for AuGlobal to avoid repeat lookups
static void* g_settings = nullptr; // AuGlobal + 0x238

void ProcessState(int state, void* context, void* aux)
{
    switch (state)
    {
    case 0: // Default, nothing should happen here
        break;

    case 1: // Early startup, right after entry point
        Au::Initialize(); // Prepares the Au functions for our use, needed to avoid static initialization timing issues
        InstallQtMessageHandler();
        stateLog(state);
        break;

    case 2: // Qt+Au initialization after CRT setup
        stateLog(state);
        break;

    case 3: // Game launch argument processing, right after QCoreApplication::arguments()
        stateLog(state);
        break;

    case 4: // Right after AuGlobal is initialized
        stateLog(state);
        break;

    case 5: // Scrolling text state, called early in UI setup
        stateLog(state);
        {
            if (!context)
                return;

            QList<QString>* textList =
                reinterpret_cast<QList<QString>*>(context); // RSI register passed through RDX (win x64 convention)
            
            textList->clear(); // Clears the original scrolling text list

            textList->append(QString::fromLatin1("*OpenShores")); // The * at the start of the string is a marker that the game checks for, and if present, removes it and centers + boldens the line
            textList->append(QString::fromLatin1("Welcome to OpenShores"));
            textList->append(QString::fromLatin1("V0.1.0 (2026)"));
        }
        break;

    case 6: // Login UI setup
        stateLog(state);
        {
            QVBoxLayout* layout = static_cast<QVBoxLayout*>(context);
            auto pAuGlobal = reinterpret_cast<uintptr_t*>(
                reinterpret_cast<uintptr_t>(hGame) + 0x8243F0 // RVA of AuGlobal
                );
            auto gpAuGlobal = *pAuGlobal;
            g_auGlobal = *reinterpret_cast<uintptr_t*>(gpAuGlobal);
            g_settings = reinterpret_cast<void*>(g_auGlobal + 0x238); // Stores the existing AuSettings variable, since it needs to be provided when calling any AuSettings function
            QString key("/Account/Host"); // Sets the key to check
            QString host; // This is used as the return variable
            Au::ReadEntry(g_settings, host, key, &host); // Actually reads the key's value
            g_ipEdit = new QLineEdit(host); // Prefills the input field if the Host key exists
            g_ipEdit->setToolTip(QStringLiteral("<html><b>IP address</b><br>Enter OpenShores IP</html>"));
            g_ipEdit->setPlaceholderText(QStringLiteral("Enter IP address"));
            layout->addWidget(g_ipEdit); // Adds the widget to the UI
            if (CheckLaunchArguments())
            {
                QPushButton* loginButton =
                    *reinterpret_cast<QPushButton**>(
                        static_cast<char*>(aux) + 0x50
                        );
                if (loginButton)
                {
                    QMetaObject::invokeMethod( // Automatically logs in if the launcher passes -nologin by simulating a click
                        loginButton,
                        "click",
                        Qt::QueuedConnection
                    );
                }
            }
        }
        break;

    case 7: // Background image loading and rendering
        stateLog(state);
        {
            QImage* image = static_cast<QImage*>(context); // Converts the passed context to a qimage
            image->load(QString::fromLatin1("assets/Background.png"));
            QWidget* mainWindow = QApplication::activeWindow();

            if (mainWindow)
                mainWindow->setWindowTitle("OpenShores");
                mainWindow->setWindowIcon(QIcon("assets/OS_Icon.png"));
        }
        break;

    case 8: // Login UI ready, painting started
        if (!g_state8Fired) // Prevents state from repeatedly firing
        {
            g_state8Fired = true;
            stateLog(state);
        }
        break;

    case 9: // Immediate post-login click
        stateLog(state);
        {
            QString key("/Account/Host");
            QString host = g_ipEdit->text();
            Au::WriteEntry(g_settings, key, host); // Writes the user's entered host to the registry for prefilling
        }
        break;

    case 10: // Login comms begin
        stateLog(state);
        {
            QString host = g_ipEdit->text();

            std::string hoststring = host.toUtf8().toStdString(); // Logging
            logMessage("Host set to: " + hoststring);

            *reinterpret_cast<QString*>(
                (g_auGlobal) + 0x150 // Login host
                ) = host;

            *reinterpret_cast<QString*>(
                (g_auGlobal) + 0x160 // Mail host
                ) = host;

            *reinterpret_cast<QString*>(
                (g_auGlobal) + 0x218 // Scene host
                ) = host;
        }
        break;

    case 11: // Avatar UI setup
        stateLog(state);
        {
            auto* stacked = static_cast<QStackedLayout*>(context); // The layout is passed to SetState() by the custom ASM
            auto* group = stacked->widget(1)->findChild<QGroupBox*>();
            auto* avatarLayout = static_cast<QBoxLayout*>(group->layout()->itemAt(0)->layout()); // Finds the layout containing the avatar slots

            auto* aPageButtons = new QHBoxLayout();
            auto* prevButton = new QPushButton("Previous", group);
            auto* nextButton = new QPushButton("Next", group);
            prevButton->setToolTip(QStringLiteral("Previous avatar list page"));
            nextButton->setToolTip(QStringLiteral("Next avatar list page"));
            aPageButtons->addWidget(prevButton);
            aPageButtons->addWidget(nextButton);

            avatarLayout->addLayout(aPageButtons);
            logMessage("Avatar page buttons added");
        }
        break;

    case 12: // Avatar UI loop
        stateLog(state);
        break;
    }
}
