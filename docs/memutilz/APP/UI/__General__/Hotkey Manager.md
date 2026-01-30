https://github.com/qt-creator/qt-creator/tree/master/src/plugins/coreplugin/actionmanager
https://doc.qt.io/qtcreator-extending/actionmanager.html

Yes — your described use case **is actually one of the strongest motivations** for introducing **some kind of centralized action/hotkey manager**, even if it's far lighter than Qt Creator's full ActionManager.

The exact pattern you're describing (**"goToAddress" doing different things depending on which view is focused**) is one of the most common pain points in multi-pane debuggers / reverse engineering tools (WinDbg, x64dbg, OllyDbg/x64dbg forks, IDA Pro, radare2+cutter, Binary Ninja, Cheat Engine's multiple windows, etc.).

Let's break it down realistically for your WinDbg-like + Cheat-Engine-scanner hybrid.

### Typical Shortcut Categories in Such Tools

| Category                  | Examples (WinDbg / x64dbg / Cheat Engine style)                  | Scope / Context                  | Conflicts likely? | Needs central manager? |
|---------------------------|------------------------------------------------------------------|----------------------------------|-------------------|------------------------|
| Debugger stepping          | F5=Go, F10=Step over, F11=Step into, F9=Run to cursor           | Only meaningful when debugging & disassembly/source has focus | Medium            | Yes — very convenient |
| View navigation            | Ctrl+G = Go to address, Ctrl+F = Find, Arrow/Page keys scrolling | Focused view (Disasm / Hex / Registers / Memory) | **High**          | **Strong yes**        |
| Memory scanner specific    | F3=Next scan, Ctrl+N=New scan, Space=Freeze selected, etc.      | Only when scanner tab/window active | Low–Medium        | Medium                |
| Global / always available  | Ctrl+O=Open process, Ctrl+S=Save table, F1=Help, Alt+F4=Quit    | Whole application                | Low               | Low–Medium            |
| Per-cheat hotkeys          | User-defined per address-list entry (most Cheat Engine usage)   | Usually when address list focused | Medium            | Separate system       |

The **highest value** pieces are exactly the ones you mentioned:

1. **Context-sensitive "logical commands"** like `goToAddress`, `find`, `copy`, `paste`, `select all`, `refresh`, `follow address`, `jump to RVA/VA`, `toggle breakpoint`, etc.
   - Same shortcut → different implementation depending on focused dock/widget.
   - This is **exactly** what Qt Creator's proxy + context system solves elegantly.
   - Without it → you either duplicate shortcuts everywhere (and fight ambiguity) or you make everything global and manually check `focusWidget()` / `qobject_cast<>()` in a giant switch — ugly and error-prone.

2. **Debugger stepping / execution control keys** (F5/F9/F10/F11/Shift+F11/etc.)
   - These should be disabled (or do nothing) when not debugging.
   - Should be disabled/re-routed when modal dialogs are open (attach dialog, symbol load, etc.).
   - Should **not** trigger when user is typing in a hex editor or expression evaluator.

### Is "goToAddress depending on focused view" good enough motivation?

**Yes — very much so**, and here's why it's worth the investment:

- Users of debuggers/RE tools **expect** Ctrl+G (or equivalent) to "just work" and take them to an address **in the current view**.
  - In disassembly → seek in disasm
  - In hex/memory dump → seek in hex
  - In registers/stack → follow value as address
  - In structure viewer → jump inside structure
- If you **don't** solve this centrally → you end up with:
  - Different shortcuts per view (user confusion)
  - Or duplicate shortcuts → Qt emits `activatedAmbiguously()` → nothing happens → user frustration
  - Or giant `if (auto* v = qobject_cast<DisasmView*>(focusWidget())) v->gotoAddress(); else if ...` in one central slot → maintenance nightmare as you add views

This pattern appears very often in real tools:
- x64dbg uses a very similar proxy-like system internally for many commands.
- Cutter/radare2 GUI went through several shortcut rewrites to get context-sensitive behavior right.

### Minimal architecture that gives you 80–90% of the value (without full Qt Creator complexity)

```text
Hotkey / Command system light (≈ 300–800 LOC)

1. CommandRegistry (singleton)
   - QMap<Id, CommandInfo>
   - CommandInfo = { id, description, defaultShortcut, currentShortcut, category }

2. ProxyAction per logical command
   - One QAction* proxy per command (added to ribbon / menus)
   - proxy->setShortcut(context-dependent or user-customized)
   - proxy->triggered → calls virtual "execute()" on active handler

3. Handler registration
   - Each major widget (DisasmView, HexView, MemoryScannerWidget, ...) registers itself as possible handler for some commands
   - registerHandler(Id commandId, QWidget* contextWidget, std::function<void()> executor)

4. Focus change handler
   - Install event filter on QApplication or central widget
   - On focus change → recompute active context (or just active widget)
   - For each proxy → find best matching handler → connect proxy to that executor (disconnect old)

5. Shortcut editing dialog
   - Simple QTreeView or QListView of all commands + QKeySequenceEdit
   - Save to QSettings

6. Memory-scanner hotkeys (separate / simpler)
   - Per-entry hotkeys → just dynamic QShortcut owned by AddressList widget
   - Global scanner actions (Next Scan, Undo Scan) → normal QAction in scanner tab, context = WidgetShortcut or WindowShortcut
```

**You don't need**:
- Full groups / insertion points / dynamic menus from plugins
- String-based Ids everywhere
- Multiple contexts per action
- Automatic shortcut conflict editor
- Touchbar / macOS special handling

**You do want**:
- The proxy + dynamic connect/disconnect on focus change
- One place to edit all application-level shortcuts
- Easy way to disable groups of shortcuts when not debugging

### Bottom line recommendation

**Worth implementing a thin command system?**  
**Yes — if you plan to have ≥ 5–7 views/docks that share similar logical operations** (go to, find, copy selection, follow, breakpoint, refresh, etc.).

**Not worth it if**:
- You keep everything very simple (one main disassembly + hex side-by-side, no separate scanner window)
- You're okay with view-specific shortcuts only
- You're fine debugging ambiguous shortcut issues manually

Start with **exactly the goToAddress / followAddress** case — implement it with a proxy QAction + focus-based switching.  
Once you have that working nicely, you'll probably want to do the same for 4–5 more common commands → and then the pattern pays for itself very quickly.

If you want, I can sketch more concrete code for the minimal proxy + focus-switch version (without groups/containers). Just say which commands feel most painful right now.