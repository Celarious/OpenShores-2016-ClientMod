## Note: These notes were from before the C++ DLL migration, where all of the modification was done through ASM patching only. 

# Client patching notes
The three hostnames that needed replacement were:
- login.hazeron.net
- scene.hazeron.net
- mail.hazeron.net

Because `login.hazeron.net` was only called once with `QAbstractSocket::connectToHost()`, I directly injected the relevant port and user-entered IP in the relevant registers. 
However, `scene.hazeron.net` and `mail.hazeron.net` are called many times throughout the game, so I had to edit their values upstream. 
Because the domains are hardcoded at program startup, before the UI even loads, in SceneMasterName, and MailMasterName, I had to find a way to edit their live value after the user enters their target host/IP and presses login.
I found that the scene domain gets converted to a QString and is stored at `AuGlobal+218` for the lifetime of the program, and the mail one at `AuGlobal+160` and `AuGlobal+170` (same pointer at both addresses).
Therefore, in caves 1c and 1d, which execute after the user clicks "Login", I use `QString::operator=(const&)` to overwrite those objects with the user-entered host/IP.

I found that there were four total mentions of /Account/Name, which is where the program stores the user's username in the Windows registry for pre-filling on next client load.
The last two mentions were in `AuLoginClient13.dll`, and were unused. The first two were in the main .exe, and the first one is `AuSettings::readEntry()`, and the second is `AuSettings::writeEntry()`.
I decided to copy this pattern and create a new "Host" key in the same Shores of Hazeron registry folder, with the host key being read (if it exists) at startup, and pre-filled with our custom QLineEdit IP input field.
When the user clicks login, their entered IP is saved using `AuSettings::writeEntry()`, and the key can be seen in plaintext in `Computer\HKEY_CURRENT_USER\SOFTWARE\Software Engineering\Shores of Hazeron\Account\Host`.
For example, if the user enters play.openshores.net as their IP, the registry key `Computer\HKEY_CURRENT_USER\SOFTWARE\Software Engineering\Shores of Hazeron\Account\Host` will be created with value "play.openshores.net".

The background image hijack was extremely simple. Although I was unable to find the raw image data of the original background image, changing it was as simple as redirecting the data source ASCII string that gets passed to `QString::fromAscii_helper`, which took a grand total of 3 instructions + the bytes to store the null-terminated Background.png string itself. Because the background image was changed from an internal embedded resource to a filesystem location, this allows the user to easily customize it by replacing Background.png with whatever they want their background to be.

### "Where is cave1b?"

I originally did all the post-login injection logic there, however, I realized too late that I had hijacked the original instructions too early, meaning it ran before the user ever presses Login. This made it useless, so I restored the original instructions and skipped straight to cave1c closer to the connection logic.

### Decomp

All debugging, testing and patching was done with x64dbg, meaning no decompilation was done, only instruction and byte modification/insertion. I'm not familiar enough with Ghidra to start doing that, sorry!
