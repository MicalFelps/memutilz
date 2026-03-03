#pragma once

#include <Utils/Id.h>

/**
 * Holds all semantic identifiers used by the main app.
 *
 * Prefixes used (naming convention):
 *	C_				-> Contexts (focus)
 *  CMD_			-> Commands / Action with shortcuts
 *	TAB_			-> RibbonBar Tabs 
 *	PANEL_			-> Panels (Groups)
 *  VIEW_			-> Semantic Widget Type
 *	SETTING_		-> Settings categories / pages
 * 
 * Hierarchy is indicated by dots in the string, for example
 *	"Module.Category.Item" or "Feature.SubFeature.Action"
 */

namespace Memutilz {
namespace Constants {

const Utils::Id DOCK_DUMMY = Utils::Id("DUMMY");

}  // namespace Constants
}  // namespace Memutilz