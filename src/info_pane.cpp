#include "game.h"
#include "preview.h"
#include "static_text.h"

#include "info_pane.h"

const int default_spacing = 20;
const int static_text_height_std = 18;
const int static_text_height_heading = 32;

InfoPane::InfoPane(Game* game) : m_game{game}, m_width{0} {
  m_preview = new Preview(game);
  m_title = new StaticText(game);
  m_size = new StaticText(game);
}

InfoPane::~InfoPane() {
  delete m_size;
  delete m_title;
  delete m_preview;
}

void InfoPane::update() {
  m_preview->update();
  m_title->update();
  m_size->update();
}

void InfoPane::draw(Renderer* renderer) const {
  m_preview->draw(renderer);
  m_title->draw(renderer);
  m_size->draw(renderer);
}

void InfoPane::setup_controls() {
  int y = default_spacing;
  
  m_title->move(default_spacing, y);
  m_title->resize(m_width - 2 * default_spacing, static_text_height_heading);
  m_title->set_string(m_game->puzzle().title());
  m_title->set_type(StaticText::Type::heading);
  
  y += static_text_height_heading;
  y += default_spacing;

  int puzzle_width = m_game->puzzle().width();
  int puzzle_height = m_game->puzzle().height();
  std::string size_str = std::to_string(puzzle_width);
  size_str += " × ";
  size_str += std::to_string(puzzle_height);
  
  m_size->move(default_spacing, y);
  m_size->resize(m_width - 2 * default_spacing, static_text_height_std);
  m_size->set_string(size_str);
  m_size->set_type(StaticText::Type::standard);

  y += static_text_height_std;
  y += default_spacing;
  
  m_preview->resize(m_width - 2 * default_spacing,
                    m_width - 2 * default_spacing);
  m_preview->update_pixel_size();

  int pixel_size = m_preview->pixel_size();
  int width = pixel_size * puzzle_width;
  int height = pixel_size * puzzle_height;
  
  m_preview->resize(width, height);
  m_preview->move(m_width / 2 - width / 2, y);

  y += height;
  y += default_spacing;
}
