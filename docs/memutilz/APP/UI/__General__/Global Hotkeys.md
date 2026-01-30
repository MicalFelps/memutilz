Yes — **it is possible** to make your memory scanner's hotkeys (e.g. Next Scan, Undo Scan, Freeze, Add to list, etc.) work **even when the game window is focused** and your tool's own window is **not focused** (or even minimized / in the background). This is a very common requirement in Cheat Engine-style tools on Windows, and Linux can achieve similar behavior, but it requires a different approach than standard Qt shortcuts.

### Why Standard Qt Shortcuts Won't Work Here

Qt's `QShortcut` / `QAction` with `Qt::ApplicationShortcut` or `Qt::WindowShortcut` only receives key events when **your application (or one of its windows) has keyboard focus**.  
- If the game (especially a fullscreen game) has focus → your app gets **no** key events at all.  
- i3wm doesn't change this fundamental rule; it manages window tiling, focus-follows-mouse (or click-to-focus), but **keyboard events still go only to the focused window** (or to i3 itself if the key is bound in `~/.config/i3/config`).

So you **cannot** rely on normal Qt key handling for this scenario.

### Realistic Solutions (from most recommended to more involved)

1. **Best & Most Practical: Use a Third-Party Global Hotkey Library (Cross-Platform Friendly)**

   The cleanest way in a Qt application is to integrate a library that registers **true system-wide hotkeys** using platform-native APIs (X11/XCB on Linux, WinAPI on Windows, Carbon/AppKit on macOS).

   **Recommended library: QHotkey**  
   - GitHub: https://github.com/Skycoder42/QHotkey  
   - It's specifically made for Qt applications  
   - Works on Linux (X11), Windows, macOS  
   - Registers hotkeys globally → receives presses even when your app is **not focused** or minimized  
   - Simple API:  
     ```cpp
     QHotkey *hotkey = new QHotkey(QKeySequence("Ctrl+Shift+F3"), true, this); // true = enabled
     connect(hotkey, &QHotkey::activated, this, &MyScanner::onNextScan);
     ```
   - On Linux it uses X11/XCB under the hood (works fine under i3wm).

   Other alternatives:
   - **kglobalaccel** (if you're willing to depend on KDE Frameworks — very reliable on Linux)
   - **libinput** / raw XCB grabbing (more work, but no extra deps)

   This is what most cross-platform tools (including some Cheat Engine alternatives) end up using when they need background hotkeys.

2. **Linux-Specific: Register Hotkeys via i3wm Itself (Very Clean for i3 Users)**

   Since you use **i3**, the simplest and most reliable way for many users is:
   - **Don't** grab the hotkeys in your Qt app at all.
   - Instead, bind them in your i3 config to send a message / command to your running scanner process.

   Example `~/.config/i3/config`:
   ```
   # Bind global hotkeys that trigger your scanner (even when game is focused)
   bindsym Ctrl+Shift+F3 exec --no-startup-id qdbus org.yourapp.MemoryScanner /Scanner triggerNextScan
   bindsym Ctrl+Shift+F4 exec --no-startup-id qdbus org.yourapp.MemoryScanner /Scanner triggerFreeze
   bindsym Ctrl+Shift+F5 exec --no-startup-id qdbus org.yourapp.MemoryScanner /Scanner addSelectedAddress
   ```

   In your Qt app, expose a **DBus interface** (very easy with `QDBusConnection`):
   ```cpp
   // In main.cpp or a manager class
   QDBusConnection::sessionBus().registerObject("/Scanner", this,
       QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllProperties);
   ```

   Then just add public slots like:
   ```cpp
   public slots:
       void triggerNextScan() { /* do scan */ }
       void triggerFreeze()   { /* freeze selected */ }
   ```

   Advantages:
   - No extra libraries
   - Works perfectly under i3 (and most WMs)
   - i3 handles the grabbing → no conflicts with games
   - Users can easily customize / disable in their i3 config

   Downsides:
   - Only works on Linux (you'd still need QHotkey or similar for Windows/macOS)
   - Requires DBus (but Qt has excellent support)

3. **Fallback / More Complex: Grab Keys at Xlib / XCB Level**

   You can bypass Qt and directly use XCB to grab keys globally (similar to how volume keys or media hotkeys work).  
   This is what QHotkey does internally anyway.  
   It's doable but error-prone (need to handle modifiers, keycode vs keysym, ungrab on exit, multi-monitor, etc.). Avoid if possible.

### Quick Comparison for Your Use Case

| Method               | Works when game focused? | Extra deps?       | i3 friendly? | Cross-platform? | Effort |
|----------------------|---------------------------|-------------------|--------------|------------------|--------|
| Standard Qt QShortcut | No                        | None              | Yes          | Yes              | Low    |
| QHotkey library       | Yes                       | Small lib         | Yes          | Yes              | Medium |
| i3 config + DBus      | Yes                       | None (QtDBus)     | Excellent    | Linux only       | Low–Medium |
| Raw XCB grab          | Yes                       | None              | Yes          | Linux only       | High   |

**My recommendation for you (i3 user on Linux):**
- Start with **i3 + DBus** — it's the most "native" feeling under i3, zero conflicts, and users can remap easily.
- If you want Windows/macOS support later or prefer everything inside the app → add **QHotkey** (you can even make it optional / #ifdef Linux use DBus fallback).

Many Cheat Engine users on Linux/Wine/Proton do something similar (either DBus-style IPC or external hotkey tools like sxhkd or i3 binds). GameConqueror (the native Cheat-Engine-like tool on Linux) doesn't have built-in global hotkeys AFAIK, so people often script around it the same way.

If you tell me whether you want to support only Linux/i3 for now or cross-platform from the start, I can give more concrete code snippets.

---

Since you want **cross-platform support** (Windows, Linux including X11-based desktops like i3, and macOS) and you're okay with dependencies, the **ideal and most practical solution** in 2025–2026 for a Qt-based Cheat Engine-style tool is to use the **QHotkey library** (by Skycoder42).

This is the go-to choice for almost all modern Qt applications needing true global/system-wide hotkeys (i.e., hotkeys that trigger even when your app is **not focused**, minimized, or in the background — exactly what you need for scanner actions while playing a game).

### Why QHotkey Is the Best Fit Right Now

- **Actively referenced and used** in recent Qt projects (still appears in discussions, ports, and integrations in 2025).
- Supports **Qt5 and Qt6** (Qt6 requires ≥6.2.0).
- True cross-platform:
  - **Windows** — uses `RegisterHotKey` (native, reliable, low overhead).
  - **Linux (X11)** — uses X11/XCB grabbing (works perfectly on i3, XFCE, etc.).
  - **macOS** — uses Carbon/AppKit APIs.
- Simple, Qt-native API (QObject-based, signals/slots).
- Small footprint — just add a few source files or use it as a submodule/subproject.
- Handles most common modifiers + keys (Ctrl, Alt, Shift, Win/Cmd, F-keys, letters, numbers, etc.).
- The hotkey is **consumed** (not passed to the foreground app/game), which is usually what you want for cheats/scans.
- Limitations are well-documented (e.g., some special keys like Delete have platform quirks; no Wayland support yet — see below).

Alternatives I considered (but QHotkey wins for your case):

| Library / Approach          | Cross-Platform? | Qt Integration Quality | Maintenance (2025+) | Wayland? | Notes |
|-----------------------------|------------------|--------------------------|----------------------|----------|-------|
| QHotkey (Skycoder42)        | Yes             | Excellent (Qt-style)    | Good (still referenced) | No      | Winner — simple, reliable |
| QHotkeys (mrousavy fork)    | Yes             | Good                    | Lower activity      | No      | Fork with some extras, but less used |
| KGlobalAccel (KDE)          | Linux-only mostly | Very good on KDE        | Active (KDE)        | Partial | Too KDE-specific, pulls Frameworks |
| Raw platform APIs           | Yes (manual)    | Poor (lots of #ifdefs)  | N/A                 | Varies  | High effort, error-prone |
| QxtGlobalShortcut (old)     | Yes             | Dated                   | Dead                | No      | Abandoned years ago |
| DBus + i3/sxhkd binds       | Linux-only      | N/A                     | N/A                 | Yes     | Great Linux fallback, but not cross-platform |

### How to Integrate QHotkey (Quick Steps)

1. **Get the library**:
   - Clone it: `git clone https://github.com/Skycoder42/QHotkey.git`
   - Or add as git submodule: `git submodule add https://github.com/Skycoder42/QHotkey.git 3rdparty/QHotkey`
   - (Recommended: use as submodule so you can pin a commit if needed.)

2. **Add to your .pro / CMake**:
   - **qmake (.pro)**:
     ```
     include(3rdparty/QHotkey/QHotkey.pri)
     ```
   - **CMake**:
     ```cmake
     add_subdirectory(3rdparty/QHotkey)
     target_link_libraries(yourapp PRIVATE QHotkey)
     ```

3. **Usage example** (in your scanner/manager class):
   ```cpp
   #include <QHotkey>

   // In constructor or init
   QHotkey *nextScanHotkey = new QHotkey(
       QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F3),  // or "Ctrl+Shift+F3"
       true,  // enabled by default
       this
   );

   connect(nextScanHotkey, &QHotkey::activated, this, [this]() {
       // Your scanner logic — runs even if game is focused!
       performNextScan();
   });

   // Optional: disable when not attached to a process
   void onProcessDetached() {
       nextScanHotkey->setEnabled(false);
   }
   ```

   - You can create one per scanner action (Next, Undo, Freeze, Add address, etc.).
   - Make them user-configurable: store QKeySequence in QSettings, recreate QHotkey on change.

4. **Important Gotchas & Tips**
   - **No Wayland support** — Wayland security model prevents global key grabs (same for all apps/libs). On modern distros (Fedora, Ubuntu 24.04+ defaulting to Wayland):
     - Many users still run X11 (especially gamers).
     - Provide a fallback message: "Global hotkeys require X11 — switch via login screen."
     - Some users use `xwayland` tricks or external tools (sxhkd), but it's not reliable.
   - Test on all platforms — numpad keys, media keys, or special chars can behave differently.
   - Conflicts: If user binds something the OS uses (e.g., Win+L on Windows), it may fail silently — catch `QHotkey::registeredChanged(bool)` signal.
   - Minimize hotkey count — 5–10 is plenty for scanner (users can remap in your settings).

### Bonus: Hybrid for Maximum Coverage

If you want perfect Linux support (including Wayland users):
- Use **QHotkey** as primary (cross-platform).
- On Linux only: offer optional DBus interface + instructions for i3/sway/hyprland/sxhkd users to bind keys to `qdbus` calls (as I described before).
- This covers 95%+ of users without much extra code.

This setup is what most cross-platform cheat/memory tools (and overlay tools, macro recorders, etc.) converge on when they need background hotkeys. It's far simpler than rolling your own native wrappers.

If you run into build issues or want help with CMake/qmake setup, share your Qt version and build system — I can give exact snippets.