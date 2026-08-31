#include "pch.h"
#include <sstream>

#include "ClientInterface.h"
#include "ClientStateHelpers.h"
#include "ClientStateProcessing.h"
#include "AuFunctions.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QStackedLayout>
#include <QtCore/QMetaObject>
#include <QtWidgets/QApplication>
#include <QtGui/QIcon>

static QLineEdit* g_ipEdit = nullptr; // Our inserted IP input field
static bool g_state6Fired = false; // Prevents state 6 from firing repeatedly during loop
static uintptr_t g_auGlobal = 0; // Storage for AuGlobal to avoid repeat lookups
static void* g_settings = nullptr; // AuGlobal + 0x218

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

    case 5: // Scrolling text and login UI setup state
        stateLog(state);
        {
            QList<QString>* textList =
                reinterpret_cast<QList<QString>*>(context); // RSI register passed through RDX (win x64 convention)
            
            textList->clear(); // Clears the original scrolling text list

            textList->append(QString::fromLatin1("*OpenShores")); // The * at the start of the string is a marker that the game checks for, and if present, removes it and centers + boldens the line
            textList->append(QString::fromLatin1("Welcome to OpenShores Classic"));
            textList->append(QString::fromLatin1("V0.1.0 (2026)"));


            auto pAuGlobal = reinterpret_cast<uintptr_t*>(
                reinterpret_cast<uintptr_t>(hGame) + 0x447B48 // RVA of AuGlobal
            );
            auto gpAuGlobal = *pAuGlobal;
            g_auGlobal = *reinterpret_cast<uintptr_t*>(gpAuGlobal);
            g_settings = reinterpret_cast<void*>(g_auGlobal + 0x218); // Stores the existing AuSettings variable, since it needs to be provided when calling any AuSettings function
            QString key("/Account/Host"); // Sets the key to check
            QString host; // This is used as the return variable
            Au::ReadEntry(g_settings, host, key, &host); // Actually reads the key's value

            QGridLayout* layout = static_cast<QGridLayout*>(aux); // Timing is different than the 2018 client so the IP input field happens here
            QLineEdit* username = qobject_cast<QLineEdit*>(layout->itemAtPosition(2, 1)->widget()); // Username field is stored at row 2 of the QGridLayout
            QVBoxLayout* fields = new QVBoxLayout();
            fields->setContentsMargins(0, 0, 0, 0);
            fields->setSpacing(4);

            g_ipEdit = new QLineEdit(host);
            g_ipEdit->setToolTip(QStringLiteral("<html><b>IP address</b><br>Enter OpenShores IP</html>"));
            g_ipEdit->setPlaceholderText(QStringLiteral("Enter IP address"));

            fields->addWidget(g_ipEdit);
            fields->addWidget(username);
            layout->removeWidget(username);
            layout->addLayout(fields, 2, 1, 1, 8); // The login box is positioned differently in this client so we need to do this weird nesting shenanigans

            QPushButton* loginButton = layout->parentWidget()->findChild<QPushButton*>();
            if (loginButton && CheckLaunchArguments())
            {
                QMetaObject::invokeMethod( // Automatically logs in if the launcher passes -nologin by simulating a click
                    loginButton,
                    "click",
                    Qt::QueuedConnection
                );
            }
        }
        break;

    case 6: // Background image loading and render loop
        {
            if (!g_state6Fired) { // Prevents state from repeatedly firing
                QImage* image = static_cast<QImage*>(context); // Converts the passed context to a qimage
                image->load(QString::fromLatin1("assets/Background.png"));
                QWidget* mainWindow = QApplication::activeWindow();

                if (mainWindow) {
                    mainWindow->setWindowTitle("OpenShores");
                    mainWindow->setWindowIcon(QIcon("assets/OS_Icon.png"));
                }
                g_state6Fired = true;
                stateLog(state);
            }
        }
        break;

    case 7: // Immediate post-login click
        stateLog(state);
        {
            QString key("/Account/Host");
            QString host = g_ipEdit->text();
            Au::WriteEntry(g_settings, key, host); // Writes the user's entered host to the registry for prefilling
        }
        break;

    case 8: // Login comms begin
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
                (g_auGlobal) + 0x1E8 // Scene host
            ) = host;
        }
        break;

    case 9: // 
        stateLog(state);
        {

        }
        break;

    case 10: //
        stateLog(state);
        {

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
