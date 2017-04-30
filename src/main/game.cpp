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

#include "main/game.hpp"

#include <cstddef>
#include <iostream>
#include "config.h"
#include "color/color.hpp"
#include "event/event_handler.hpp"
#include "input/input_handler.hpp"
#include "settings/game_settings.hpp"
#include "video/font.hpp"
#include "view/menu_view.hpp"
#include "view/puzzle_view.hpp"

Game::Game(int argc, char* argv[])
{
  m_video = VideoSystem::create();

  WindowSettings ws;
  ws.title = NONNY_TITLE;
  m_window = m_video->new_window(ws);
  m_renderer = m_video->new_renderer(*m_window);

  m_view_mgr = std::make_unique<ViewManager>(*m_video,
                                             *m_renderer,
                                             m_settings);

  auto view = std::make_shared<MenuView>(*m_view_mgr);
  m_view_mgr->push(view);
  m_view_mgr->resize(m_window->width(), m_window->height());
  
  /*//For now, start the game with a puzzle open
  auto pview = std::make_shared<PuzzleView>(*m_view_mgr,
                                            m_settings.puzzle_dir()
                                            + "easy/test.non");
  m_view_mgr->push(pview);
  m_view_mgr->resize(m_window->width(), m_window->height());*/
}

void Game::run()
{
  std::unique_ptr<InputHandler> input = InputHandler::create();
  std::unique_ptr<EventHandler> event = EventHandler::create();

  bool exit = false;
  std::size_t prev_ticks = event->get_ticks();
  std::size_t ticks = prev_ticks;
  unsigned elapsed = 0;
  while (!exit) {
    ticks = event->get_ticks();
    elapsed = ticks - prev_ticks;
    prev_ticks = ticks;
    
    input->update(elapsed);

    event->process(*input, *m_view_mgr);
    exit = m_view_mgr->empty();

    m_renderer->set_draw_color(default_colors::white);
    m_renderer->clear();

    m_view_mgr->update(elapsed, *input);
    m_view_mgr->draw(*m_renderer);
    
    m_renderer->present();
  }
}
