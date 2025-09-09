#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

#include "war-simulator/war-simulator.hpp"
#include <future>

static std::random_device rd;
std::mt19937 gen(rd());

static inline void print_vector(std::vector<Results> &results) {
  std::cout << "S1, S2, P1, P2, Tie, P1 Turn Loss Average, P2 Turn Loss "
               "Average, Hands, Games\n";
  for (auto &res : results) {
    std::cout << res << "\n";
  }
}

int main(int argc, const char *argv[]) {
  char *end = nullptr;

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " N_GAMES [indices...]\n";
    return 1;
  }

  // number of games
  std::size_t n_games = strtoul(argv[1], &end, 10);
  if (n_games == 0) {
    n_games = kGameCount;
  }

  // strategy indices from CLI args
  std::vector<std::size_t> selected_indices;
  if (argc > 2) {
    for (int i = 2; i < argc; i++) {
      std::size_t idx = strtoul(argv[i], &end, 10);
      if (idx >= strategies.size()) {
        std::cerr << "Invalid strategy index " << idx << " (max allowed "
                  << strategies.size() - 1 << ")\n";
        return 1;
      }
      selected_indices.push_back(idx);
    }
  } else {
    // default: all strategies
    selected_indices.resize(strategies.size());
    for (std::size_t i = 0; i < strategies.size(); ++i) {
      selected_indices[i] = i;
    }
  }

  // run matrix of chosen strategies
  std::vector<std::future<Results>> futures;
  for (auto i : selected_indices) {
    for (auto j : selected_indices) {
      futures.push_back(std::async(std::launch::async, [=]() {
        return simulate_strategy(strategies[i], strategies[j], n_games);
      }));
    }
  }

  // collect results
  std::vector<Results> results;
  results.reserve(futures.size());
  for (auto &fut : futures) {
    results.push_back(fut.get());
  }

  print_vector(results);
  return 0;
}
