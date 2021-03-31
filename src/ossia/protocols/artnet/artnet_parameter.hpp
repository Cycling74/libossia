#pragma once
#include <ossia/detail/config.hpp>
#if defined(OSSIA_PROTOCOL_ARTNET)
#include <ossia/protocols/artnet/artnet_protocol.hpp>
#include <ossia/network/common/device_parameter.hpp>

#include <artnet/artnet.h>

#include <cstdint>

namespace ossia
{
namespace net
{

class artnet_parameter : public device_parameter
{
  using dmx_buffer = artnet_protocol::dmx_buffer;

public:
  artnet_parameter(
      net::node_base& node, dmx_buffer* buffer, const unsigned int channel);
  ~artnet_parameter();

private:
  void device_update_value() override;

  dmx_buffer& m_buffer;
  const unsigned int m_channel;

  friend struct artnet_visitor;
};
}
}
#endif
