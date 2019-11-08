#include <ossia/editor/scenario/scenario.hpp>
#include <ossia/editor/scenario/time_sync.hpp>
#include <ossia/editor/scenario/time_event.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <ossia/editor/scenario/quantification.hpp>
#include <ossia/detail/algorithms.hpp>
#include <iostream>

namespace ossia
{
optional<time_value> get_quantification_date(const ossia::token_request& tk, double rate)
{
  optional<time_value> quantification_date{};
  if(rate <= 1.)
  {
    // Quantize relative to bars
    if(tk.musical_end_last_bar != tk.musical_start_last_bar)
    {
      // There is a bar change in this tick
      double musical_tick_duration = tk.musical_end_position - tk.musical_start_position;
      double musical_bar_start = tk.musical_end_last_bar - tk.musical_start_position;

      double ratio = musical_bar_start / musical_tick_duration;
      time_value dt = tk.date - tk.prev_date; // TODO should be tick_offset

      quantification_date = tk.prev_date + dt * ratio;
    }
  }
  else
  {
    // Quantize relative to quarter divisions
    // TODO ! if there is a bar change,
    // and no prior quantization date before that, we have to quantize to the bar change

    double start_quarter = (tk.musical_start_position - tk.musical_start_last_bar);
    double end_quarter = (tk.musical_end_position - tk.musical_start_last_bar);

    // rate = 2 -> half
    // rate = 4 -> quarter
    // rate = 8 -> 8th..

    // duration of what we quantify in terms of quarters
    double musical_quant_dur = rate / 4.;
    double start_quant = std::floor(start_quarter * musical_quant_dur);
    double end_quant = std::floor(end_quarter * musical_quant_dur);

    if(start_quant != end_quant)
    {
      // Date to quantify is the next one :
      double musical_tick_duration = tk.musical_end_position - tk.musical_start_position;
      double quantified_duration = (tk.musical_start_last_bar + (start_quant + 1) * 4. / rate) - tk.musical_start_position;
      double ratio = (tk.date - tk.prev_date) / musical_tick_duration;

      quantification_date = tk.prev_date + quantified_duration * ratio;
    }
  }
  return quantification_date;
}

sync_status scenario::trigger_sync_musical(
    time_sync& sync, small_event_vec& maxReachedEvents,
    ossia::time_value tick_offset,
    const ossia::token_request& tk, bool maximalDurationReached) noexcept
{
  if (!sync.m_evaluating)
  {
    sync.m_evaluating = true;
    sync.trigger_request = false;
    sync.entered_evaluation.send();
  }

  if (*sync.m_expression != expressions::expression_true()
      && !maximalDurationReached)
  {
    if (!sync.has_trigger_date() && !sync.m_is_being_triggered)
    {
      if (!sync.is_observing_expression())
        expressions::update(*sync.m_expression);

      sync.observe_expression(true);

      if (sync.trigger_request)
        sync.trigger_request = false;
      else if (!expressions::evaluate(*sync.m_expression))
        return sync_status::NOT_READY;
    }

    sync.m_is_being_triggered = true;

    if (!sync.has_trigger_date() && sync.has_sync_rate())
    {
      return quantify_time_sync(sync, tk);
    }
  }
  return sync_status::NOT_READY;
}

int is_timesync_ready(time_sync& sync, small_event_vec& pendingEvents, bool& maximalDurationReached)
{
  pendingEvents.clear();
  auto activeCount = sync.get_time_events().size();
  std::size_t pendingCount = 0;

  auto on_pending = [&](ossia::time_event* timeEvent) {
    if (!ossia::contains(pendingEvents, timeEvent))
    {
      pendingCount++;
      pendingEvents.push_back(timeEvent);
    }

    for (const std::shared_ptr<ossia::time_interval>& timeInterval :
         timeEvent->previous_time_intervals())
    {
      if (timeInterval->get_date() >= timeInterval->get_max_duration())
      {
        maximalDurationReached = true;
        break;
      }
    }
  };

  for (const std::shared_ptr<time_event>& timeEvent : sync.get_time_events())
  {
    switch (timeEvent->get_status())
    {
      // check if NONE TimeEvent is ready to become PENDING
      case time_event::status::NONE:
      {
        bool minimalDurationReached = true;

        for (const std::shared_ptr<ossia::time_interval>& timeInterval :
             timeEvent->previous_time_intervals())
        {
          const auto& ev = timeInterval->get_start_event();
          // previous TimeIntervals with a DISPOSED start event are ignored
          if (ev.get_status() == time_event::status::DISPOSED)
          {
            continue;
          }

          // previous TimeInterval with a none HAPPENED start event
          // can't have reached its minimal duration
          if (ev.get_status() != time_event::status::HAPPENED)
          {
            minimalDurationReached = false;
            break;
          }

          // previous TimeInterval which doesn't reached its minimal duration
          // force to quit
          if (timeInterval->get_date() < timeInterval->get_min_duration())
          {
            minimalDurationReached = false;
            break;
          }
        }

        // access to PENDING status once all previous TimeIntervals allow it
        if (minimalDurationReached)
        {
          timeEvent->set_status(time_event::status::PENDING);
          on_pending(timeEvent.get());
        }
        break;
      }

      // PENDING TimeEvent is ready for evaluation
      case time_event::status::PENDING:
        on_pending(timeEvent.get());
        break;

      case time_event::status::HAPPENED:
        break;

      case time_event::status::DISPOSED:
        activeCount--;
        break;
    }
  }

  return pendingCount == activeCount;
}

sync_status scenario::process_this_musical(
    time_sync& sync, small_event_vec& pendingEvents, small_event_vec& maxReachedEvents,
    ossia::time_value tick_offset,
    const ossia::token_request& req) noexcept
{
  // prepare to remember which event changed its status to PENDING
  // because it is needed in time_sync::trigger

  bool maximalDurationReached = false;
  // if all TimeEvents are not PENDING
  if (!is_timesync_ready(sync, pendingEvents, maximalDurationReached))
  {
    if (sync.m_evaluating)
    {
      sync.m_evaluating = false;
      sync.trigger_request = false;
      sync.left_evaluation.send();
    }

    return sync_status::NOT_READY;
  }

  return trigger_sync_musical(sync, maxReachedEvents, tick_offset, req, maximalDurationReached);
}
}
