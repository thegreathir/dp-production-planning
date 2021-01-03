#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "json.hpp"

using json = nlohmann::json;

class State {
public:
  std::vector<std::optional<int>> decisions;
  std::optional<int> optimal_cost;
  std::optional<int> optimal_decision;
};

class Stage {
public:
  std::vector<State> states;
};

auto init_stages(int production_cap, int store_cap, int stages_count) {
  std::vector<Stage> stages;

  stages.resize(stages_count);

  for (auto &stage : stages) {
    stage.states.resize(store_cap + 1);

    for (auto &state : stage.states)
      state.decisions.resize(production_cap + 1);
  }

  return stages;
}

std::string to_string(const std::optional<int> &opt_int) {
  if (opt_int)
    return std::to_string(opt_int.value());

  return "-";
}

void print_stage(const Stage &stage) {
  for (auto &&state : stage.states) {
    for (auto &&x : state.decisions) {
      std::cout << to_string(x) << "\t";
    }
    std::cout << "||\t";
    std::cout << to_string(state.optimal_cost) << "\t";
    std::cout << to_string(state.optimal_decision);
    std::cout << std::endl;
  }
  std::cout << "______________________________" << std::endl << std::endl;
}

int main() {

  std::ifstream config_stream("config.json");
  json config;
  config_stream >> config;

  auto stages =
      init_stages(config["production"]["capacity"], config["store"]["capacity"],
                  config["request"].size());

  std::vector<int> requests(config["request"].size());
  std::reverse_copy(config["request"].begin(), config["request"].end(),
                    requests.begin());

  int store_cap = config["store"]["capacity"];
  int production_cap = config["production"]["capacity"];
  int const_cost = config["production"]["constant_cost"];
  int store_cost = config["store"]["cost"];

  for (int stage_it = 0; stage_it < requests.size(); ++stage_it) {
    for (int state = 0; state <= store_cap; ++state) {

      std::optional<int> optimal_cost;
      std::optional<int> optimal_decision;
      for (int x = 0; x <= production_cap; ++x) {
        int total_supply = state + x;

        // Last stage
        if (stage_it == requests.size() - 1 && state > 0)
          continue;

        // Initial stage
        if (stage_it == 0 && total_supply != requests[stage_it])
          continue;

        if (total_supply < requests[stage_it])
          continue;

        int to_store = total_supply - requests[stage_it];
        if (stage_it > 0 && !stages[stage_it - 1].states[to_store].optimal_cost) {
          continue;
        }

        int production_cost = x > 0 ? const_cost : 0;
        int current_store_cost = store_cost * state;
        int total_cost = production_cost + current_store_cost;

        if (stage_it > 0)
          total_cost +=
              stages[stage_it - 1].states[to_store].optimal_cost.value();

        stages[stage_it].states[state].decisions[x] = total_cost;
        if (!optimal_cost || optimal_cost.value() > total_cost) {
          optimal_cost = total_cost;
          optimal_decision = x;
        }
      }

      stages[stage_it].states[state].optimal_cost = optimal_cost;
      stages[stage_it].states[state].optimal_decision = optimal_decision;
    }
    print_stage(stages[stage_it]);
  }

  return 0;
}