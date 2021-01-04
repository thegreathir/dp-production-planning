#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <iomanip>

#include "json.hpp"

using json = nlohmann::json;

void print_table(const std::vector<std::vector<std::string>> &table)
{
  constexpr char boxing[] = "++|-+++++++";
  constexpr std::size_t pad_size = 2;
  std::vector<std::size_t> columns_width;
  for (std::size_t col_index = 0; col_index < table[0].size(); ++col_index)
  {
    std::optional<std::size_t> max_width;
    for (std::size_t row_index = 0; row_index < table.size(); ++row_index)
    {
      if (!max_width || max_width.value() < table[row_index][col_index].length() + pad_size)
        max_width = table[row_index][col_index].length() + pad_size;
    }

    columns_width.emplace_back(max_width.value());
  }

  std::cout << boxing[8];
  for (std::size_t col_index = 0; col_index < table[0].size(); ++col_index)
  {
    std::cout << std::string(columns_width[col_index], boxing[3]);
    if (col_index < table[0].size() - 1)
      std::cout << boxing[1];
    else
      std::cout << boxing[7] << std::endl;
  }

  for (std::size_t row_index = 0; row_index < table.size(); ++row_index)
  {
    std::cout << boxing[2];
    for (std::size_t col_index = 0; col_index < table[0].size(); ++col_index)
    {
      std::cout << std::setw(columns_width[col_index]) << table[row_index][col_index];
      std::cout << boxing[2];
    }
    std::cout << std::endl;

    if (row_index == table.size() - 1)
      continue;
    std::cout << boxing[10];
    for (std::size_t col_index = 0; col_index < table[0].size(); ++col_index)
    {
      std::cout << std::string(columns_width[col_index], boxing[3]);
      if (col_index < table[0].size() - 1)
        std::cout << boxing[4];
      else
        std::cout << boxing[9] << std::endl;
    }
  }
  std::cout << boxing[5];
  for (std::size_t col_index = 0; col_index < table[0].size(); ++col_index)
  {
    std::cout << std::string(columns_width[col_index], boxing[3]);
    if (col_index < table[0].size() - 1)
      std::cout << boxing[0];
    else
      std::cout << boxing[6] << std::endl;
  }
}

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
    std::vector<std::vector<std::string>> table;

    std::vector<std::string> header;
    header.push_back("s\\x");
    for (std::size_t i = 0; i < stage.states[0].decisions.size(); ++i)
      header.push_back(std::to_string(i));
    header.push_back("optimal cost");
    header.push_back("x*");

    table.push_back(header);

    for (std::size_t state_it = 0; state_it < stage.states.size(); ++state_it)
    {
      std::vector<std::string> row;
      row.push_back(std::to_string(state_it));
      for (std::size_t x = 0; x < stage.states[state_it].decisions.size(); ++x)
      {
        row.push_back(to_string(stage.states[state_it].decisions[x]));
      }
      row.push_back(to_string(stage.states[state_it].optimal_cost));
      row.push_back(to_string(stage.states[state_it].optimal_decision));
      table.push_back(row);
    }

    print_table(table);
    std::cout << std::endl;
  }

  const std::size_t production_capacity;
  const std::size_t store_capacity;
  const std::size_t store_cost;
  const std::size_t constant_production_cost;
  const std::size_t good_production_cost;

  std::vector<Stage> stages;
  std::vector<int> requests;
  std::vector<int> reversed_requests;

public:
  void trace_stages()
  {
    std::vector<std::size_t> results;
    std::size_t used_store_space = 0;
    for (int stage_index = stages.size() - 1; stage_index >= 0; stage_index--)
    {
      const auto &state = stages[stage_index].states[used_store_space];
      if (state.optimal_decision)
      {
        results.emplace_back(state.optimal_decision.value());
        if (stage_index > 0)
          used_store_space += state.optimal_decision.value() - reversed_requests[stage_index];
      }
      else
      {
        std::cout << "No solution found!" << std::endl;
        return;
      }
    }
    std::cout << "Optimal decisions:" << std::endl;
    for (auto iterator = results.begin(); iterator < results.end(); iterator++)
      std::cout << "x" << (iterator - results.begin()) << ": " << (*iterator) << std::endl;

    std::size_t total_cost = 0;
    for (auto &&request : requests)
      total_cost += good_production_cost * request;

    total_cost += stages.back().states[0].optimal_cost.value();

    std::cout << "Total cost: " << total_cost << std::endl;
  }

  DpProductionPlanner(const std::size_t i_production_capacity,
                      const std::size_t i_store_capacity,
                      const std::size_t i_store_cost,
                      const std::size_t i_constant_production_cost,
                      const std::size_t i_good_production_cost,
                      const std::vector<int> &i_requests) : production_capacity(i_production_capacity),
                                                            store_capacity(i_store_capacity),
                                                            store_cost(i_store_cost),
                                                            constant_production_cost(i_constant_production_cost),
                                                            good_production_cost(i_good_production_cost),
                                                            stages(init_stages(production_capacity, store_capacity, i_requests.size())),
                                                            requests(i_requests),
                                                            reversed_requests(i_requests.size())
  {
    std::reverse_copy(i_requests.begin(), i_requests.end(), reversed_requests.begin());
  }

  void calculate_stages()
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
      std::cout << "Stage " << reversed_requests.size() - stage_it << ":" << std::endl;
      print_stage(stages[stage_it]);
    }
  }
};

int main()
{
  std::ifstream config_stream("config.json");
  json config;
  config_stream >> config;

  std::vector<int> requests(config["requests"].size());
  std::copy(config["requests"].begin(), config["requests"].end(), requests.begin());

  DpProductionPlanner dpp(config["production"]["capacity"],
                          config["store"]["capacity"],
                          config["store"]["cost"],
                          config["production"]["constant_cost"],
                          config["production"]["good_cost"],
                          requests);

  dpp.calculate_stages();
  dpp.trace_stages();

  return 0;
}