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

#include "view/file_view.hpp"

#include <functional>
#include <vector>
#include "color/color.hpp"
#include "input/input_handler.hpp"
#include "save/save_manager.hpp"
#include "settings/game_settings.hpp"
#include "ui/tooltip.hpp"
#include "video/renderer.hpp"
#include "view/message_box_view.hpp"
#include "view/view_manager.hpp"

namespace stdfs = std::experimental::filesystem;

const Color background_color(123, 175, 212);

constexpr int path_spacing = 8;
constexpr int button_spacing = 16;
constexpr int panel_spacing = 32;

std::string FileView::s_last_open_path;
std::string FileView::s_last_save_path;

FileView::FileView(ViewManager& vm, Mode mode)
  : View(vm), m_mode(mode), m_cur_path(m_paths.end())
{
  load_resources();
  open_default_dir();
}

FileView::FileView(ViewManager& vm, Mode mode,
                   int width, int height)
  : View(vm, width, height), m_mode(mode), m_cur_path(m_paths.end())
{
  load_resources();
  open_default_dir();
  resize(width, height);
}

void FileView::update(unsigned ticks, InputHandler& input)
{
  //see if mouse is hovering over or pressing a directory link
  if (m_cur_path != m_paths.end()) {
    if (input.was_mouse_button_pressed(Mouse::Button::left)
        || input.was_mouse_moved()) {
      int x = m_path_start.x(), y = m_path_start.y();
      int width, height;
      int index = 0;
      bool set_cursor = false;
      for (const auto& e : *m_cur_path) {
        if (index < m_path_collapse_start || index >= m_path_collapse_end) {
          m_filename_font->text_size(e.string(), &width, &height);
          if (Rect(x, y, width, height)
              .contains_point(input.mouse_position())) {
            if (input.was_mouse_button_pressed(Mouse::Button::left))
              open_subdir(index);
            set_cursor = true;
          }
          x += width + path_spacing;
        } else if (index == m_path_collapse_start) {
          m_filename_font->text_size("...", &width, nullptr);
          x += width + path_spacing;
        }

        if (index <= m_path_collapse_start || index >= m_path_collapse_end) {
          m_filename_font->text_size(">", &width, nullptr);
          x += width + path_spacing;
        }
        ++index;
      }

      if (set_cursor && input.cursor() != Mouse::Cursor::hand)
        input.set_cursor(Mouse::Cursor::hand);
      if (!set_cursor && input.cursor() != Mouse::Cursor::arrow)
        input.reset_cursor();
    }
  }

  //scroll with mouse wheel
  int mwheel_scroll = input.vert_mouse_wheel_scroll();
  if (mwheel_scroll < 0)
    m_file_selection.smooth_scroll_down();
  else if (mwheel_scroll > 0)
    m_file_selection.smooth_scroll_up();

  // Adding some hotkeys to be used by devices that map gamepads keys

  if (input.was_key_pressed(Keyboard::Key::letter_h))
      home();

  if (input.was_key_pressed(Keyboard::Key::letter_s))
      open_saved();

  if (input.was_key_pressed(Keyboard::Key::backspace))
      up();

  if (input.was_key_pressed(Keyboard::Key::tab)) {
    bool back = input.is_key_down(Keyboard::Key::lshift)
      || input.is_key_down(Keyboard::Key::rshift);
    switch_focus(!back);
  }

  if (!m_filename_box->has_focus()) {
    if (input.was_key_pressed(Keyboard::Key::left)
        || input.was_key_pressed(Keyboard::Key::kp_left)) {
      back();
    } else if (input.was_key_pressed(Keyboard::Key::right)
               || input.was_key_pressed(Keyboard::Key::kp_right))
      forward();

    if (input.was_key_pressed(Keyboard::Key::up)
        || input.was_key_pressed(Keyboard::Key::down))
      clear_focus();
  }

  if (input.was_key_pressed(Keyboard::Key::escape))
    m_mgr.schedule_action(ViewManager::Action::open_menu);

  for (auto& c : m_controls)
    c->update(ticks, input);

  m_file_selection.update(ticks, input);

  //make sure file selection panel was not scrolled past its start
  UIPanel& main = m_file_selection.main_panel();
  if (main.boundary().y() > m_file_selection.boundary().y())
    main.move(main.boundary().x(), m_file_selection.boundary().y());

  if (m_need_path_change) {
    m_need_path_change = false;
    open_path(stdfs::path(m_selected_path));
    m_selected_path = "";
  }
}

void FileView::draw(Renderer& renderer)
{
  renderer.set_draw_color(background_color);
  renderer.fill_rect(Rect(0, 0, m_width, m_height));

  renderer.set_draw_color(default_colors::black);
  if (m_cur_path != m_paths.end()) {
    int x = m_path_start.x(), y = m_path_start.y();
    int index = 0;
    for (auto& e : *m_cur_path) {
      Rect r;
      if (index != 0
          && (index <= m_path_collapse_start
              || index >= m_path_collapse_end)) {
        r = renderer.draw_text(Point(x, y), *m_filename_font, ">");
        x += r.width() + path_spacing;
      }
      if (index == m_path_collapse_start
          && index < m_path_collapse_end) { //collapsed - draw ellipsis
        r = renderer.draw_text(Point(x, y), *m_filename_font, "...");
        x += r.width() + path_spacing;
      } else if (index < m_path_collapse_start
                 || index >= m_path_collapse_end) { //not collapsed
        r = renderer.draw_text(Point(x, y), *m_filename_font, e.string());

        //draw underline
        renderer.draw_line(Point(x, y + r.height()),
                           Point(x + r.width(), y + r.height()));
        x += r.width() + path_spacing;
      }

      ++index;
    }
  }

  for (auto& c : m_controls)
    c->draw(renderer);

  renderer.set_draw_color(default_colors::white);
  renderer.fill_rect(m_file_selection.boundary());
  m_file_selection.draw(renderer);

  draw_tooltips(renderer);
}

void FileView::resize(int width, int height)
{
  View::resize(width, height);

  //position navigation buttons
  int x = width - panel_spacing - m_forward_button->boundary().width();
  int y = panel_spacing;
  m_forward_button->move(x, y);

  x -= button_spacing + m_back_button->boundary().width();
  m_back_button->move(x, y);

  x -= button_spacing + m_up_button->boundary().width();
  m_up_button->move(x, y);

  x -= button_spacing + m_saved_button->boundary().width();
  m_saved_button->move(x, y);

  x -= button_spacing + m_home_button->boundary().width();
  m_home_button->move(x, y);

  x -= button_spacing + m_menu_button->boundary().width();
  m_menu_button->move(x, y);

  x = panel_spacing;

  //vertically center path text
  int text_ht = 0;
  m_filename_font->text_size(">", nullptr, &text_ht);
  m_path_start = Point(x, y);
  m_path_start.y() += m_menu_button->boundary().height() / 2 - text_ht / 2;

  collapse_path();

  x = panel_spacing;
  y += m_menu_button->boundary().height() + button_spacing;

  //position file selection panel
  int new_width = m_width - 2 * panel_spacing;
  int new_height = m_height - 2 * panel_spacing - button_spacing
    - m_up_button->boundary().height()
    - m_open_button->boundary().height() - button_spacing;
  UIPanel& panel = m_file_selection.main_panel();
  panel.resize(new_width, panel.boundary().height());
  m_file_selection.move(x, y);
  m_file_selection.resize(new_width, new_height);
  panel.move(panel.boundary().x(), m_file_selection.boundary().y());
  y += m_file_selection.boundary().height() + button_spacing;

  m_filename_box->move(x, y);
  m_filename_box->resize(m_width - m_open_button->boundary().width()
                         - 2 * panel_spacing - button_spacing,
                         m_filename_box->boundary().height());
  m_open_button->move(m_width
                      - m_open_button->boundary().width()
                      - panel_spacing, y);
}

void FileView::load_resources()
{
  using namespace std::placeholders;

  const GameSettings& settings = m_mgr.game_settings();
  VideoSystem& vs = m_mgr.video_system();

  std::string font_file = settings.font_dir()
    + settings.filesystem_separator() + "FreeSans.ttf";
  std::string nav_texture_file = settings.image_dir()
    + settings.filesystem_separator() + "nav.png";
  std::string icon_texture_file = settings.image_dir()
    + settings.filesystem_separator() + "file.png";
  m_filename_font = vs.new_font(font_file, 18);
  m_info_font = vs.new_font(font_file, 16);
  m_control_font = vs.new_font(font_file, 24);
  m_nav_texture = vs.load_image(m_mgr.renderer(), nav_texture_file);
  m_file_icons_texture = vs.load_image(m_mgr.renderer(), icon_texture_file);

  m_menu_button = std::make_shared<ImageButton>(*m_nav_texture, 0);
  m_home_button = std::make_shared<ImageButton>(*m_nav_texture, 1);
  m_saved_button = std::make_shared<ImageButton>(*m_nav_texture, 2);
  m_up_button = std::make_shared<ImageButton>(*m_nav_texture, 3);
  m_back_button = std::make_shared<ImageButton>(*m_nav_texture, 4);
  m_forward_button = std::make_shared<ImageButton>(*m_nav_texture, 5);

  std::string open_label = m_mode == Mode::open ? "Load" : "Save";
  m_open_button = std::make_shared<Button>(*m_control_font, open_label);

  m_menu_button->register_callback([this]() {
      m_mgr.schedule_action(ViewManager::Action::open_menu); });
  m_home_button->register_callback(std::bind(&FileView::home, this));
  m_saved_button->register_callback(std::bind(&FileView::open_saved, this));
  m_up_button->register_callback(std::bind(&FileView::up, this));
  m_back_button->register_callback(std::bind(&FileView::back, this));
  m_forward_button->register_callback(std::bind(&FileView::forward, this));

  m_open_button->register_callback(std::bind(&FileView::open_file, this, ""));

  m_filename_box = std::make_shared<TextBox>(*m_control_font);

  auto fsv = std::make_shared<FileSelectionPanel>(m_mgr.save_manager(),
                                                  *m_filename_font,
                                                  *m_info_font,
                                                  *m_file_icons_texture);
  fsv->on_dir_change([this](const std::string& p)
                     { m_selected_path = p; m_need_path_change = true; });
  fsv->on_file_open(std::bind(&FileView::open_file, this, _1));
  fsv->on_file_select([this](const std::string& f)
                      { handle_selection_change(); });
  m_file_selection.attach_panel(fsv);

  if (m_mode == Mode::save)
    m_filename_box->give_focus();

  m_controls = { m_menu_button,
                 m_home_button,
                 m_saved_button,
                 m_up_button,
                 m_back_button,
                 m_forward_button,
                 m_filename_box,
                 m_open_button };
}

void FileView::open_path(const stdfs::path& p)
{
  if (m_cur_path == m_paths.end()) {
    m_paths.push_back(p);
    m_cur_path = m_paths.end() - 1;
  } else if (p != *m_cur_path) {
    m_paths.erase(m_cur_path + 1, m_paths.end());
    m_paths.push_back(p);
    m_cur_path = m_paths.end() - 1;
  }

  handle_directory_change();
}

void FileView::home()
{
  open_path(stdfs::canonical(stdfs::path(m_mgr.game_settings().puzzle_dir())));
}

void FileView::open_saved()
{
  stdfs::path dir = m_mgr.game_settings().saved_puzzle_dir();
  if (!stdfs::exists(dir))
    stdfs::create_directories(dir);
  dir = stdfs::canonical(dir);

  open_path(dir);
}

void FileView::back()
{
  if (m_cur_path != m_paths.begin())
    --m_cur_path;
  handle_directory_change();
}

void FileView::forward()
{
  if (m_cur_path != m_paths.end()
      && m_cur_path + 1 != m_paths.end())
    ++m_cur_path;
  handle_directory_change();
}

void FileView::up()
{
  if (m_cur_path != m_paths.end() && m_cur_path->has_parent_path())
    open_path(m_cur_path->parent_path());
}

void FileView::switch_focus(bool fwd) {
  bool focus_changed = false;
  for (int i = 0; i < static_cast<int>(m_controls.size()); ++i) {
    if (m_controls[i]->has_focus()) {
      m_controls[i]->remove_focus();
      int next;
      if (i == 0 && !fwd)
        next = m_controls.size() - 1;
      else if (i == static_cast<int>(m_controls.size()) - 1 && fwd)
        next = 0;
      else
        next = fwd ? i + 1 : i - 1;
      m_controls[next]->give_focus();
      focus_changed = true;
      break;
    }
  }

  if (!focus_changed) {
    if (fwd)
      m_menu_button->give_focus();
    else
      m_filename_box->give_focus();
  }
}

void FileView::clear_focus()
{
  for (auto& c : m_controls)
    c->remove_focus();
}

void FileView::open_default_dir()
{
  if (m_mode == Mode::open) {
    if (!s_last_open_path.empty())
      open_path(stdfs::path(s_last_open_path));
    else
      home();
  } else if (m_mode == Mode::save) {
    if (!s_last_save_path.empty())
      open_path(stdfs::path(s_last_save_path));
    else
      open_saved();
  }
}

void FileView::open_file(const std::string& filename)
{
  if (filename.empty()) {
    if (m_mode == Mode::open) {
      FileSelectionPanel& panel
        = dynamic_cast<FileSelectionPanel&>(m_file_selection.main_panel());
      panel.open_selection();
    } else if (m_cur_path != m_paths.end()) {
      stdfs::path p = *m_cur_path / m_filename_box->get_text();
      if (p.has_filename()) {
        if (p.extension().empty())
          p.replace_extension(".non");
        if (!p.empty())
          open_file(p.string());
      }
    }
  } else {
    if (m_mode == Mode::open)
      m_mgr.schedule_action(ViewManager::Action::load_puzzle, filename);
    else {
      auto save = [this, filename]() {
        m_mgr.schedule_action(ViewManager::Action::save_puzzle, filename); };
      auto cancel = [this]() {
        m_mgr.schedule_action(ViewManager::Action::close_message_box); };
      if (stdfs::exists(filename)) {
        m_mgr.message_box("Are you sure you want to overwrite the file \""
                          + stdfs::path(filename).filename().string() + "\"?",
                          MessageBoxView::Type::yes_no,
                          save, cancel, cancel);
      } else {
        save();
      }
    }
  }
}

int FileView::path_name_width() const
{
  int total = 0;

  if (m_cur_path != m_paths.end()) {
    int index = 0;
    for (const auto& e : *m_cur_path) {
      int width = 0;
      if (index == m_path_collapse_start
          && index < m_path_collapse_end)
        m_filename_font->text_size("...", &width, nullptr);
      else if (index < m_path_collapse_start
               || index >= m_path_collapse_end)
        m_filename_font->text_size(e.filename().string(), &width, nullptr);
      if (width > 0) {
        total += width + path_spacing;
        if (index != 0) {
          m_filename_font->text_size(">", &width, nullptr);
          total += width + path_spacing;
        }
      }
      ++index;
    }
  }

  return total;
}

int FileView::path_subdir_count() const
{
  int count = 0;
  if (m_cur_path != m_paths.end()) {
    for (auto it = m_cur_path->begin(); it != m_cur_path->end(); ++it) {
      ++count;
    }
  }
  return count;
}

void FileView::handle_directory_change()
{
  if (m_cur_path != m_paths.end()) {
    //update selection view
    FileSelectionPanel& file_panel
      = dynamic_cast<FileSelectionPanel&>(m_file_selection.main_panel());
    //if path hasn't already been changed, change it
    if (*m_cur_path != file_panel.path())
      file_panel.open_path(m_cur_path->string());
    //resize and reposition panels
    int panel_width = m_file_selection.boundary().width();
    int panel_height = m_file_selection.boundary().height();
    file_panel.resize(panel_width, file_panel.boundary().height());
    m_file_selection.resize(panel_width, panel_height); //refresh scrollbars
    file_panel.move(file_panel.boundary().x(),
                    m_file_selection.boundary().y());

    //save path for future FileView instances
    if (m_mode == Mode::open)
      s_last_open_path = m_cur_path->string();
    else
      s_last_save_path = m_cur_path->string();
  }

  collapse_path();
}

void FileView::handle_selection_change()
{
  FileSelectionPanel& panel
    = dynamic_cast<FileSelectionPanel&>(m_file_selection.main_panel());
  m_filename_box->set_text(panel.selected_file());
  clear_focus();
}

void FileView::collapse_path()
{
  if (m_cur_path != m_paths.end()) {
    //max allowed width is screen width minus the six nav buttons
    int max_width = 2 * panel_spacing
      + 6 * (m_up_button->boundary().width() + button_spacing);
    if (max_width < m_width)
      max_width = m_width - max_width;
    else
      max_width = 0;

    //move collapse points until width is within bounds
    int count = path_subdir_count();
    m_path_collapse_start = count / 2;
    m_path_collapse_end = m_path_collapse_start;

    if (max_width == 0) {
      m_path_collapse_start = 0;
      m_path_collapse_end = count;
      return;
    }

    while (path_name_width() > max_width
           && m_path_collapse_end - m_path_collapse_start < count) {
      if (m_path_collapse_end + 1 >= count && m_path_collapse_start > 0)
        --m_path_collapse_start;
      else if (m_path_collapse_start <= 1 && m_path_collapse_end < count)
        ++m_path_collapse_end;
      else {
        if (m_path_collapse_start % 2 == 0 && m_path_collapse_start > 0)
          --m_path_collapse_start;
        else
          ++m_path_collapse_end;
      }
    }
  }
}

void FileView::open_subdir(int index)
{
  if (m_cur_path != m_paths.end()) {
    int count = path_subdir_count();
    stdfs::path p = *m_cur_path;

    while (index + 1 < count) {
      p = p.parent_path();
      ++index;
    }

    open_path(p);
  }
}

void FileView::draw_tooltips(Renderer& renderer) const
{
  const int tt_spacing = 2;

  std::string tooltip;
  Rect bound;
  if (m_menu_button->is_mouse_over()) {
    tooltip = "Open menu";
    bound = m_menu_button->boundary();
  } else if (m_home_button->is_mouse_over()) {
    tooltip = "Go to default puzzles";
    bound = m_home_button->boundary();
  } else if (m_saved_button->is_mouse_over()) {
    tooltip = "Go to saved puzzles";
    bound = m_saved_button->boundary();
  } else if (m_up_button->is_mouse_over()) {
    tooltip = "Go up";
    bound = m_up_button->boundary();
  } else if (m_back_button->is_mouse_over()) {
    tooltip = "Go back";
    bound = m_back_button->boundary();
  } else if (m_forward_button->is_mouse_over()) {
    tooltip = "Go forward";
    bound = m_forward_button->boundary();
  }

  if (!tooltip.empty()) {
    int text_wd;
    m_filename_font->text_size(tooltip, &text_wd, nullptr);
    Point pt(bound.x(), bound.y() + bound.height() + tt_spacing);

    if (pt.x() + text_wd >= m_width)
      pt.x() = m_width - text_wd - tt_spacing * 2;

    draw_tooltip(renderer, pt, *m_filename_font, tooltip);
  }
}


