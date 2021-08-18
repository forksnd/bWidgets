#include <cassert>
#include <cmath>
#include <iostream>

#include "bwEvent.h"
#include "bwPainter.h"
#include "bwRange.h"
#include "bwStyle.h"

#include "screen_graph/Builder.h"
#include "screen_graph/Drawer.h"
#include "screen_graph/Node.h"

#include "bwScrollBar.h"

#include "bwScrollView.h"

namespace bWidgets {

bwScrollView::bwScrollView(bwScreenGraph::ContainerNode& node,
                           std::string_view identifier,
                           unsigned int _width,
                           unsigned int _height)
    : bwContainerWidget(node, _width, _height), identifier(identifier)
{
  auto scrollbar = std::make_unique<bwScrollBar>(17, _height);
  scrollbar_node = std::make_unique<bwScreenGraph::WidgetNode>();

  bwScreenGraph::Builder::setWidget(*scrollbar_node, std::move(scrollbar));
}

auto bwScrollView::getTypeIdentifier() const -> std::string_view
{
  return "bwScrollView";
}

auto bwScrollView::getVerticalScrollBar() const -> bwScrollBar&
{
  return static_cast<bwScrollBar&>(*scrollbar_node->Widget());
}

auto bwScrollView::getVerticalScrollbarRect(const bwStyle& style) const -> bwRectanglePixel
{
  bwRectanglePixel scroll_rectangle{rectangle};
  /* TODO hardcoded padding */
  const int padding = 4 * (int)style.dpi_fac;

  scroll_rectangle.xmin = scroll_rectangle.xmax - bwScrollView::getScrollbarWidth(style.dpi_fac) -
                          padding;
  scroll_rectangle.resize(-padding);

  return scroll_rectangle;
}

void bwScrollView::drawScrollBars(bwStyle& style)
{
  bwScrollBar& scrollbar = getVerticalScrollBar();

  validizeScrollValues();

  scrollbar.rectangle = getVerticalScrollbarRect(style);
  scrollbar.ratio = (rectangle.height() - 2) / float(content_rect.height());
  scrollbar.scroll_offset = vert_scroll;

  bwScreenGraph::Drawer::drawSubtree(*scrollbar_node, style);
}

void bwScrollView::draw(bwStyle& style)
{
  content_rect = node.ContentRectangle();

  bwPainter painter;

  painter.active_drawtype = bwPainter::DrawType::FILLED;
  painter.setActiveColor(base_style.background_color);
  painter.drawRectangle(rectangle);

  if (base_style.isBorderVisible()) {
    painter.active_drawtype = bwPainter::DrawType::OUTLINE;
    painter.setActiveColor(base_style.border_color);
    painter.drawRectangle(rectangle);
  }
  if (isScrollable()) {
    drawScrollBars(style);
  }
}

auto bwScrollView::matches(const bwWidget& other) const -> bool
{
  const bwScrollView* other_scroll_view = widget_cast<bwScrollView>(other);
  if (!other_scroll_view) {
    return false;
  }

  return identifier == other_scroll_view->identifier &&
         scrollbar_node->matches(*other_scroll_view->scrollbar_node);
}

auto bwScrollView::alwaysPersistent() const -> bool
{
  return true;
}

void bwScrollView::copyState(const bwWidget& from)
{
  bwWidget::copyState(from);

  const bwScrollView* other_scroll_view = widget_cast<bwScrollView>(from);
  if (!other_scroll_view) {
    return;
  }

  content_rect = other_scroll_view->content_rect;
  vert_scroll = other_scroll_view->vert_scroll;

  /* XXX hack to move the handler of the nested scrollbar. Instead the scrollbar should be in the
   * screen-graph. */
  scrollbar_node->moveState(*other_scroll_view->scrollbar_node);
}

void bwScrollView::validizeScrollValues()
{
  assert(isScrollable());

  bwRange<int>::clampValue(vert_scroll, 0, content_rect.height() - node.Rectangle().height());
}

auto bwScrollView::getScrollOffsetY() const -> int
{
  return vert_scroll;
}

auto bwScrollView::getContentBounds(float interface_scale) const -> bwRectanglePixel
{
  bwRectanglePixel bounds{rectangle};
  if (isScrollable()) {
    bounds.xmax -= getScrollbarWidth(interface_scale);
  }
  return bounds;
}

auto bwScrollView::isScrollable() const -> bool
{
  return (content_rect.height() > node.Rectangle().height()) || (vert_scroll != 0);
}

auto bwScrollView::getScrollbarWidth(float interface_scale) -> int
{
  return std::round(SCROLL_BAR_SIZE * interface_scale);
}

// ------------------ Handling ------------------

class bwScrollViewHandler : public bwScreenGraph::EventHandler {
 public:
  bwScrollViewHandler(bwScreenGraph::Node& node);
  ~bwScrollViewHandler() = default;

  void onMouseMove(bwEvent&) override;
  void onMouseEnter(bwEvent&) override;
  void onMouseLeave(bwEvent&) override;
  void onMousePress(bwMouseButtonEvent& event) override;
  void onMouseRelease(bwMouseButtonEvent& event) override;
  void onMouseClick(bwMouseButtonEvent& event) override;
  void onMouseDrag(bwMouseButtonDragEvent& event) override;
  void onMouseWheel(bwMouseWheelEvent& event) override;

  void onScrollbarMouseEnter(bwEvent& event) const;
  void onScrollbarMouseLeave(bwEvent& event) const;

  auto ScrollView() const -> bwScrollView&;

  auto isEventInsideScrollbar(const class bwEvent& event) const -> bool;

  void setScrollValue(int value);

 private:
  constexpr static int SCROLL_STEP_SIZE = 40;

  bool was_inside_scrollbar{false};
};

bwScrollViewHandler::bwScrollViewHandler(bwScreenGraph::Node& node)
    : bwScreenGraph::EventHandler(node)
{
}

auto bwScrollView::createHandler(bwScreenGraph::Node& node)
    -> std::unique_ptr<bwScreenGraph::EventHandler>
{
  return std::make_unique<bwScrollViewHandler>(node);
}

template<typename... _Args> using HandlerFunc = void (bwScreenGraph::EventHandler::*)(_Args&&...);

/* Could turn this into a general utility to forward events to different widgets. */
template<typename... _Args>
static void forwardEventToNode(bwScreenGraph::Node& to_node,
                               HandlerFunc<_Args&&...> handler_func,
                               _Args&&... __args)
{
  (to_node.eventHandler()->*handler_func)(std::forward<_Args>(__args)...);
}

template<typename... _Args>
static auto forwardEventToScrollbarIfInside(const bwScrollViewHandler& scrollview_handler,
                                            bwScreenGraph::Node& scrollbar_node,
                                            const class bwEvent& event,
                                            HandlerFunc<_Args&&...> handler_func,
                                            _Args&&... __args) -> bool
{
  if (scrollview_handler.isEventInsideScrollbar(event)) {
    forwardEventToNode<_Args&&...>(scrollbar_node, handler_func, std::forward<_Args>(__args)...);
    return true;
  }

  return false;
}

void bwScrollViewHandler::onMouseWheel(bwMouseWheelEvent& event)
{
  if (!ScrollView().isScrollable()) {
    return;
  }

  char direction_fac = 0;

  switch (event.getDirection()) {
    case bwMouseWheelEvent::Direction::UP:
      direction_fac = -1;
      break;
    case bwMouseWheelEvent::Direction::DOWN:
      direction_fac = 1;
      break;
  }

  setScrollValue(ScrollView().vert_scroll + (direction_fac * SCROLL_STEP_SIZE));

  event.swallow();
}

auto bwScrollViewHandler::ScrollView() const -> bwScrollView&
{
  assert(Widget<bwScrollView>());
  return *Widget<bwScrollView>();
}

auto bwScrollViewHandler::isEventInsideScrollbar(const bwEvent& event) const -> bool
{
  return ScrollView().isScrollable() &&
         ScrollView().scrollbar_node->Rectangle().isCoordinateInside(event.location.x,
                                                                     event.location.y);
}

void bwScrollViewHandler::onScrollbarMouseEnter(bwEvent& event) const
{
  forwardEventToNode<bwEvent&>(*ScrollView().scrollbar_node, &EventHandler::onMouseEnter, event);
}

void bwScrollViewHandler::onScrollbarMouseLeave(bwEvent& event) const
{
  forwardEventToNode<bwEvent&>(*ScrollView().scrollbar_node, &EventHandler::onMouseLeave, event);
}

void bwScrollViewHandler::onMouseMove(bwEvent& event)
{
  forwardEventToScrollbarIfInside<bwEvent&>(
      *this, *ScrollView().scrollbar_node, event, &EventHandler::onMouseMove, event);

  if (was_inside_scrollbar && !isEventInsideScrollbar(event)) {
    onScrollbarMouseLeave(event);
    was_inside_scrollbar = false;
  }
  else if (!was_inside_scrollbar && isEventInsideScrollbar(event)) {
    onScrollbarMouseEnter(event);
    was_inside_scrollbar = true;
  }
}

void bwScrollViewHandler::onMouseEnter(bwEvent& event)
{
  if (!was_inside_scrollbar && isEventInsideScrollbar(event)) {
    onScrollbarMouseEnter(event);
    was_inside_scrollbar = true;
  }
}

void bwScrollViewHandler::onMouseLeave(bwEvent& event)
{
  if (was_inside_scrollbar && !isEventInsideScrollbar(event)) {
    onScrollbarMouseLeave(event);
    was_inside_scrollbar = false;
  }
}

void bwScrollViewHandler::onMouseDrag(bwMouseButtonDragEvent& event)
{
  if (forwardEventToScrollbarIfInside<bwMouseButtonDragEvent&>(
          *this, *ScrollView().scrollbar_node, event, &EventHandler::onMouseDrag, event)) {
    setScrollValue(ScrollView().getVerticalScrollBar().scroll_offset);
    event.swallow();
  }
}

void bwScrollViewHandler::onMousePress(bwMouseButtonEvent& event)
{
  if (forwardEventToScrollbarIfInside<bwMouseButtonEvent&>(
          *this, *ScrollView().scrollbar_node, event, &EventHandler::onMousePress, event)) {
    event.swallow();
  }
}

void bwScrollViewHandler::onMouseRelease(bwMouseButtonEvent& event)
{
  if (forwardEventToScrollbarIfInside<bwMouseButtonEvent&>(
          *this, *ScrollView().scrollbar_node, event, &EventHandler::onMouseRelease, event)) {
    event.swallow();
  }
}

void bwScrollViewHandler::onMouseClick(bwMouseButtonEvent& event)
{
  if (forwardEventToScrollbarIfInside<bwMouseButtonEvent&>(
          *this, *ScrollView().scrollbar_node, event, &EventHandler::onMouseClick, event)) {
    setScrollValue(ScrollView().getVerticalScrollBar().scroll_offset);
    event.swallow();
  }
}

void bwScrollViewHandler::setScrollValue(int value)
{
  assert(ScrollView().isScrollable());

  ScrollView().vert_scroll = value;
  ScrollView().validizeScrollValues();
}

}  // namespace bWidgets
