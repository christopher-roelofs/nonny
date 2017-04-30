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

#include "ui/dialog.hpp"

#include <algorithm>
#include <iterator>
#include "input/input_handler.hpp"

void Dialog::add_control(ControlPtr control)
{
  m_controls.push_back(control);
  m_focused = m_controls.end();
  m_need_reposition = true;
}

void Dialog::focus_prev()
{
  if (m_focused != m_controls.end()
      && (*m_focused)->can_focus()
      && !(*m_focused)->has_focus())
    give_focus();
  else {
    remove_focus();

    auto pred = [](ControlPtr& p) { return p->can_focus(); };
    auto result = std::make_reverse_iterator(m_focused);
    result = std::find_if(result, m_controls.rend(), pred);
    if (result == m_controls.rend())
      result = std::find_if(m_controls.rbegin(), m_controls.rend(), pred);
    m_focused = result.base();

    if (m_focused != m_controls.begin())
      --m_focused;

    give_focus();
  }
}

void Dialog::focus_next()
{
  if (m_focused != m_controls.end()
      && (*m_focused)->can_focus()
      && !(*m_focused)->has_focus())
    give_focus();
  else {
    remove_focus();
    ++m_focused;

    auto pred = [](ControlPtr& p) { return p->can_focus(); };
    auto result = std::find_if(m_focused, m_controls.end(), pred);
    if (result == m_controls.end())
      result = std::find_if(m_controls.begin(), m_controls.end(), pred);
    m_focused = result;

    give_focus();
  }
}

void Dialog::update(unsigned ticks, InputHandler& input,
                    const Rect& active_region)
{
  if (m_need_reposition) {
    m_need_reposition = false;
    position_controls();
  }

  //change control focus with tab key
  if (input.was_key_pressed(Keyboard::Key::tab)) {
    if (input.is_key_down(Keyboard::Key::lshift)
        || input.is_key_down(Keyboard::Key::rshift))
      focus_prev();
    else
      focus_next();
  }
  
  for (auto& c : m_controls)
    c->update(ticks, input, active_region);
}

void Dialog::draw(Renderer& renderer, const Rect& region) const
{
  for (auto& c : m_controls)
    c->draw(renderer, region);
}

void Dialog::move(int x, int y)
{
  int old_x = m_boundary.x(), old_y = m_boundary.y();
  UIPanel::move(x, y);

  for (auto& c : m_controls)
    c->scroll(x - old_x, y - old_y);
}

void Dialog::give_focus()
{
  if (m_focused != m_controls.end() && (*m_focused)->can_focus())
    (*m_focused)->give_focus();
}

void Dialog::remove_focus()
{
  if (m_focused != m_controls.end())
    (*m_focused)->remove_focus();
}
