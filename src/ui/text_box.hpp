/* Nonny -- Play and create nonogram puzzles.
 * Copyright (C) 2017 Gregory Kikola.
 *
 * This file is part of Nonny.
 *
 * Nonny is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nonny is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nonny.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Written by Gregory Kikola <gkikola@gmail.com>. */

#ifndef NONNY_TEXT_BOX_HPP
#define NONNY_TEXT_BOX_HPP

#include "ui/control.hpp"

class Font;

class TextBox : public Control {
public:
  TextBox() = default;
  TextBox(const Font& font) : m_font(&font) { calc_size(); }

  void set_text(const std::string& text);
  
  using UIPanel::update;
  using UIPanel::draw;
  void update(unsigned ticks, InputHandler& input,
              const Rect& active_region) override;
  void draw(Renderer& renderer, const Rect& region) const override;
private:
  void calc_size();
  int pos_to_screen_coord(unsigned pos) const;
  unsigned screen_coord_to_pos(int x) const;
  
  const Font* m_font = nullptr;
  std::string m_text;
  unsigned m_cursor = 0;
  unsigned m_visible = 0;
  unsigned m_sel_start = 0;
  unsigned m_sel_length = 0;
  bool m_cursor_visible = true;
  unsigned m_cursor_duration = 0;
};

#endif