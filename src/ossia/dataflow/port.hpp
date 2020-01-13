#pragma once
#include <ossia/dataflow/data.hpp>
#include <ossia/network/common/path.hpp>
namespace ossia
{
struct OSSIA_EXPORT port
{
  data_type data;

  enum scope_t : uint8_t
  {
    none = 1 << 0,
    local = 1 << 1,
    global = 1 << 2,
    both = local | global
  };

  scope_t scope{scope_t::both};

protected:
  port(data_type&& d) noexcept : data{std::move(d)}
  {
  }

  port() = delete;
  port(const port&) = delete;
  port(port&&) = delete;
  port& operator=(const port&) = delete;
  port& operator=(port&&) = delete;
};

struct OSSIA_EXPORT inlet : public port
{
  inlet(data_type&& d) noexcept : port{std::move(d)}
  {
  }
  inlet(data_type&& d, destination_t dest) noexcept
      : port{std::move(d)}, address{std::move(dest)}
  {
  }

  inlet(data_type&& d, ossia::net::parameter_base& addr) noexcept
      : port{std::move(d)}, address{&addr}
  {
  }

  inlet(data_type&& d, graph_edge& edge) noexcept : port{std::move(d)}
  {
    sources.push_back(&edge);
  }

  ~inlet()
  {

  }

  void connect(graph_edge* e) noexcept
  {
    auto it = ossia::find(sources, e);
    if (it == sources.end())
      sources.push_back(e);
  }

  void disconnect(graph_edge* e) noexcept
  {
    ossia::remove_erase(sources, e);
  }

  destination_t address;
  ossia::small_vector<graph_edge*, 2> sources;
};

struct OSSIA_EXPORT outlet : public port
{
  outlet(data_type&& d) noexcept : port{std::move(d)}
  {
  }
  outlet(data_type&& d, destination_t dest) noexcept
      : port{std::move(d)}, address{std::move(dest)}
  {
  }

  outlet(data_type&& d, ossia::net::parameter_base& addr) noexcept
      : port{std::move(d)}, address{&addr}
  {
  }

  outlet(data_type&& d, graph_edge& edge) noexcept : port{std::move(d)}
  {
    targets.push_back(&edge);
  }

  ~outlet()
  {

  }

  void connect(graph_edge* e) noexcept
  {
    auto it = ossia::find(targets, e);
    if (it == targets.end())
      targets.push_back(e);
  }

  void disconnect(graph_edge* e) noexcept
  {
    ossia::remove_erase(targets, e);
  }

  void write(execution_state& e);

  destination_t address;
  ossia::small_vector<graph_edge*, 2> targets;
};

template <typename T, typename... Args>
inlet_ptr make_inlet(Args&&... args)
{
  return new inlet(T{}, std::forward<Args>(args)...);
}
template <typename T, typename... Args>
outlet_ptr make_outlet(Args&&... args)
{
  return new outlet(T{}, std::forward<Args>(args)...);
}
}
