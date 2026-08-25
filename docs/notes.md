# Patching notes
I decided to use a hook-and-event model for the client interface, with the game's process split into separate logical state hooks depending on functionality, such as post-CRT setup and login UI setup. This would allow easy modding in the future with the mod able to select the specific game state they need for their functionality.
Each hook is a tiny ASM hijack where we call our ClientInterace.dll's exported `SetState(int)` function, then immediately return to original ASM code. `SetState()` internally calls `ProcessState()`, which is where further work happens depending on the current state.
Certain functionality required extra context and data to be passed to the DLL, so SetState() is able to take two extra arguments, both default initialized to nullptr and typed as void to allow for easy casting. Per Windows x64 convention, argument 1 goes in RCX, argument 2 in RDX, argument 3 in R8. This keeps the ASM modification extremely minimal even when three arguments are needed.

#### Server redirection
The first thing that was done with ProcessState() was to port the hostname connection targets from ASM to our DLL. I decided to do this in state 6, which is positioned during the login UI setup. I made the game pass the qvboxlayout as the 2nd arg, to easily add our field to it.
Then, the code was simply `new QLineEdit()` and `layout->addWidget()`. `setToolTip()` and `setPlaceholderText` were also used for QoL.
The QLineEdit also needed to be prefilled just like the ASM originally did. For that, `GetModuleHandleW(nullptr) + 0x8243F0` was used to get the address of AuGlobal's object pointer, was dereferenced, and `AuGlobal + 0x238` was stored as the location of AuSettings. Then, the game's existing registry read/write functions were repurposed, by simply doing `GetModuleHandleW(nullptr) + 0x824320` and `GetModuleHandleW(nullptr) + 0x824318`, which are the RVAs for `AuSettings::readEntry()` and `AuSettings::writeEntry()` respectively. `AuGlobal + 0x238` was necessary because the functions expect AuSettings passed as the first argument. Then the repurposed ReadEntry() was called for `/Account/Host`, and if that key exists, its value was passed to `new QLineEdit(host)`, which prefills the field so the user doesn't have to re-enter their server every time. 
Likewise, `WriteEntry()` was called in state 9 immediately after the user clicks the Login button and stores their entered server in `/Account/Host`. 
Matching the ABI for ReadEntry was difficult since it used unusual return registers and arguments, but eventually `void* settings, QString& result, const QString& key, QString* resultStorage` worked. WriteEntry was much easier.

Later, in state 10, which happens after the user clicks "Login", we redirect the actual stored hostnames in AuGlobal. This was as simple as doing `(AuGlobal + 0xXXX) = host` where XXX is the offset of the three hosts. I found that `0x150` = login, `0x160` = mail, `0x218` = scene. 
Compared to the original ASM-only mod, this was extremely simple. The ports were already hardcoded and handled by the game, so they did not need changing.

#### "Why use the game's registry functions if you can do it through Qt/WinAPI?"
I wanted to prove the concept that we can "steal" the game's existing imported and exported functions, which will help massively for future client modifications. If the .exe imports them, we can just do the IAT trick, if not but they're exported by the Au* DLLs, then we can use GetProcAddress on the mangled C++ symbol.

#### Other
ClientInterface.dll and its exports were added to the game's exe import table with CFF explorer's Import Adder, and the DLL was built with MSVC v140 linked against Qt 5.8.0 in VS2026. As of V0.0.8, the DLL build was switched from Debug to Release config for wider compatibility.
CFF explorer was also used to add code caves to the game exe and AuLoginClient13.dll+AuUtil13.dll in order to fit our SetState hook instructions.

Next, I wanted to port the Background.png replacement functionality to C++. This was extremely simple and just required the image object passed to SetState's context, and then we do `image->load(QString::fromLatin1("assets/Background.png"))`. Later, I also added `mainWindow->setWindowTitle("OpenShores");` and `mainWindow->setWindowIcon(QIcon("assets/OS_Icon.png"));` to this state (7) too.
For debugging, every ProcessState() case had a stateLog() call added to it, just logging which state was received. Timestamps were added to the log using `std::chrono`, and relevant helper functions were organized in a ClientStateHelpers.cpp file.

In state 5, which was placed early in the UI setup, I passed the RSI register through RDX as SetState's context, did `context + 0x30` which is where the scrolling text string lives, and cleared it with `textList->clear()`. I then simply added our own custom string with `textList->append()`. However, a quirk of the unmodified scrolling text was that certain lines were larger, bold, and centered. After some investigation, I found that the game checks for a `*` marker at the start of the line, removes it, and then applies special formatting to that line.

The default game avatar list only provides 6 slots, but the server is able to hold many more. Thus I added Previous and Next buttons to the avatar UI in state 11, which is the avatar UI setup post-login. This required walking the layout tree to align the buttons correctly, as the avatar slot menu is several levels deep within QGroupBoxes and QStackedLayouts.
