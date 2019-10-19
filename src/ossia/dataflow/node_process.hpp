#pragma once
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <ossia/editor/scenario/time_process.hpp>

namespace ossia
{

class OSSIA_EXPORT node_process : public ossia::time_process
{
public:
  node_process(ossia::node_ptr n);
  ~node_process() override;
  void offset_impl(ossia::time_value, double pos) override;
  void transport_impl(ossia::time_value date, double pos) override;

  void state_impl(ossia::token_request) override;

  void start() override;
  void stop() override;
  void pause() override;
  void resume() override;
  void mute_impl(bool) override;
};
}
