/*
  Fairy-Sanmill, a UCI mill variant playing engine derived from Sanmill
  Copyright (C) 2019-2023 The Sanmill developers

  Fairy-Sanmill is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Fairy-Sanmill is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PIECE_H_INCLUDED
#define PIECE_H_INCLUDED

#include <string>
#include <map>

#include "types.h"
#include "variant.h"

namespace Sanmill {

enum MoveModality {MODALITY_QUIET, MODALITY_CAPTURE, MOVE_MODALITY_NB};

/// PieceInfo struct stores information about the piece movements.

struct PieceInfo {
  std::string name = "";
  std::string betza = "";
  std::map<Direction, int> steps[2][MOVE_MODALITY_NB] = {};
  std::map<Direction, int> slider[2][MOVE_MODALITY_NB] = {};
  std::map<Direction, int> hopper[2][MOVE_MODALITY_NB] = {};
};

struct PieceMap : public std::map<PieceType, const PieceInfo*> {
  void init(const Variant* v = nullptr);
  void add(PieceType pt, const PieceInfo* v);
  void clear_all();
};

extern PieceMap pieceMap;

inline std::string piece_name(PieceType pt) {
  return is_custom(pt) ? "customPiece" + std::to_string(pt - CUSTOM_PIECES + 1)
                       : pieceMap.find(pt)->second->name;
}

} // namespace Sanmill

#endif // #ifndef PIECE_H_INCLUDED
