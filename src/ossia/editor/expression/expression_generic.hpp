#pragma once
#include <ossia/editor/expression/expression_fwd.hpp>

#include <ossia/detail/config.hpp>

#include <memory>

namespace ossia
{
namespace expressions
{
//! This expression allows the users of the library to implement their own
//! behaviour.
struct OSSIA_EXPORT expression_generic_base
{
  virtual ~expression_generic_base();
  virtual void update() = 0;
  virtual bool evaluate() const = 0;
  virtual void on_first_callback_added(expression_generic&) = 0;
  virtual void on_removing_last_callback(expression_generic&) = 0;
};

class OSSIA_EXPORT expression_generic final
    : public expression_callback_container
{
public:
  expression_generic(std::unique_ptr<expression_generic_base> ptr)
      : expr{std::move(ptr)}
  {
  }
  expression_generic(expression_generic_base* ptr) : expr{ptr}
  {
  }
  ~expression_generic() override;
  std::unique_ptr<expression_generic_base> expr;

  void update() const
  {
    expr->update();
  }
  bool evaluate() const
  {
    return expr->evaluate();
  }

private:
  void on_first_callback_added() override
  {
    expr->on_first_callback_added(*this);
  }
  void on_removing_last_callback() override
  {
    expr->on_removing_last_callback(*this);
  }
};
}
}
