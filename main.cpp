#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "json.hpp"

using json = nlohmann::json;

class DpProductionPlanner
{

  struct State
  {
    std::vector<std::optional<int>> decisions;
    std::optional<int> optimal_cost;
    std::optional<int> optimal_decision;
  };

  struct Stage
  {
    std::vector<State> states;
  };

  static auto init_stages(int production_capacity, int store_capacity, int stages_count)
  {
    std::vector<Stage> stages;

    stages.resize(stages_count);

    for (auto &stage : stages)
    {
      stage.states.resize(store_capacity + 1);

      for (auto &state : stage.states)
        state.decisions.resize(production_capacity + 1);
    }

    return stages;
  }

  static std::string to_string(const std::optional<int> &opt_int)
  {
    if (opt_int)
      return std::to_string(opt_int.value());

    return "-";
  }

  static void print_stage(const Stage &stage)
  {
    for (auto &&state : stage.states)
    {
      for (auto &&x : state.decisions)
      {
        std::cout << to_string(x) << "\t";
      }
      std::cout << "||\t";
      std::cout << to_string(state.optimal_cost) << "\t";
      std::cout << to_string(state.optimal_decision);
      std::cout << std::endl;
    }
    std::cout << "______________________________" << std::endl
              << std::endl;
  }

  const std::size_t production_capacity;
  const std::size_t store_capacity;
  const std::size_t store_cost;
  const std::size_t constant_production_cost;

  std::vector<Stage> stages;
  std::vector<int> requests;
  std::vector<int> reversed_requests;

public:
  DpProductionPlanner(const std::size_t i_production_capacity,
                      const std::size_t i_store_capacity,
                      const std::size_t i_store_cost,
                      const std::size_t i_constant_production_cost,
                      const std::vector<int> &i_requests) : production_capacity(i_production_capacity),
                                                            store_capacity(i_store_capacity),
                                                            store_cost(i_store_cost),
                                                            constant_production_cost(i_constant_production_cost),
                                                            stages(init_stages(production_capacity, store_capacity, i_requests.size())),
                                                            requests(i_requests),
                                                            reversed_requests(i_requests.size())
  {
    std::reverse_copy(i_requests.begin(), i_requests.end(), reversed_requests.begin());
  }

  void do_the_thing()
  {
    for (int stage_it = 0; stage_it < reversed_requests.size(); ++stage_it)
    {
      for (int state = 0; state <= store_capacity; ++state)
      {
        std::optional<int> optimal_cost;
        std::optional<int> optimal_decision;

        for (int x = 0; x <= production_capacity; ++x)
        {
          int total_supply = state + x;

          // Last stage
          if (stage_it == reversed_requests.size() - 1 && state > 0)
            continue;

          // Initial stage
          if (stage_it == 0 && total_supply != reversed_requests[stage_it])
            continue;

          if (total_supply < reversed_requests[stage_it])
            continue;

          int to_store = total_supply - reversed_requests[stage_it];
          if (stage_it > 0 && !stages[stage_it - 1].states[to_store].optimal_cost)
          {
            continue;
          }

          int production_cost = x > 0 ? constant_production_cost : 0;
          int current_store_cost = store_cost * state;
          int total_cost = production_cost + current_store_cost;

          if (stage_it > 0)
            total_cost +=
                stages[stage_it - 1].states[to_store].optimal_cost.value();

          stages[stage_it].states[state].decisions[x] = total_cost;
          if (!optimal_cost || optimal_cost.value() > total_cost)
          {
            optimal_cost = total_cost;
            optimal_decision = x;
          }
        }

        stages[stage_it].states[state].optimal_cost = optimal_cost;
        stages[stage_it].states[state].optimal_decision = optimal_decision;
      }
      print_stage(stages[stage_it]);
    }
  }
};

int main()
{
  std::ifstream config_stream("config.json");
  json config;
  config_stream >> config;

  std::vector<int> requests(config["request"].size());
  std::copy(config["request"].begin(), config["request"].end(), requests.begin());

  DpProductionPlanner dpp(config["production"]["capacity"],
                          config["store"]["capacity"],
                          config["store"]["cost"],
                          config["production"]["constant_cost"],
                          requests);

  dpp.do_the_thing();

  return 0;
}