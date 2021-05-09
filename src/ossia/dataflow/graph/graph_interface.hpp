#pragma once
#include <ossia/dataflow/dataflow_fwd.hpp>
#include <ossia/dataflow/transport.hpp>

#include <smallfun.hpp>

namespace spdlog
{
class logger;
}
namespace ossia
{
struct bench_map;
class time_interval;
class OSSIA_EXPORT graph_interface
{
public:
  virtual ~graph_interface();
  virtual void add_node(ossia::node_ptr) = 0;
  virtual void remove_node(const ossia::node_ptr&) = 0;

  virtual void connect(ossia::edge_ptr) = 0;
  virtual void disconnect(const ossia::edge_ptr&) = 0;
  virtual void disconnect(ossia::graph_edge*) = 0;

  virtual void mark_dirty() = 0;

  virtual void state(execution_state& e) = 0;

  virtual void clear() = 0;

  virtual void print(std::ostream&) = 0;

  virtual const std::vector<ossia::graph_node*>& get_nodes() const noexcept
      = 0;
};

struct graph_setup_options
{
  enum
  {
    StaticFixed,
    StaticBFS,
    StaticTC,
    Dynamic
  } scheduling{};
  enum
  {
    Creation,
    XY,
    YX,
    Temporal
  } order{};
  enum
  {
    Merge,
    Append,
    Replace
  } merge{};

  bool parallel{};
  std::shared_ptr<spdlog::logger> log{};
  std::shared_ptr<bench_map> bench{};
};

struct tick_setup_options
{
  enum
  {
    Default,
    Ordered,
    Priorized,
    Merged
  } commit{};
  enum
  {
    Buffer,
    ScoreAccurate,
    Precise
  } tick{};
};

OSSIA_EXPORT
std::shared_ptr<ossia::graph_interface> make_graph(const graph_setup_options&);

}
