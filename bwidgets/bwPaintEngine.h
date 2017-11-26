#pragma once

#include "bwRectangle.h"

namespace bWidgets {

enum TextAlignment: unsigned int;

/**
 * \class bwPaintEngine
 * \brief Abstract class for drawing of geometry and text.
 *
 * bwPaintEngine mainly gives us two benefits:
 * * Abstracting away any drawing to the application, out of bWidgets.
 * * Support multiple drawing backends (e.g. OpenGL and Vulcan).
 *
 * Having support for multiple drawing backends is not only useful for
 * supporting multiple graphics libraries (as in OpenGL, DirectX, Vulcan,
 * etc.), but also to draw on different mediums. E.g. for drawing into textures
 * or exporting as HTML & CSS code.
 *
 * Creating a paint-engine is done by subclassing this class and
 * implementing the abstract functions.
 */
class bwPaintEngine
{
public:
	/**
	 * Setup function for application to prepare viewport (prepare matrices,
	 * clear background, etc.) for drawing.
	 *
	 * \note bWidgets doesn't call this itself, this is just a utility for
	 *       the application to use.
	 *
	 * \param rect The rectangle defining the viewport location and size.
	 * \param clear_color Color to be used for buffer clearing.
	 */
	virtual void setupViewport(
	        const bwRectanglePixel& rect,
	        const class bwColor& clear_color) = 0;

	/**
	 * The main polygon draw function which is used to draw all geometry of
	 * widgets.
	 * \param painter Provides all aditional information needed for drawing
	 *                (color, draw-style, etc.).
	 */
	virtual void drawPolygon(
	        const class bwPainter& painter,
	        const class bwPolygon& polygon) = 0;
	/**
	 * The main text draw function which is used to draw all text of widgets.
	 * \param painter Provides all aditional information needed for drawing
	 *                (color, draw-style, etc.).
	 * \param rect The bounding-box to draw the text into.
	 */
	virtual void drawText(
	        const class bwPainter& painter,
	        const std::string& text,
	        const bwRectanglePixel& rect,
	        const TextAlignment alignment) = 0;
};

} // namespace bWidgets
