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

#include "puzzle/puzzle.hpp"

Puzzle::Puzzle(unsigned width, unsigned height)
  : m_grid(width, height)
{
  PuzzleClue zero;
  
  for (unsigned y = 0; y < height; ++y) {
    m_row_clues.push_back(ClueSequence(1, zero)); // add 0 clue for each row
  }

  for (unsigned x = 0; x < width; ++x) {
    m_col_clues.push_back(ClueSequence(1, zero)); // add 0 clue for each column
  }
}

void Puzzle::mark_cell(unsigned col, unsigned row, const Color& color)
{
  auto& cell = m_grid.at(col, row);
  cell.state = PuzzleCell::State::filled;
  cell.color = color;
}

bool Puzzle::is_solved() const
{
  for (unsigned i = 0; i < m_grid.width(); ++i) {
    if (!is_col_solved(i))
      return false;
  }
  for (unsigned j = 0; j < m_grid.height(); ++j) {
    if (!is_row_solved(j))
      return false;
  }
  return true;
}

bool Puzzle::is_row_solved(unsigned row) const
{
  //if only clue is 0, make sure row is empty
  if (row_clues(row).size() == 1 && row_clues(row)[0].value == 0) {
    for (unsigned i = 0; i < m_grid.width(); ++i) {
      if (m_grid.at(row, i).state == PuzzleCell::State::filled)
        return false;
    }
    return true;
  }

  unsigned pos = 0;
  for (auto clue : row_clues(row)) {
    while (pos < m_grid.width()
           && m_grid.at(row, pos).state != PuzzleCell::State::filled)
      ++pos;

    for (unsigned i = 0; i < clue.value; ++i) {
      if (pos + i > m_grid.width())
        return false;
      auto cell = m_grid.at(row, pos + i);
      if (cell.state != PuzzleCell::State::filled
          || clue.color != cell.color)
        return false;
    }
  }

  return true;
}

bool Puzzle::is_col_solved(unsigned col) const
{
  return true;
}

Puzzle::PuzzleCol Puzzle::operator[](unsigned col) const
{
  return PuzzleCol(*this, col);
}

const PuzzleCell& Puzzle::PuzzleCol::operator[](unsigned row) const
{
  return m_parent.m_grid.at(m_col, row);
}
