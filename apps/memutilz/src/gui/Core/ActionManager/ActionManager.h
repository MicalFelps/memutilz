#pragma once

#include <QObject>
#include <QAction>

namespace Core
{
class ActionManager : public QObject {
	Q_OBJECT
public:
	static ActionManager* instance();

	QAction* registerAction(
		const QString& id,
		const QString& text,
		const QIcon& icon = QIcon(),
		const QKeySequence& shortcut = QKeySequence(),
		const QString& statusTip = QString(),
		QWidget* parent = nullptr
	);

	QAction* action(const QString& id) const;
	bool trigger(const QString& id);
	bool contains(const QString& id) const;

signals:
	void actionTriggered(const QString& id);

private:
	explicit ActionManager(QObject* parent = nullptr);
	virtual ~ActionManager() override = default;

	static ActionManager* _instance;
	QHash<QString, QAction*> _actions;
};
} // namespace Core