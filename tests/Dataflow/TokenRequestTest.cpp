#include <ossia/detail/config.hpp>
#include <iterator>
#include <ossia/dataflow/token_request.hpp>

#include <catch.hpp>
#include "../Editor/TestUtils.hpp"
TEST_CASE ("test_loops_integer", "test_loops_integer")
{
  using namespace ossia;

  token_request r;
  r.prev_date = 0_tv;
  r.date = 100_tv;

  token_request_vec vec;
  r.loop(0_tv, 10_tv, [&] (auto& req) { vec.push_back(req); }, [] (const auto&){ });

  simple_token_request_vec expected{
    {.prev_date = 0_tv, .date = 10_tv, .offset = 0_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 10_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 20_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 30_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 40_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 50_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 60_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 70_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 80_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 90_tv}
  };

  REQUIRE(expected == vec);
}

TEST_CASE ("test_loops_fract", "test_loops_fract")
{
  using namespace ossia;

  token_request r;
  r.prev_date = 0_tv;
  r.date = 97_tv;

  token_request_vec vec;
  r.loop(0_tv, 10_tv, [&] (auto& req) { vec.push_back(req); }, [](const auto&) { });

  simple_token_request_vec expected{
    {.prev_date = 0_tv, .date = 10_tv, .offset = 0_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 10_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 20_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 30_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 40_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 50_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 60_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 70_tv},
    {.prev_date = 0_tv, .date = 10_tv, .offset = 80_tv},
    {.prev_date = 0_tv, .date = 7_tv,  .offset = 90_tv}
  };

  REQUIRE(expected == vec);
}

TEST_CASE ("test_loops_offset", "test_loops_offset")
{
  using namespace ossia;

  token_request r;
  r.prev_date = 2_tv;
  r.date = 12_tv;

  token_request_vec vec;
  r.loop(0_tv, 5_tv, [&] (auto& req) { vec.push_back(req); }, [] (const auto&){ });

  simple_token_request_vec expected{
    {.prev_date = 2_tv, .date = 5_tv, .offset = 0_tv},
    {.prev_date = 0_tv, .date = 5_tv, .offset = 3_tv},
    {.prev_date = 0_tv, .date = 2_tv, .offset = 8_tv},
  };

  REQUIRE(expected == vec);
}

TEST_CASE ("test_loops_smaller", "test_loops_smaller")
{
  using namespace ossia;

  token_request r;
  r.prev_date = 0_tv;
  r.date = 15_tv;

  token_request_vec vec;
  r.loop(0_tv, 20_tv, [&] (auto& req) { vec.push_back(req); }, [](const auto&) { });

  simple_token_request_vec expected{
    {.prev_date = 0_tv, .date = 15_tv, .offset = 0_tv}
  };

  REQUIRE(expected == vec);
}

TEST_CASE ("test_loops_mid", "test_loops_mid")
{
  using namespace ossia;

  token_request r;
  r.prev_date = 15_tv;
  r.date = 30_tv;

  token_request_vec vec;
  r.loop(0_tv, 20_tv, [&] (auto& req) { vec.push_back(req); }, [] (const auto&){ });

  simple_token_request_vec expected{
    {.prev_date = 15_tv, .date = 20_tv, .offset = 0_tv},
    {.prev_date = 0_tv,  .date = 10_tv, .offset = 5_tv}
  };

  REQUIRE(expected == vec);
}
