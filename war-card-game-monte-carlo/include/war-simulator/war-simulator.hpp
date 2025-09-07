/*
 * Rules:
 *  + Each player can shuffle with either has run out of cards
 *  + Choice is whether or not to shuffle based on if the hand__ is enriched.
 * */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <numeric> // for std::accumulate
#include <random>
#include <string>
#include <utility>
#include <vector>

const std::size_t kMaxCard = 14;
const std::size_t kDeckSize = 52;
const std::size_t kMaxRounds = 100000;
const std::size_t kGameCount = 1000000;

extern std::mt19937 gen;

enum class PlayerEnum { kOne = 0, kTwo = 1, kNone = 2 };

class Player;
typedef void (*Strategy_fp_t)(Player &, const std::size_t);

struct Strategy {
  std::size_t id;
  Strategy_fp_t fp;
  void operator()(Player &player, const std::size_t n) { fp(player, n); }
};

class Player {
public:
  std::deque<uint32_t> hand_;
  Strategy strategy_;
  std::vector<uint32_t> pile_;

  std::size_t hand_size() const { return hand_.size(); }
  std::size_t ncards() const { return hand_size() + pile_.size(); }

  uint32_t draw() {
    assert(hand_size());
    const auto v = hand_.front();
    hand_.pop_front();
    return v;
  }

  void take(uint32_t card) { pile_.push_back(card); }

  void combine_pile() {
    hand_.insert(hand_.end(), pile_.begin(), pile_.end());
    pile_.clear();
  }

  bool is_valid() {
    bool valid = true;
    for (const auto pt : hand_) {
      valid &= (pt > 1 && pt <= kMaxCard);
    }
    for (const auto pt : pile_) {
      valid &= (pt > 1 && pt <= kMaxCard);
    }
    return valid;
  }

  Player(std::deque<uint32_t> hand, Strategy strategy)
      : hand_{hand}, strategy_{strategy} {
    assert(is_valid());
  }
  Player(Player &&other) noexcept
      : hand_(std::move(other.hand_)), strategy_(other.strategy_),
        pile_(std::move(other.pile_))
  // function pointer just copied
  {
    // After the move, other.hand_ and other.pile_ are left in a valid but empty
    // state.
    assert(is_valid());
    pile_.reserve(kDeckSize);
  }

  // Custom move assignment
  Player &operator=(Player &&other) noexcept {
    if (this != &other) {
      hand_ = std::move(other.hand_);
      pile_ = std::move(other.pile_);
      strategy_ = other.strategy_;
      pile_.reserve(kDeckSize);
      assert(is_valid());
    }
    return *this;
  }
};

class WarHand {
public:
  std::vector<uint32_t> dump{};
  uint32_t flip = 0;

  bool is_valid() const { return flip > 0; }

  WarHand(Player &player) {
    //  If no cards in hand when a war starts then player loses.
    if (player.hand_size() >= 1) {
      const std::size_t dump_size = std::min(4ul, player.hand_size());
      for (std::size_t i = 0; i < dump_size; i++) {
        const auto card = player.draw();
        assert(card > 1);
        dump.push_back(card);
      }
      flip = dump.back();
      dump.pop_back();
    }
  }
};

/*
 * 52 % 13 + 2 moves the ace to 14
 * */
inline std::array<uint32_t, kDeckSize> make_deck() {
  std::array<uint32_t, kDeckSize> deck{};
  for (std::size_t i = 0; i < deck.size(); i++) {
    const uint32_t card = 2 + i % (kMaxCard - 1);
    assert(card > 0 && card <= kMaxCard);
    deck.at(i) = card;
  }
  return deck;
}

template <typename T> inline void shuffle_hand(T &v) {
  // Use a random device to seed the PRNG
  std::shuffle(v.begin(), v.end(), gen);
}

template <typename T> inline double average(const T &arr) {
  return arr.empty()
             ? 0.0
             : static_cast<double>(std::accumulate(arr.begin(), arr.end(), 0)) /
                   arr.size();
}

template <typename T, typename U>
inline std::size_t count_card_type(const T &arr, const std::vector<U> &types) {
  std::size_t cnt = 0;
  for (const auto &pt : arr) {
    for (const auto &item : types) {
      cnt += static_cast<std::size_t>(pt == item);
    }
  }
  return cnt;
}

struct AverageEnrichment {
  static bool enriched(const Player &player) {
    return average(player.hand_) > average(player.pile_);
  }
};

struct AcesEnrichment {
  static bool enriched(const Player &player) {
    const std::vector<uint32_t> aces{kMaxCard};
    auto nhand = count_card_type(player.hand_, aces);
    auto npile = count_card_type(player.pile_, aces);
    return static_cast<double>(nhand) / player.hand_size() >
           static_cast<double>(npile) / player.pile_.size();
  }
};

struct FaceCardEnrichment {
  static bool enriched(const Player &player) {
    const std::vector<uint32_t> face_cards{kMaxCard, kMaxCard - 1, kMaxCard - 2,
                                           kMaxCard - 3};
    auto nhand = count_card_type(player.hand_, face_cards);
    auto npile = count_card_type(player.pile_, face_cards);
    return static_cast<double>(nhand) / player.hand_size() >
           static_cast<double>(npile) / player.pile_.size();
  }
};

template <typename EnrichmentPolicy, bool ShuffleWhenEnriched>
inline void combine_strategy(Player &player, std::size_t ncards) {
  const bool enriched_hand = EnrichmentPolicy::enriched(player);

  bool shuffle = ShuffleWhenEnriched ? enriched_hand : !enriched_hand;
  bool combine =
      ((player.hand_size() < ncards) && !player.pile_.empty()) || shuffle;

  if (combine) {
    player.combine_pile();
    if (shuffle) {
      shuffle_hand(player.hand_);
    }
  }
}

inline void combine_only_strategy(Player &player,

                                  const std::size_t ncards) {
  bool combine = (player.hand_size() < ncards && player.pile_.size());
  if (combine) {
    player.combine_pile();
  }
}

inline void always_shuffle_strategy(Player &player, const std::size_t ncards) {
  player.combine_pile();
  shuffle_hand(player.hand_);
}

inline void combine_and_shuffle_strategy(Player &player,
                                         const std::size_t ncards) {
  combine_only_strategy(player, ncards);
  shuffle_hand(player.hand_);
}

inline void decide_shuffle(Player &p1, Player &p2, const std::size_t ncards) {
  if (p1.hand_size() < ncards || p2.hand_size() < ncards) {
    //  Shuffle event
    p1.strategy_(p1, ncards);
    p2.strategy_(p2, ncards);
  }
  assert(p1.hand_size() >= ncards || p1.pile_.size() == 0);
  assert(p2.hand_size() >= ncards || p2.pile_.size() == 0);
}

struct Results {
  std::size_t s1;
  std::size_t s2;
  uint32_t p1 = 0;
  uint32_t p2 = 0;
  uint32_t tie = 0;
  double average_p1_war_lost = 0;
  double average_p2_war_lost = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Results &r) {
  os << r.s1 << ", " << r.s2 << ", " << r.p1 << ", " << r.p2 << ", " << r.tie
     << ", " << r.average_p1_war_lost << ", " << r.average_p2_war_lost;
  return os;
}

struct GameResult {
  PlayerEnum winner = PlayerEnum::kNone;
  std::vector<uint32_t> war_hands_p1_lost{};
  std::vector<uint32_t> war_hands_p2_lost{};
  std::size_t nhands = 0;
};

inline PlayerEnum play_hand(const uint32_t c1, const uint32_t c2, Player &p1,
                            Player &p2, GameResult *result) {

  assert(c1 > 0);
  assert(c2 > 0);

  result->nhands += 1;

  PlayerEnum winner = PlayerEnum::kNone;
  if (c1 > c2) {
    winner = PlayerEnum::kOne;
  } else if (c1 < c2) {
    winner = PlayerEnum::kTwo;
  } else {
    // War
    assert(c1);
    assert(c2);

    // If a players last card is a war then they lose
    if (p1.ncards() == 0) {
      p2.take(c1);
      p2.take(c2);
      return PlayerEnum::kTwo;
    }
    if (p2.ncards() == 0) {
      p1.take(c1);
      p1.take(c2);
      return PlayerEnum::kOne;
    }

    const std::size_t kWarSize = 4;
    decide_shuffle(p1, p2, kWarSize);

    const auto wh1 = WarHand{p1};
    const auto wh2 = WarHand{p2};
    assert(wh1.is_valid());
    assert(wh2.is_valid());

    winner = play_hand(wh1.flip, wh2.flip, p1, p2, result);

    std::vector<uint32_t> cards{}; //  flip cards are assiged in play_hand

    for (auto &pt : wh1.dump) {
      assert(pt > 1);
      cards.push_back(pt);
    }
    for (auto &pt : wh2.dump) {
      assert(pt > 1);
      cards.push_back(pt);
    }

    shuffle_hand(cards);

    switch (winner) {
    case PlayerEnum::kOne: {
      for (auto card : cards) {
        p1.take(card);
      }
      for (auto card : wh2.dump) {
        result->war_hands_p2_lost.push_back(card);
      }
      break;
    }
    case PlayerEnum::kTwo: {
      for (auto card : cards) {
        p2.take(card);
      }
      for (auto card : wh1.dump) {
        result->war_hands_p1_lost.push_back(card);
      }
      break;
    }
    default:
      assert(0);
      break;
    }
  }

  //  Cheap shuffle of the two cards
  uint32_t card1 = c1;
  uint32_t card2 = c2;
  if (rand() % 2)
    std::swap(card1, card2);

  switch (winner) {
  case PlayerEnum::kOne:
    p1.take(card1);
    p1.take(card2);
    break;
  case PlayerEnum::kTwo:
    p2.take(card1);
    p2.take(card2);
    break;
  default:
    assert(0);
    break;
  }
  return winner;
}

inline GameResult simulate(Player &p1, Player &p2) {
  assert(p1.ncards() == p2.ncards());
  // std::cout << p1.size() << "\t" << p2.size() << std::endl;

  GameResult result{};

  for (std::size_t i = 0; i < kMaxRounds; i++) {
    const auto size1 = p1.ncards();
    const auto size2 = p2.ncards();

    if (size1 <= 0) {
      result.winner = PlayerEnum::kTwo;
      break;
    }
    if (size2 <= 0) {
      result.winner = PlayerEnum::kOne;
      break;
    }

    assert(size1 + size2 == kDeckSize);
    decide_shuffle(p1, p2, 1);
    assert(p1.hand_size());
    assert(p2.hand_size());

    play_hand(p1.draw(), p2.draw(), p1, p2, &result);
  }
  return result;
}

inline std::pair<Player, Player> make_players(Strategy s1, Strategy s2) {
  const auto deck_array = make_deck();
  std::vector<uint32_t> deck{deck_array.begin(), deck_array.end()};
  shuffle_hand(deck);
  Player p1{{deck.begin(), deck.begin() + deck.size() / 2}, s1};
  Player p2{{deck.begin() + deck.size() / 2, deck.end()}, s2};
  return {std::move(p1), std::move(p2)};
}

inline Results simulate_strategy(Strategy s1, Strategy s2,
                                 const std::size_t ngames = 10) {
  Results result_struct{s1.id, s2.id};

  uint64_t total_p1_war_cards = 0;
  size_t total_p1_war_count = 0;
  uint64_t total_p2_war_cards = 0;
  size_t total_p2_war_count = 0;

  for (std::size_t i = 0; i < ngames; i++) {
    auto players = make_players(s1, s2);
    Player &p1 = (players.first);
    Player &p2 = (players.second);
    const auto game_result = simulate(p1, p2);

    for (auto c : game_result.war_hands_p1_lost) {
      total_p1_war_cards += c;
      total_p1_war_count++;
    }
    for (auto c : game_result.war_hands_p2_lost) {
      total_p2_war_cards += c;
      total_p2_war_count++;
    }

    switch (game_result.winner) {
    case (PlayerEnum::kOne): {
      result_struct.p1 += 1;
      break;
    }
    case (PlayerEnum::kTwo): {
      result_struct.p2 += 1;
      break;
    }
    default:
      result_struct.tie += 1;
      break;
    }
  }
  result_struct.average_p1_war_lost =
      static_cast<double>(total_p1_war_cards) / total_p1_war_count;

  result_struct.average_p2_war_lost =
      static_cast<double>(total_p2_war_cards) / total_p2_war_count;

  return result_struct;
}

const std::vector<Strategy> strategies{{
    {0, &combine_and_shuffle_strategy},
    {1, &always_shuffle_strategy},
    {2, &combine_only_strategy},
    {3, &combine_strategy<AverageEnrichment, true>},
    {4, &combine_strategy<AverageEnrichment, false>},
    {5, &combine_strategy<AcesEnrichment, true>},
    {6, &combine_strategy<AcesEnrichment, false>},
    {7, &combine_strategy<FaceCardEnrichment, true>},
    {8, &combine_strategy<FaceCardEnrichment, false>},
}};
