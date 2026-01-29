#pragma once

#include <QPointer>
#include <DockManager.h>

#include "../Utils/Id.h"

/**
 * @brief Owns and manages all visible dock widgets
 * 
 * CentralDockingArea is a wrapper around dockManager that
 * manages the ownership of all registered dock widgets. Closing
 * a dock widget either hides it or deletes it entirely depending
 * on whether DockWidgetDeleteOnClose is set for that CDockWidget.
 * This lets the caller decide which widgets are considered cheap, and
 * which are considered expensive (and thus not worth deleting).
 * If an expensive dock widget remains hidden for a long time
 * it could become a memory leak, but this is very very unlikely.
 */
class CentralDockingArea : public QWidget {
	Q_OBJECT

public:
	explicit CentralDockingArea(QWidget* parent = nullptr);
	virtual ~CentralDockingArea() override;

	QPointer<ads::CDockManager> dockManager() const { return _dockManager; }
	ads::CDockWidget* dockWidget(Utils::Id id) const;
	std::optional<Utils::Id> focusedDockId() const;
	
	bool contains(Utils::Id id) const { return _docks.contains(id); }
	std::optional<Utils::Id> idFromDock(const ads::CDockWidget* dockWidget) const;

	/**
	 * Adds the dock widget if the id is new, otherwise shows the existing one.
	 * Returns the managed dock widget (either the new one, or the existing one).
	 *
	 * @warning The returned pointer is only valid until the widget is removed
	 *          or closed with DockWidgetDeleteOnClose enabled.
	 */
	QPointer<ads::CDockWidget> addOrShowFloating(Utils::Id id, ads::CDockWidget* dockWidget);
	QPointer<ads::CDockWidget> addOrShowDocked(
		Utils::Id id,
		ads::CDockWidget* dockWidget,
		ads::CDockAreaWidget* container			= nullptr,
		ads::DockWidgetArea relativePosition	= ads::CenterDockWidgetArea
	);
	QPointer<ads::CDockWidget> addOrShowTabbed(
		Utils::Id id,
		ads::CDockWidget* dockWidget,
		ads::CDockAreaWidget* container			= nullptr
	);
	QPointer<ads::CDockWidget> addOrShowSidebar(
		Utils::Id id,
		ads::CDockWidget* dockWidget,
		ads::SideBarLocation side				= ads::SideBarLeft
	);

	/**
	 * Creates and shows a dock widget using the registered factory for the given id.
	 * Returns nullptr if no factory is registered for the aforementioned id.
	 */
	QPointer<ads::CDockWidget> addOrShowFactory(Utils::Id id);

	void showDock(Utils::Id id, bool bringToFront = true);
	void hideDock(Utils::Id id);
	void toggleDock(Utils::Id id);
	void activateDock(Utils::Id id); // show + raise + setFocus

	/// <summary>
	/// Removes from layout and map. Deletes the widget UNLESS
	/// DockWidgetDeleteOnClose is set (in which case widget deletes itself)
	/// </summary>
	/// <param name="id"> Dock Widget ID </param>
	/// <returns> Returns true if dock existed and deleted successfully </returns>
	bool remove(Utils::Id id);
	void removeAll();

	// Layout

	QStringList perspectiveNames() const;
	std::optional<QString> currentPerspectiveName() const;

	bool savePerspective(const QString& name);
	bool loadPerspective(const QString& name);
	bool deletePerspective(const QString& name);

	void setLayoutLocked(bool locked);
	bool isLayoutLocked() const;

	void createFactory(Utils::Id id, std::function<ads::CDockWidget*()> factory);
signals:
	void focusedDockWidgetChanged(std::optional<Utils::Id> prev, std::optional<Utils::Id> curr);
	void dockShown(Utils::Id id);
	void dockHidden(Utils::Id id);
	void dockRemoved(Utils::Id id);

	void perspectiveListChanged();
	void perspectiveLoaded(const QString& name);
private:
	QPointer<ads::CDockManager>								_dockManager;
	QHash<Utils::Id, QPointer<ads::CDockWidget>>			_docks;
	QHash<ads::CDockWidget*, Utils::Id>						_ids;
	QHash<QString, QByteArray>								_perspectives;
	std::optional<QString>									_currentPerspective;
	std::optional<Utils::Id>								_focusedDock;

	QHash<Utils::Id, std::function<ads::CDockWidget*>()>	_factories;
};