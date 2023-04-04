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

// header used in NNUE evaluation function

#ifndef NNUE_EVALUATE_NNUE_H_INCLUDED
#define NNUE_EVALUATE_NNUE_H_INCLUDED

#include "nnue_feature_transformer.h"

#include <memory>

namespace Sanmill::Eval::NNUE {

  // Hash value of evaluation function structure
  constexpr std::uint32_t HashValue =
      FeatureTransformer::get_hash_value() ^ Network::get_hash_value();

  // Deleter for automating release of memory area
  template <typename T>
  struct AlignedDeleter {
    void operator()(T* ptr) const {
      ptr->~T();
      std_aligned_free(ptr);
    }
  };

  template <typename T>
  struct LargePageDeleter {
    void operator()(T* ptr) const {
      ptr->~T();
      aligned_large_pages_free(ptr);
    }
  };

  template <typename T>
  using AlignedPtr = std::unique_ptr<T, AlignedDeleter<T>>;

  template <typename T>
  using LargePagePtr = std::unique_ptr<T, LargePageDeleter<T>>;

}  // namespace Sanmill::Eval::NNUE

#endif // #ifndef NNUE_EVALUATE_NNUE_H_INCLUDED
