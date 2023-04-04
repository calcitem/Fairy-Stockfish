/*
  Sanmill, a UCI mill playing engine derived from Stockfish
  Copyright (C) 2019-2023 The Sanmill developers (see AUTHORS file)

  Sanmill is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Sanmill is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PSQT_H_INCLUDED
#define PSQT_H_INCLUDED


#include "types.h"

#include "variant.h"

namespace Sanmill::PSQT
{

extern Score psq[PIECE_NB][SQUARE_NB + 1];

// Fill psqt array from a set of internally linked parameters
extern void init(const Variant*);

} // namespace Sanmill::PSQT


#endif // PSQT_H_INCLUDED
