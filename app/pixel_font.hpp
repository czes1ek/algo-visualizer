#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <string_view>

namespace algoviz::app {

/// Tiny built-in 5x7 bitmap font, so the app ships with no font files.
/// Draws upper-case text as quads into a Triangles vertex array.
inline constexpr float kGlyphW = 5.f;
inline constexpr float kGlyphH = 7.f;

/// Appends text at (x, y). `scale` is the size of one font pixel. Returns the end x.
float drawText(sf::VertexArray& out, std::string_view text, float x, float y, float scale, sf::Color color);

/// Appends a solid rectangle.
void drawRect(sf::VertexArray& out, float x, float y, float w, float h, sf::Color color);

}  // namespace algoviz::app
