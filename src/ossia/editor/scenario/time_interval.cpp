// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia/dataflow/nodes/forward_node.hpp>
#include <ossia/detail/algorithms.hpp>
#include <ossia/detail/logger.hpp>
#include <ossia/editor/scenario/time_event.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <ossia/editor/scenario/time_process.hpp>

#include <algorithm>
#include <iostream>
namespace ossia
{

void time_interval::tick_impl(
    ossia::time_value old_date, ossia::time_value new_date,
    ossia::time_value offset, const ossia::token_request& parent_request)
{
  m_tick_offset = offset;
  m_date = new_date;

  m_current_signature = signature(new_date, parent_request);
  m_current_tempo = tempo(new_date, parent_request);

  state(old_date, new_date);
  if (m_callback)
    (*m_callback)(new_date);
}

void time_interval::tick_current(ossia::time_value offset, const ossia::token_request& parent_request)
{
  tick_impl(m_date, m_date, offset, parent_request);
}

void time_interval::tick(time_value date, const ossia::token_request& parent_request, double ratio)
{
  tick_impl(
      m_date, m_date + std::ceil(date.impl * m_speed / ratio), m_tick_offset, parent_request);
}

void time_interval::tick_offset(time_value date, ossia::time_value offset, const ossia::token_request& parent_request)
{
  tick_impl(m_date, m_date + std::ceil(date.impl * m_speed), offset, parent_request);
}

void time_interval::tick_offset_speed_precomputed(time_value date, ossia::time_value offset, const ossia::token_request& parent_request)
{
  tick_impl(m_date, m_date + date.impl, offset, parent_request);
}

std::shared_ptr<time_interval> time_interval::create(
    time_interval::exec_callback callback, time_event& startEvent,
    time_event& endEvent, ossia::time_value nominal, ossia::time_value min,
    ossia::time_value max)
{
  auto timeInterval = std::make_shared<time_interval>(
      std::move(callback), startEvent, endEvent, nominal, min, max);

  startEvent.next_time_intervals().push_back(timeInterval);
  endEvent.previous_time_intervals().push_back(timeInterval);

  return timeInterval;
}

time_interval::time_interval(
    time_interval::exec_callback callback, time_event& startEvent,
    time_event& endEvent, ossia::time_value nominal, ossia::time_value min,
    ossia::time_value max)
    : node{std::make_shared<ossia::nodes::interval>()}
    , m_callback(std::move(callback))
    , m_start(startEvent)
    , m_end(endEvent)
    , m_nominal(nominal)
    , m_min(min)
    , m_max(max)
{
}

time_interval::~time_interval()
{
}

void time_interval::start_and_tick()
{
  start();
  tick_current(0_tv, {});
}

void time_interval::start()
{
  // launch the clock
  if (m_nominal <= m_offset)
    return stop();

  // start all time processes
  for (const auto& timeProcess : get_time_processes())
  {
    if (timeProcess->enabled())
      timeProcess->start();
  }

  // set clock at a tick
  m_running = true;
  m_date = m_offset;

  if (m_callback)
    (*m_callback)(m_date);
}

void time_interval::stop()
{
  // stop all time processes
  for (const std::shared_ptr<ossia::time_process>& timeProcess :
       get_time_processes())
  {
    timeProcess->stop();
  }

  m_date = Zero;
  m_running = false;
}

void time_interval::offset(ossia::time_value date)
{
  m_offset = date;
  m_date = date;

  const auto& processes = get_time_processes();
  const auto N = processes.size();
  if (N > 0)
  {
    // get the state of each TimeProcess at current clock position and date
    for (const auto& timeProcess : processes)
    {
      if (timeProcess->enabled())
      {
        timeProcess->offset(m_date);
      }
    }
  }
  if (m_callback)
    (*m_callback)(m_date);
}

void time_interval::transport(time_value date)
{
  m_offset = date;
  m_date = date;

  const auto& processes = get_time_processes();
  const auto N = processes.size();
  if (N > 0)
  {
    // get the state of each TimeProcess at current clock position and date
    for (const auto& timeProcess : processes)
    {
      if (timeProcess->enabled())
      {
        timeProcess->transport(m_date);
      }
    }
  }
  if (m_callback)
    (*m_callback)(m_date);
}

void time_interval::state(ossia::time_value from, ossia::time_value to)
{
  const auto& processes = get_time_processes();
  const auto N = processes.size();

  if (N > 0)
  {
    const ossia::token_request tok{from, to, m_nominal, m_tick_offset, m_globalSpeed, m_current_signature, m_current_tempo};
    node->request(tok);
    // get the state of each TimeProcess at current clock position and date
    for (const std::shared_ptr<ossia::time_process>& timeProcess : processes)
    {
      time_process& p = *timeProcess;
      if (p.enabled())
      {
        p.state(tok);
      }
    }
  }
}

time_signature time_interval::signature(time_value date, const ossia::token_request& parent_request) const noexcept
{
  if(m_hasSignature && !m_timeSignature.empty())
  {
    auto it = m_timeSignature.lower_bound(date);
    if(it != m_timeSignature.end())
    {
      return it->second;
    }
  }
  return parent_request.signature;
}

double time_interval::tempo(time_value date, const ossia::token_request& parent_request) const noexcept
{
  // TODO tempo should be hierarchic
  if(m_hasTempo)
  {
    return m_tempoCurve.value_at(date.impl);
  }
  return parent_request.tempo;
}

void time_interval::pause()
{
  // pause all time processes
  for (const auto& timeProcess : get_time_processes())
  {
    timeProcess->pause();
  }
}

void time_interval::resume()
{
  // resume all time processes
  for (const auto& timeProcess : get_time_processes())
  {
    timeProcess->resume();
  }
}

void time_interval::set_callback(time_interval::exec_callback cb)
{
  if (cb)
    m_callback = std::move(*cb);
  else
    m_callback = {};
}

void time_interval::set_callback(
    smallfun::function<void(time_value), 32> cb)
{
  m_callback = std::move(cb);
}

void time_interval::set_stateless_callback(time_interval::exec_callback cb)
{
  if (cb)
    m_callback = std::move(*cb);
  else
    m_callback = {};
}

void time_interval::set_stateless_callback(
    smallfun::function<void(time_value), 32> cb)
{
  m_callback = std::move(cb);
}

const time_value& time_interval::get_nominal_duration() const
{
  return m_nominal;
}

time_interval&
time_interval::set_nominal_duration(ossia::time_value durationNominal)
{
  m_nominal = durationNominal;

  if (m_nominal < m_min)
    set_min_duration(m_nominal);

  if (m_nominal > m_max)
    set_max_duration(m_nominal);

  if (!m_max.infinite() && m_date > m_max)
    m_date = m_max;

  return *this;
}

const time_value& time_interval::get_min_duration() const
{
  return m_min;
}

time_interval& time_interval::set_min_duration(ossia::time_value durationMin)
{
  m_min = durationMin;

  if (m_min > m_nominal)
    set_nominal_duration(m_min);

  return *this;
}

const time_value& time_interval::get_max_duration() const
{
  return m_max;
}

time_interval& time_interval::set_max_duration(ossia::time_value durationMax)
{
  m_max = durationMax;

  if (durationMax < m_nominal)
    set_nominal_duration(m_max);

  return *this;
}

time_event& time_interval::get_start_event() const
{
  return m_start;
}

time_event& time_interval::get_end_event() const
{
  return m_end;
}

void time_interval::add_time_process(std::shared_ptr<time_process> timeProcess)
{
  if (!timeProcess)
    return;

  // todo what if the interval started
  if (m_running)
    timeProcess->start();

  if(bool b = node->muted())
    timeProcess->mute(b);

  // store a TimeProcess if it is not already stored
  if (find(m_processes, timeProcess) == m_processes.end())
  {
    m_processes.push_back(std::move(timeProcess));
  }
}

void time_interval::remove_time_process(time_process* timeProcess)
{
  auto it = find_if(m_processes, [=](const auto& other) {
    return other.get() == timeProcess;
  });
  if (it != m_processes.end())
  {
    m_processes.erase(it);
  }
}

void time_interval::cleanup()
{
  m_processes.clear();
}

void time_interval::mute(bool m)
{
  node->set_mute(m);
  for(auto& p : get_time_processes())
  {
    p->mute(m);
  }
}

void time_interval::set_tempo_curve(optional<tempo_curve> curve)
{
  if((m_hasTempo = bool(curve)))
    m_tempoCurve = *std::move(curve);
  else
    m_tempoCurve.reset();
}

void time_interval::set_time_signature_map(optional<time_signature_map> map)
{

}
}
