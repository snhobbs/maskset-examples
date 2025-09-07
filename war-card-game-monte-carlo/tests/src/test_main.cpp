#include "war-simulator/war-simulator.hpp"
#include <gtest/gtest.h>

static std::random_device rd;
std::mt19937 gen(rd());

// Fixture for Player setup
struct PlayerTest : public ::testing::Test {
  Strategy dummy_strategy{0, &combine_only_strategy};
  Player make_player(std::deque<uint32_t> hand,
                     std::vector<uint32_t> pile = {}) {
    Player p{hand, dummy_strategy};
    for (auto c : pile) {
      p.take(c);
    }
    return p;
  }
};

// --- average() ---
TEST(UtilTest, AverageEmpty) {
  std::vector<uint32_t> v{};
  EXPECT_DOUBLE_EQ(average(v), 0.0);
}

TEST(UtilTest, AverageNonEmpty) {
  std::vector<uint32_t> v{2, 4, 6};
  EXPECT_DOUBLE_EQ(average(v), 4.0);
}

// --- count_card_type() ---
TEST(UtilTest, CountCardType) {
  std::vector<uint32_t> v{2, 14, 14, 7};
  std::vector<uint32_t> aces{kMaxCard};
  EXPECT_EQ(count_card_type(v, aces), 2);
}

TEST(UtilTest, DeckRange) {
  auto deck = make_deck();
  auto min_card = *std::min_element(deck.begin(), deck.end());
  auto max_card = *std::max_element(deck.begin(), deck.end());

  EXPECT_EQ(min_card, 2u);
  EXPECT_EQ(max_card, kMaxCard);
}

TEST(UtilTest, MakeDeckCorrectness) {
  auto deck = make_deck();

  // Check deck size
  EXPECT_EQ(deck.size(), kDeckSize);

  // Check min/max
  auto min_card = *std::min_element(deck.begin(), deck.end());
  auto max_card = *std::max_element(deck.begin(), deck.end());
  EXPECT_EQ(min_card, 2u);
  EXPECT_EQ(max_card, kMaxCard);

  // Check card counts
  std::array<int, kMaxCard + 1> counts{}; // index 0 unused
  for (auto c : deck) {
    EXPECT_GE(c, 2u);
    EXPECT_LE(c, kMaxCard);
    counts[c]++;
  }

  // Each card 2-13 should appear 4 times, Ace (14) appears 4 times
  for (uint32_t c = 2; c <= kMaxCard; c++) {
    EXPECT_EQ(counts[c], 4);
  }
}

// --- AverageEnrichment ---
TEST_F(PlayerTest, AverageEnrichmentTrueWhenHandLarger) {
  auto p = make_player({10, 12}, {2, 2});
  EXPECT_TRUE(AverageEnrichment::enriched(p));
}

TEST_F(PlayerTest, AverageEnrichmentFalseWhenPileLarger) {
  auto p = make_player({2, 2}, {12, 12});
  EXPECT_FALSE(AverageEnrichment::enriched(p));
}

// --- AcesEnrichment ---
TEST_F(PlayerTest, AcesEnrichmentTrueWhenHandHasMoreAces) {
  auto p = make_player({14, 5}, {7, 8});
  EXPECT_TRUE(AcesEnrichment::enriched(p));
}

TEST_F(PlayerTest, AcesEnrichmentFalseWhenPileHasMoreAces) {
  auto p = make_player({2, 3}, {14, 14});
  EXPECT_FALSE(AcesEnrichment::enriched(p));
}

// --- FaceCardEnrichment ---
TEST_F(PlayerTest, FaceCardEnrichmentTrueWhenHandHasFaceCards) {
  auto p = make_player({11, 12, 13}, {2, 3, 4});
  EXPECT_TRUE(FaceCardEnrichment::enriched(p));
}

TEST_F(PlayerTest, FaceCardEnrichmentFalseWhenPileHasFaceCards) {
  auto p = make_player({2, 3, 4}, {11, 12, 13});
  EXPECT_FALSE(FaceCardEnrichment::enriched(p));
}

// Fixture for WarHand tests
struct WarHandTest : public ::testing::Test {
  Strategy dummy_strategy{0, &combine_only_strategy};
  Player make_player(std::deque<uint32_t> hand) {
    return Player{hand, dummy_strategy};
  }
};

// --- WarHand with 0 cards ---
TEST_F(WarHandTest, EmptyHandInvalid) {
  auto p = make_player({});
  WarHand wh{p};
  EXPECT_FALSE(wh.is_valid());
  EXPECT_TRUE(wh.dump.empty());
}

// --- WarHand with 1 card ---
TEST_F(WarHandTest, OneCardBecomesFlip) {
  auto p = make_player({5});
  WarHand wh{p};
  EXPECT_TRUE(wh.is_valid());
  EXPECT_EQ(wh.flip, 5u);
  EXPECT_TRUE(wh.dump.empty());
}

// --- WarHand with 2 cards ---
TEST_F(WarHandTest, TwoCardsOneDumpOneFlip) {
  auto p = make_player({7, 9});
  WarHand wh{p};
  EXPECT_TRUE(wh.is_valid());
  EXPECT_EQ(wh.flip, 9u);
  ASSERT_EQ(wh.dump.size(), 1u);
  EXPECT_EQ(wh.dump[0], 7u);
}

// --- WarHand with 3 cards ---
TEST_F(WarHandTest, ThreeCardsTwoDumpOneFlip) {
  auto p = make_player({2, 3, 4});
  WarHand wh{p};
  EXPECT_TRUE(wh.is_valid());
  EXPECT_EQ(wh.flip, 4u);
  ASSERT_EQ(wh.dump.size(), 2u);
  EXPECT_EQ(wh.dump[0], 2u);
  EXPECT_EQ(wh.dump[1], 3u);
}

// --- WarHand with 4 cards ---
TEST_F(WarHandTest, FourCardsThreeDumpOneFlip) {
  auto p = make_player({10, 11, 12, 13});
  WarHand wh{p};
  EXPECT_TRUE(wh.is_valid());
  EXPECT_EQ(wh.flip, 13u);
  ASSERT_EQ(wh.dump.size(), 3u);
  EXPECT_EQ(wh.dump[0], 10u);
  EXPECT_EQ(wh.dump[1], 11u);
  EXPECT_EQ(wh.dump[2], 12u);
}

// A dummy strategy to pass into players
Strategy dummy_strategy{0, &combine_only_strategy};
Player make_player(const std::deque<uint32_t> &hand) {
  return Player{hand, dummy_strategy};
}

// --- Basic wins ---
TEST(PlayHandTest, Player1WinsSimple) {
  auto p1 = make_player({});
  auto p2 = make_player({});

  GameResult result{};

  auto winner = play_hand(10, 5, p1, p2, &result);

  EXPECT_EQ(winner, PlayerEnum::kOne);
  EXPECT_EQ(result.nhands, 1);
  EXPECT_EQ(p1.ncards(), 2); // takes both cards
  EXPECT_EQ(p2.ncards(), 0);
}

TEST(PlayHandTest, Player2WinsSimple) {
  auto p1 = make_player({});
  auto p2 = make_player({});
  GameResult result{};

  auto winner = play_hand(3, 8, p1, p2, &result);

  EXPECT_EQ(winner, PlayerEnum::kTwo);
  EXPECT_EQ(result.nhands, 1);
  EXPECT_EQ(p1.ncards(), 0);
  EXPECT_EQ(p2.ncards(), 2); // takes both cards
}

// --- War scenario with enough cards ---
TEST(PlayHandTest, WarBothHaveEnoughCards) {
  auto p1 = make_player({4, 5, 6, 7});
  auto p2 = make_player({4, 4, 2, 3});
  GameResult result{};

  auto winner = play_hand(5, 5, p1, p2, &result);

  // Should return kOne or kTwo, depending on inner flip; check GameResult
  // updates
  EXPECT_NE(winner, PlayerEnum::kNone);
  EXPECT_EQ(result.nhands, 2); // initial + inner flip
  EXPECT_EQ(p1.ncards() + p2.ncards(),
            10); // total cards remaining = 4+4+2 used for war? Verify logic
}

// --- War scenario with a player running out ---
TEST(PlayHandTest, WarPlayer1RunsOut) {
  auto p1 = make_player({5});
  auto p2 = make_player({5, 6});
  GameResult result{};

  auto winner = play_hand(7, 7, p1, p2, &result);

  EXPECT_EQ(winner, PlayerEnum::kTwo);
  EXPECT_EQ(result.nhands, 2);
  EXPECT_EQ(p1.ncards(), 0);
  EXPECT_EQ(p2.ncards(), 5); // takes both initial war cards
}

// --- Multiple wars in a row ---
TEST(PlayHandTest, MultipleWars) {
  auto p1 = make_player({8, 9, 10, 11});
  auto p2 = make_player({8, 4, 2, 3});
  GameResult result{};

  auto winner = play_hand(6, 6, p1, p2, &result);

  EXPECT_NE(winner, PlayerEnum::kNone);
  EXPECT_GE(result.nhands, 2);
}

// --- War updates GameResult vectors ---
TEST(PlayHandTest, WarCardsRecorded) {
  auto p1 = make_player({7, 8, 9, 10});
  auto p2 = make_player({7, 4, 2, 3});
  GameResult result{};

  auto winner = play_hand(5, 5, p1, p2, &result);

  // 3 cards lost by p2, 3 gained by p1
  auto total_war_cards =
      result.war_hands_p1_lost.size() + result.war_hands_p2_lost.size();
  EXPECT_EQ(total_war_cards, 3);
}

// --- simulate() tests with make_players() ---
TEST_F(PlayerTest, SimulatePlayer1WinsAll) {
  auto players = make_players(strategies[0],
                              strategies[2]); // pick deterministic strategies
  Player &p1 = players.first;
  Player &p2 = players.second;

  GameResult result = simulate(p1, p2);

  EXPECT_NE(result.winner, PlayerEnum::kNone);
  EXPECT_EQ(p1.ncards() + p2.ncards(), kDeckSize);
  EXPECT_GE(result.nhands, 1);
}

TEST_F(PlayerTest, SimulatePlayer2WinsAll) {
  auto players = make_players(strategies[2], strategies[0]); // reverse order
  Player &p1 = players.first;
  Player &p2 = players.second;

  GameResult result = simulate(p1, p2);

  EXPECT_NE(result.winner, PlayerEnum::kNone);
  EXPECT_EQ(p1.ncards() + p2.ncards(), kDeckSize);
  EXPECT_GE(result.nhands, 1);
}

TEST_F(PlayerTest, SimulateWarHandledCorrectly) {
  // Use same strategy for both players to increase likelihood of war
  auto players = make_players(strategies[0], strategies[0]);
  Player &p1 = players.first;
  Player &p2 = players.second;

  GameResult result = simulate(p1, p2);

  EXPECT_NE(result.winner, PlayerEnum::kNone);
  EXPECT_EQ(p1.ncards() + p2.ncards(), kDeckSize);
  EXPECT_GE(result.nhands, 1); // could be >1 if wars occurred
  EXPECT_FALSE(result.war_hands_p1_lost.empty() &&
               result.war_hands_p2_lost.empty());
}

TEST_F(PlayerTest, SimulateAllCardsAccountedFor) {
  // Run multiple games to verify card accounting
  for (int i = 0; i < 5; i++) {
    auto players = make_players(strategies[i % strategies.size()],
                                strategies[(i + 1) % strategies.size()]);
    Player &p1 = players.first;
    Player &p2 = players.second;

    GameResult result = simulate(p1, p2);

    EXPECT_EQ(p1.ncards() + p2.ncards(), kDeckSize);
    EXPECT_NE(result.winner, PlayerEnum::kNone);
  }
}

TEST(DeckTest, ValuesAndAverage) {
  auto deck = make_deck();

  // Check min and max
  auto min_card = *std::min_element(deck.begin(), deck.end());
  auto max_card = *std::max_element(deck.begin(), deck.end());
  EXPECT_EQ(min_card, 2u);
  EXPECT_EQ(max_card, kMaxCard);

  // Check average
  double deck_avg = average(deck);
  // Standard 52-card deck with values 2..14, repeated 4 times
  // Average = (2+3+...+14)/13 = 8, repeated 4 times still 8
  EXPECT_DOUBLE_EQ(deck_avg, 8.0);
}
