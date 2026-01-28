https://github.com/uglide/QtConsole - old
https://github.com/gapost/qconsolewidget
https://github.com/WaiwaiTAN/QConsoleWidget

Usually subclass either QTextEdit or QPlainTextEdit (simpler and faster) then add features

QConsole is composed of 3 widgets:
1. QPlainTextEdit
	This does output handling and coloring with QSyntaxHighlighter
2. LineEditWithHistory
	This handles history (duh) but also autocomplete, that QConsole must know which commands are available
3. Label (could be button or other interactable)
	This is just for decoration

```
|------------------------------------------------------|
|                                                      |
|                                                      |
|                                                      |
|                                                      |
|                    Text Scroll Area                  |
|                                                      |
|                                                      |
|                                                      |
|------------------------------------------------------|
| Label: |               LineEdit                      |
|------------------------------------------------------|
```

For autocomplete, you need to have a working bridge between your UI and the debug backend, since the backend is what dictates which commands are available to you. The backend handles command interpretation, and the console simply queries the options through the bridge. This is how it get autocomplete recommendations. For the actual UI part of autocomplete, use QCompleter.

For colors, your best choice is probably `QSyntaxHighlighter`, it's a class that lets you define rules for coloring text blocks. It works line by line. Also, `QPlainTextEdit` supports **`ExtraSelection`**, which is good for temporary highlighting, like a cursor position, selection, or search highlight.

For a full terminal emulator, if you want scripting, use this
https://github.com/lxqt/qtermwidget