#pragma once

#include "screen_graph/PersistentPtr.h"

namespace bWidgets {

class bwWidget;
namespace bwScreenGraph {
class Node;
}

/**
 * \brief Important state information for the screen-graph.
 *
 * Blender used to manage special states of widgets (e.g. hovered, active,
 * etc.) mostly using per widget flags. To query these special widgets (i.e.
 * the currently hovered widget), a lookup had to be performed.
 * Rather than requiring lookups, bWidgets uses this context data-struture for
 * access to special state widgets.
 */
class bwContext {
 public:
  /** The currently hovered node (if any). */
  bwScreenGraph::PersistentNodePtr hovered = nullptr;
  /** The currently active widget which gets any input (if any). */
  bwScreenGraph::PersistentNodePtr active = nullptr;
};

}  // namespace bWidgets
