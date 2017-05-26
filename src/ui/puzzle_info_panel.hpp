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

#ifndef NONNY_PUZZLE_INFO_PANEL_HPP
#define NONNY_PUZZLE_INFO_PANEL_HPP

#include <functional>
#include <memory>
#include "ui/image_button.hpp"
#include "ui/palette_panel.hpp"
#include "ui/puzzle_preview.hpp"
#include "ui/ui_panel.hpp"

class Font;
class Puzzle;
class Texture;

/*
 * Displays information about a puzzle that the user is solving and
 * holds some buttons to allow the user to access the menu or change
 * options.
 */
class PuzzleInfoPanel : public UIPanel {
public:
  PuzzleInfoPanel(Font& title_font, Font& info_font, Font& size_font,
                  Texture& ctrl_texture, unsigned max_width);
  PuzzleInfoPanel(Font& title_font, Font& info_font, Font& size_font,
                  Texture& ctrl_texture, unsigned max_width, Puzzle& puzzle);
  void attach_puzzle(Puzzle& puzzle);

  // Register callback functions
  typedef std::function<void()> Callback;
  void on_menu_open(Callback fn) { m_menu_button->register_callback(fn); }
  void on_zoom_in(Callback fn) { m_zoom_in_button->register_callback(fn); }
  void on_zoom_out(Callback fn) { m_zoom_out_button->register_callback(fn); }
  void on_hint_toggle(Callback fn) { m_hint_button->register_callback(fn); }
  void on_color_change(Callback fn) { m_color_selector.on_color_change(fn); }
  
  // Start/stop sliding animation
  void start_slide() { m_sliding = true; }
  void stop_slide() { m_sliding = false; }

  // Get currently selected color
  Color active_color() const;

  // Get/set time puzzle has been open
  unsigned time() const { return m_time; }
  void time(unsigned time) { m_time = time; }

  using UIPanel::update; //make all update and draw overloads visible
  using UIPanel::draw;
  void update(unsigned ticks, InputHandler& input,
              const Rect& active_region) override;

  void draw(Renderer& renderer, const Rect& region) const override;

  void move(int x, int y) override;

private:
  void setup_buttons();
  void retrieve_puzzle_info();
  void calculate_bounds();

  Font& m_title_font;
  Font& m_info_font;
  Font& m_size_font;
  Texture& m_ctrl_texture;

  Puzzle* m_puzzle = nullptr;
  std::string m_puzzle_title;
  std::string m_puzzle_author;
  std::string m_puzzle_size;

  PuzzlePreview m_preview;
  PalettePanel m_color_selector;
  std::shared_ptr<ImageButton> m_menu_button;
  std::shared_ptr<ImageButton> m_zoom_in_button;
  std::shared_ptr<ImageButton> m_zoom_out_button;
  std::shared_ptr<ImageButton> m_hint_button;

  bool m_sliding = false;
  unsigned m_time = 0;
  unsigned m_max_width = 0;
};

#endif
