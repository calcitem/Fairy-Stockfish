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

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

/// When compiling with provided Makefile (e.g. for Linux and OSX), configuration
/// is done automatically. To get started type 'make help'.
///
/// When Makefile is not used (e.g. with Microsoft Visual Studio) some switches
/// need to be set manually:
///
/// -DNDEBUG      | Disable debugging mode. Always use this for release.
///
/// -DNO_PREFETCH | Disable use of prefetch asm-instruction. You may need this to
///               | run on some very old machines.
///
/// -DUSE_POPCNT  | Add runtime support for use of popcnt asm-instruction. Works
///               | only in 64-bit mode and requires hardware with popcnt support.
///
/// -DUSE_PEXT    | Add runtime support for use of pext asm-instruction. Works
///               | only in 64-bit mode and requires hardware with pext support.

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <algorithm>

#if defined(_MSC_VER)
// Disable some silly and noisy warning from MSVC compiler
#pragma warning(disable: 4127) // Conditional expression is constant
#pragma warning(disable: 4146) // Unary minus operator applied to unsigned type
#pragma warning(disable: 4800) // Forcing value to bool 'true' or 'false'
#pragma comment(linker, "/STACK:8000000") // Use 8 MB stack size for MSVC
#pragma comment(lib, "advapi32.lib") // Fix linker error
#endif

/// Predefined macros hell:
///
/// __GNUC__           Compiler is gcc, Clang or Intel on Linux
/// __INTEL_COMPILER   Compiler is Intel
/// _MSC_VER           Compiler is MSVC or Intel on Windows
/// _WIN32             Building on Windows (any)
/// _WIN64             Building on Windows 64 bit

#if defined(__GNUC__ ) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ <= 2)) && defined(_WIN32) && !defined(__clang__)
#define ALIGNAS_ON_STACK_VARIABLES_BROKEN
#endif

#define ASSERT_ALIGNED(ptr, alignment) assert(reinterpret_cast<uintptr_t>(ptr) % alignment == 0)

#if defined(_WIN64) && defined(_MSC_VER) // No Makefile used
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#  define IS_64BIT
#endif

#if defined(USE_POPCNT) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <nmmintrin.h> // Intel and Microsoft header for _mm_popcnt_u64()
#endif

#if !defined(NO_PREFETCH) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <xmmintrin.h> // Intel and Microsoft header for _mm_prefetch()
#endif

#if defined(USE_PEXT)
#  include <immintrin.h> // Header for _pext_u64() intrinsic
#  ifdef LARGEBOARDS
#    define pext(b, m) (_pext_u64(b, m) ^ (_pext_u64(b >> 64, m >> 64) << popcount((m << 64) >> 64)))
#  else
#    define pext(b, m) _pext_u64(b, m)
#  endif
#else
#  define pext(b, m) 0
#endif

namespace Sanmill {

#ifdef USE_POPCNT
constexpr bool HasPopCnt = true;
#else
constexpr bool HasPopCnt = false;
#endif

#ifdef USE_PEXT
constexpr bool HasPext = true;
#else
constexpr bool HasPext = false;
#endif

#ifdef IS_64BIT
constexpr bool Is64Bit = true;
#else
constexpr bool Is64Bit = false;
#endif

typedef uint64_t Key;
typedef uint32_t Bitboard;

constexpr int MAX_MOVES = 72;
constexpr int MAX_PLY   = 48;

/// A move needs 32 bits to be stored
///
/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
/// any normal move destination square is always different from origin square
/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.

enum Move : int {
  MOVE_NONE,
  MOVE_NULL = 65
};

enum MoveType : int {
  MOVETYPE_PLACE,
  MOVETYPE_MOVE,
  MOVETYPE_REMOVE,
};

constexpr int MOVE_TYPE_BITS = 4;

enum Color {
  WHITE, BLACK, COLOR_NB = 2
};

enum CheckCount : int {
  CHECKS_0 = 0, CHECKS_NB = 11
};

enum MaterialCounting {
  NO_MATERIAL_COUNTING, JANGGI_MATERIAL, UNWEIGHTED_MATERIAL, WHITE_DRAW_ODDS, BLACK_DRAW_ODDS
};

enum CountingRule {
  NO_COUNTING, MAKRUK_COUNTING, CAMBODIAN_COUNTING, ASEAN_COUNTING
};

enum ChasingRule {
  NO_CHASING, AXF_CHASING
};

enum EnclosingRule {
  NO_ENCLOSING, REVERSI, ATAXX
};

enum OptBool {
  NO_VALUE, VALUE_FALSE, VALUE_TRUE
};

enum class Phase { none, ready, placing, moving, gameOver };

// enum class that represents an action that one player can take when it's
// his turn at the board. The can be on of the following:
//   - Select a piece on the board;
//   - Place a piece on the board;
//   - Move a piece on the board:
//       - Slide a piece between two adjacent locations;
//       - 'Jump' a piece to any empty location if the player has less than
//         three or four pieces and mayFly is |true|;
//   - Remove an opponent's piece after successfully closing a mill.
enum class Action { none, select, place, remove };

enum class GameOverReason {
    none,

    // A player wins by reducing the opponent to two pieces
    // (where they could no longer form mills and thus be unable to win)
    loseLessThanThree,

    // A player wins by leaving them without a legal move.
    loseNoWay,
    loseBoardIsFull,

    loseResign,
    loseTimeOver,
    drawThreefoldRepetition,
    drawRule50,
    drawEndgameRule50,
    drawBoardIsFull,
    drawNoWay,
};

enum ScaleFactor {
  SCALE_FACTOR_DRAW    = 0,
  SCALE_FACTOR_NORMAL  = 64,
  SCALE_FACTOR_MAX     = 128,
  SCALE_FACTOR_NONE    = 255
};

enum Bound {
  BOUND_NONE,
  BOUND_UPPER,
  BOUND_LOWER,
  BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
};

enum Value : int {
    VALUE_ZERO = 0,
    VALUE_DRAW = 0,
#ifdef ENDGAME_LEARNING
    VALUE_KNOWN_WIN = 25,
#endif
    VALUE_MATE = 80,
    VALUE_UNIQUE = 100,
    VALUE_INFINITE = 125,
    VALUE_UNKNOWN = INT8_MIN,
    VALUE_NONE = VALUE_UNKNOWN,

    VALUE_TB_WIN_IN_MAX_PLY = VALUE_MATE - 2 * MAX_PLY,
    VALUE_TB_LOSS_IN_MAX_PLY = -VALUE_TB_WIN_IN_MAX_PLY,
    VALUE_MATE_IN_MAX_PLY = VALUE_MATE - MAX_PLY,
    VALUE_MATED_IN_MAX_PLY = -VALUE_MATE_IN_MAX_PLY,

    PieceValue = 5,
    VALUE_EACH_PIECE = PieceValue,
    VALUE_EACH_PIECE_INHAND = VALUE_EACH_PIECE,
    VALUE_EACH_PIECE_ONBOARD = VALUE_EACH_PIECE,
    VALUE_EACH_PIECE_PLACING_NEEDREMOVE = VALUE_EACH_PIECE,
    VALUE_EACH_PIECE_MOVING_NEEDREMOVE = VALUE_EACH_PIECE,

    VALUE_MTDF_WINDOW = VALUE_EACH_PIECE,
    VALUE_PVS_WINDOW = VALUE_EACH_PIECE,

    VALUE_PLACING_WINDOW = VALUE_EACH_PIECE_PLACING_NEEDREMOVE +
                           (VALUE_EACH_PIECE_ONBOARD -
                            VALUE_EACH_PIECE_INHAND) +
                           1,
    VALUE_MOVING_WINDOW = VALUE_EACH_PIECE_MOVING_NEEDREMOVE + 1,
};

enum PieceType {
    NO_PIECE_TYPE = 0,
    WHITE_PIECE = 1,
    BLACK_PIECE = 2,
    BAN = 3,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 4,

    IN_HAND = 0x10,
    ON_BOARD = 0x20,
};

enum Piece : uint8_t {
    NO_PIECE = 0x00,
    BAN_PIECE = 0x0F,

    W_PIECE = 0x10,

    B_PIECE = 0x20,

    // Fix overflow
    PIECE_NB = 64,
};

enum PieceSet : uint64_t {
  NO_PIECE_SET = 0,
  CHESS_PIECES = (1ULL << PAWN) | (1ULL << KNIGHT) | (1ULL << BISHOP) | (1ULL << ROOK) | (1ULL << QUEEN) | (1ULL << KING),
  COMMON_FAIRY_PIECES = (1ULL << IMMOBILE_PIECE) | (1ULL << COMMONER) | (1ULL << ARCHBISHOP) | (1ULL << CHANCELLOR),
  SHOGI_PIECES = (1ULL << SHOGI_PAWN) | (1ULL << GOLD) | (1ULL << SILVER) | (1ULL << SHOGI_KNIGHT) | (1ULL << LANCE)
                | (1ULL << DRAGON)| (1ULL << DRAGON_HORSE) | (1ULL << KING),
  COMMON_STEP_PIECES = (1ULL << COMMONER) | (1ULL << FERS) | (1ULL << WAZIR) | (1ULL << BREAKTHROUGH_PIECE),
};

extern Value PieceValue[PHASE_NB][PIECE_NB];
extern Value EvalPieceValue[PHASE_NB][PIECE_NB]; // variant piece values for evaluation
extern Value CapturePieceValue[PHASE_NB][PIECE_NB]; // variant piece values for captures/search

typedef int Depth;

enum : int {
  DEPTH_QS_CHECKS     =  0,
  DEPTH_QS_NO_CHECKS  = -1,
  DEPTH_QS_RECAPTURES = -5,

  DEPTH_NONE   = -6,

  DEPTH_OFFSET = -7 // value used only for TT entry occupancy check
};

enum Square : int {
    SQ_0 = 0,
    SQ_1 = 1,
    SQ_2 = 2,
    SQ_3 = 3,
    SQ_4 = 4,
    SQ_5 = 5,
    SQ_6 = 6,
    SQ_7 = 7,
    SQ_8 = 8,
    SQ_9 = 9,
    SQ_10 = 10,
    SQ_11 = 11,
    SQ_12 = 12,
    SQ_13 = 13,
    SQ_14 = 14,
    SQ_15 = 15,
    SQ_16 = 16,
    SQ_17 = 17,
    SQ_18 = 18,
    SQ_19 = 19,
    SQ_20 = 20,
    SQ_21 = 21,
    SQ_22 = 22,
    SQ_23 = 23,
    SQ_24 = 24,
    SQ_25 = 25,
    SQ_26 = 26,
    SQ_27 = 27,
    SQ_28 = 28,
    SQ_29 = 29,
    SQ_30 = 30,
    SQ_31 = 31,

    SQ_A1 = 8,
    SQ_A2 = 9,
    SQ_A3 = 10,
    SQ_A4 = 11,
    SQ_A5 = 12,
    SQ_A6 = 13,
    SQ_A7 = 14,
    SQ_A8 = 15,
    SQ_B1 = 16,
    SQ_B2 = 17,
    SQ_B3 = 18,
    SQ_B4 = 19,
    SQ_B5 = 20,
    SQ_B6 = 21,
    SQ_B7 = 22,
    SQ_B8 = 23,
    SQ_C1 = 24,
    SQ_C2 = 25,
    SQ_C3 = 26,
    SQ_C4 = 27,
    SQ_C5 = 28,
    SQ_C6 = 29,
    SQ_C7 = 30,
    SQ_C8 = 31,

    SQ_32 = 32,
    SQ_33 = 33,
    SQ_34 = 34,
    SQ_35 = 35,
    SQ_36 = 36,
    SQ_37 = 37,
    SQ_38 = 38,
    SQ_39 = 39,

    SQ_NONE = 0,

    // The board consists of a grid with twenty-four intersections or points.
    SQUARE_NB = 24,

    SQUARE_ZERO = 0,
    SQUARE_EXT_NB = 40,

    SQ_BEGIN = SQ_8,
    SQ_END = SQ_32
};

enum MoveDirection : int {
    MD_CLOCKWISE = 0,
    MD_BEGIN = MD_CLOCKWISE,
    MD_ANTICLOCKWISE = 1,
    MD_INWARD = 2,
    MD_OUTWARD = 3,
    MD_NB = 4
};

enum LineDirection : int {
    LD_HORIZONTAL = 0,
    LD_VERTICAL = 1,
    LD_SLASH = 2,
    LD_NB = 3
};

enum File : int {
  FILE_A, FILE_B, FILE_C,
  FILE_MAX = FILE_NB - 1
};

enum Rank : int {
  RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,
  RANK_NB,
  RANK_MAX = RANK_NB - 1
};

// Keep track of what a move changes on the board (used by NNUE)
struct DirtyPiece {

  // Number of changed pieces
  int dirty_num;

  // Max 3 pieces can change in one move. A promotion with capture moves
  // both the pawn and the captured piece to SQ_NONE and the piece promoted
  // to from SQ_NONE to the capture square.
  Piece piece[12];
  Piece handPiece[12];
  int handCount[12];

  // From and to squares, which may be SQ_NONE
  Square from[12];
  Square to[12];
};

/// Score enum stores a middlegame and an endgame value in a single integer (enum).
/// The least significant 16 bits are used to store the middlegame value and the
/// upper 16 bits are used to store the endgame value. We have to take care to
/// avoid left-shifting a signed int to avoid undefined behavior.
enum Score : int { SCORE_ZERO };

constexpr Score make_score(int mg, int eg) {
  return Score((int)((unsigned int)eg << 16) + mg);
}

/// Extracting the signed lower and upper 16 bits is not so trivial because
/// according to the standard a simple cast to short is implementation defined
/// and so is a right shift of a signed integer.
inline Value eg_value(Score s) {
  union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
  return Value(eg.s);
}

inline Value mg_value(Score s) {
  union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
  return Value(mg.s);
}

#define ENABLE_BIT_OPERATORS_ON(T)                                        \
constexpr T operator~ (T d) { return (T)~(int)d; }                        \
constexpr T operator| (T d1, T d2) { return (T)((int)d1 | (int)d2); }     \
constexpr T operator& (T d1, T d2) { return (T)((int)d1 & (int)d2); }     \
constexpr T operator^ (T d1, T d2) { return (T)((int)d1 ^ (int)d2); }     \
inline T& operator|= (T& d1, T d2) { return (T&)((int&)d1 |= (int)d2); }  \
inline T& operator&= (T& d1, T d2) { return (T&)((int&)d1 &= (int)d2); }  \
inline T& operator^= (T& d1, T d2) { return (T&)((int&)d1 ^= (int)d2); }

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); }    \
constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); }    \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }       \
inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Value)
ENABLE_INCR_OPERATORS_ON(MoveDirection)

ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
ENABLE_INCR_OPERATORS_ON(CheckCount)

ENABLE_BASE_OPERATORS_ON(Score)

ENABLE_BASE_OPERATORS_ON(PieceType)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON
#undef ENABLE_BIT_OPERATORS_ON

constexpr PieceSet piece_set(PieceType pt) {
  return PieceSet(1ULL << pt);
}

constexpr PieceSet operator~ (PieceSet ps) { return (PieceSet)~(uint64_t)ps; }
constexpr PieceSet operator| (PieceSet ps1, PieceSet ps2) { return (PieceSet)((uint64_t)ps1 | (uint64_t)ps2); }
constexpr PieceSet operator| (PieceSet ps, PieceType pt) { return ps | piece_set(pt); }
constexpr PieceSet operator& (PieceSet ps1, PieceSet ps2) { return (PieceSet)((uint64_t)ps1 & (uint64_t)ps2); }
constexpr PieceSet operator& (PieceSet ps, PieceType pt) { return ps & piece_set(pt); }
inline PieceSet& operator|= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((uint64_t&)ps1 |= (uint64_t)ps2); }
inline PieceSet& operator|= (PieceSet& ps, PieceType pt) { return ps |= piece_set(pt); }
inline PieceSet& operator&= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((uint64_t&)ps1 &= (uint64_t)ps2); }

static_assert(piece_set(PAWN) & PAWN);

/// Only declared but not defined. We don't want to multiply two scores due to
/// a very high risk of overflow. So user should explicitly convert to integer.
Score operator*(Score, Score) = delete;

/// Division of a Score must be handled separately for each term
inline Score operator/(Score s, int i) {
  return make_score(mg_value(s) / i, eg_value(s) / i);
}

/// Multiplication of a Score by an integer. We check for overflow in debug mode.
inline Score operator*(Score s, int i) {

  Score result = Score(int(s) * i);

  assert(eg_value(result) == (i * eg_value(s)));
  assert(mg_value(result) == (i * mg_value(s)));
  assert((i == 0) || (result / i) == s);

  return result;
}

/// Multiplication of a Score by a boolean
inline Score operator*(Score s, bool b) {
  return b ? s : SCORE_ZERO;
}

constexpr Color operator~(Color c) {
  return static_cast<Color>(c ^ 3); // Toggle color
}

constexpr Piece operator~(Piece pc) {
  // Swap color of piece
  if (pc == W_PIECE) {
    return B_PIECE;
  }

  return W_PIECE;
}

constexpr Value mate_in(int ply) {
  return VALUE_MATE - ply;
}

constexpr Value mated_in(int ply) {
  return -VALUE_MATE + ply;
}

constexpr Value convert_mate_value(Value v, int ply) {
  return  v ==  VALUE_MATE ? mate_in(ply)
        : v == -VALUE_MATE ? mated_in(ply)
        : v;
}

constexpr Square make_square(File f, Rank r) {
  return static_cast<Square>((f << 3) + r - 1);
}

constexpr Piece make_piece(Color c)
{
    return static_cast<Piece>(c << 4);
}

constexpr Piece make_piece(Color c, PieceType pt) {
    if (pt == WHITE_PIECE || pt == BLACK_PIECE) {
        return make_piece(c);
    }

    if (pt == BAN) {
        return BAN_PIECE;
    }

    return NO_PIECE;
}

constexpr PieceType type_of(Piece pc) {
    if (pc == BAN_PIECE) {
        return BAN;
    }

    if (color_of(pc) == WHITE) {
        return WHITE_PIECE;
    }

    if (color_of(pc) == BLACK) {
        return BLACK_PIECE;
    }

    return NO_PIECE_TYPE;
}

inline Color color_of(Piece pc) {
  assert(pc != NO_PIECE);
  return static_cast<Color>(pc >> 4);
}

constexpr bool is_ok(Square s) {
  return s == SQ_NONE || (s >= SQ_BEGIN && s < SQ_END);
}

constexpr File file_of(Square s) {
  return static_cast<File>(s >> 3);
}

constexpr Rank rank_of(Square s) {
  return static_cast<Rank>((s & 0x07) + 1);
}

constexpr MoveType type_of(Move m) {
    if (m < 0) {
        return MOVETYPE_REMOVE;
    }
    if (m & 0x1f00) {
        return MOVETYPE_MOVE;
    }

    return MOVETYPE_PLACE; // m & 0x00ff
}

constexpr Square to_sq(Move m) {
    if (m < 0)
        m = static_cast<Move>(-m);

    return static_cast<Square>(m & 0x00FF);
}

constexpr Square from_sq(Move m) {
    if (m < 0)
        m = static_cast<Move>(-m);

    return static_cast<Square>(m >> 8);
}

constexpr Move make_move(Square from, Square to) {
  return static_cast<Move>((from << 8) + to);
}

constexpr Move reverse_move(Move m) {
  return make_move(to_sq(m), from_sq(m));
}

inline bool is_ok(Move m) {
  return from_sq(m) != to_sq(m); // Catch MOVE_NULL and MOVE_NONE
}

/// Based on a congruential pseudo random number generator
constexpr Key make_key(uint64_t seed) {
  return seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

} // namespace Sanmill

#endif // #ifndef TYPES_H_INCLUDED

#include "tune.h" // Global visibility to tuning setup
