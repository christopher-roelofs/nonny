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

#ifndef NONNY_BLOCK_SEQUENCE_HPP
#define NONNY_BLOCK_SEQUENCE_HPP

#include <cstddef>
#include <vector>
#include "puzzle/puzzle_clue.hpp"

struct Block {
  unsigned pos = 0;
  unsigned length = 0;
  Color color;
};

class BlockSequence {
public:
  enum class Init { left, right };
  BlockSequence(unsigned line_size,
                const std::vector<PuzzleClue>& line_clues,
                Init init_type = Init::left);

  bool slide_left();
  bool slide_right();
  
  std::size_t size() const { return m_blocks.size(); }
  bool empty() const { return m_blocks.empty(); }

  typedef std::vector<Block>::iterator iterator;
  typedef std::vector<Block>::const_iterator const_iterator;
  typedef std::vector<Block>::reverse_iterator reverse_iterator;
  typedef std::vector<Block>::const_reverse_iterator const_reverse_iterator;
  iterator begin() { return m_blocks.begin(); }
  iterator end() { return m_blocks.end(); }
  const_iterator begin() const { return m_blocks.begin(); }
  const_iterator end() const { return m_blocks.end(); }
  const_iterator cbegin() const { return m_blocks.cbegin(); }
  const_iterator cend() const { return m_blocks.cend(); }
  reverse_iterator rbegin() { return m_blocks.rbegin(); }
  reverse_iterator rend() { return m_blocks.rend(); }
  const_reverse_iterator rbegin() const { return m_blocks.rbegin(); }
  const_reverse_iterator rend() const { return m_blocks.rend(); }

  Block& operator[](unsigned index) { return m_blocks[index]; }
  const Block& operator[](unsigned index) const { return m_blocks[index]; }
  
private:
  void flush_left();
  void flush_right();

  std::vector<Block> m_blocks;
  unsigned m_line_size;
};

#endif