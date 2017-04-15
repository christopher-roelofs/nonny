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

#ifndef NONNY_VIEW_HPP
#define NONNY_VIEW_HPP

class InputHandler;
class Renderer;
class ViewManager;

class View {
public:
  View() { }
  View(unsigned width, unsigned height) : m_width(width), m_height(height) { }
  virtual ~View() { }
  
  virtual void update(unsigned ticks, InputHandler& input) = 0;
  virtual void draw(Renderer& renderer) = 0;
  virtual void resize(unsigned width, unsigned height);
protected:
  unsigned m_width = 0;
  unsigned m_height = 0;
};

#endif