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

// Input features and network structure used in NNUE evaluation function

#ifndef NNUE_ARCHITECTURE_H_INCLUDED
#define NNUE_ARCHITECTURE_H_INCLUDED

#include "nnue_common.h"

#include "features/half_ka_v2_variants.h"

#include "layers/input_slice.h"
#include "layers/affine_transform.h"
#include "layers/clipped_relu.h"

namespace Sanmill::Eval::NNUE {

  // Input features used in evaluation function
  using FeatureSet = Features::HalfKAv2Variants;

  // Number of input feature dimensions after conversion
  constexpr IndexType TransformedFeatureDimensions = 512;
  constexpr IndexType PSQTBuckets = 8;
  constexpr IndexType LayerStacks = 8;

  namespace Layers {

    // Define network structure
    using InputLayer = InputSlice<TransformedFeatureDimensions * 2>;
    using HiddenLayer1 = ClippedReLU<AffineTransform<InputLayer, 16>>;
    using HiddenLayer2 = ClippedReLU<AffineTransform<HiddenLayer1, 32>>;
    using OutputLayer = AffineTransform<HiddenLayer2, 1>;

  }  // namespace Layers

  using Network = Layers::OutputLayer;

  static_assert(TransformedFeatureDimensions % MaxSimdWidth == 0, "");
  static_assert(Network::OutputDimensions == 1, "");
  static_assert(std::is_same<Network::OutputType, std::int32_t>::value, "");

}  // namespace Sanmill::Eval::NNUE

#endif // #ifndef NNUE_ARCHITECTURE_H_INCLUDED
