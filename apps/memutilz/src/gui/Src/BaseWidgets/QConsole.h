#ifndef WIDGETS_QCONSOLE_H
#define WIDGETS_QCONSOLE_H

#include <QPlainTextEdit>

class QConsole : public QPlainTextEdit {
    Q_OBJECT

public:
    /*
    explicit QConsole(QWidget* parent = nullptr);
    virtual ~QConsole() override = default;

    // Output helpers
    void print(const QString& text);
    void println(const QString& text = QString());
    void printError(const QString& text);

    // Prompt
    void setPrompt(const QString& prompt);
    QString prompt() const;

signals:
    // Emitted when user presses Enter on a command
    void commandEntered(const QString& command);

protected:
    // Input handling
    void keyPressEvent(QKeyEvent* event) override;

private:
    // Internal helpers
    void insertPrompt();
    QString currentCommand() const;
    void replaceCurrentCommand(const QString& text);
    bool cursorInEditableArea() const;

private:
    QString m_prompt = "> ";
    int m_promptPosition = 0;

    QStringList m_history;
    int m_historyIndex = -1;
    */
};
#endif