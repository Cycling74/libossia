#pragma once
#include <string>
#include <regex>
#include <ossia/network/base/name_validation.hpp>

namespace ossia
{
namespace net
{
class node_base;
}
/**
 * \namespace regex_path
 * \brief Utilities to construct regexes to validate paths.
 *
 * Paths can be constructed like this :
 *
 * \code
 * {
 *   using namespace ossia::regex_path;
 *   auto path = device("foo") / "bar" / any_instance("baz");
 *   std::regex_match("foo:/bar/baz.2", path.regex()); // shall return true.
 * }
 * \endcode
 */
namespace regex_path
{
/**
 * @brief Base class for our paths
 */
struct OSSIA_EXPORT path_element
{
  std::string address;
  path_element(std::string s)
    : address{std::move(s)}
  {

  }

  template<int N>
  path_element(const char (&s)[N])
    : address(s, N - 1)
  {

  }

  friend std::ostream& operator<<(std::ostream& s, const path_element& p)
  {
    return s << p.address;
  }

  operator std::string() const { return address; }
  std::regex regex() const { return std::regex(address); }
};

//! Represents a device in a path, e.g. "foo:"
struct OSSIA_EXPORT device : public path_element
{
  explicit device(std::string s)
    : path_element{"^" + std::move(s) + ":"}
  {

  }
};

//! Can match nodes that are instances : foo:/bar, foo:/bar.1, etc.
struct OSSIA_EXPORT any_instance : public path_element
{
  explicit any_instance(std::string s)
    : path_element{std::move(s) + "(\\.[0-9]+?)?"}
  {

  }
};


//! Can match nodes from an alternative, foo:/bar/baz, foo:/bob/baz, and not foo:/bin/baz
struct OSSIA_EXPORT any_between : public path_element
{
  any_between(std::string s): path_element{s} { }

  any_between(std::initializer_list<std::string> args):
    path_element{""}
  {
    const int N = args.size();
    if(N > 0)
    {
      address += '(';

      auto it = args.begin();
      address += *it;

      for(int i = 1; i < N; i++)
      {
        ++it;
        address += '|';
        address += *it;
      }

      address += ')';
    }
  }
};


//! Can match any node : foo:/bar, foo:/baz.1234, etc.
struct OSSIA_EXPORT any_node
{
};

//! Can match any subpath : foo:/bar/baz, foo:/bar/bo.12/baz, etc.
struct OSSIA_EXPORT any_path
{
};

//! Matches the end of an address
struct OSSIA_EXPORT stop
{
};

inline path_element operator/(const path_element& lhs, const path_element& rhs)
{
  return path_element{lhs.address + "\\/" + rhs.address};
}

inline path_element operator/(const path_element& lhs, const any_instance& rhs)
{
  return path_element{lhs.address + "\\/" + rhs.address};
}

inline path_element operator/(const path_element& lhs, const any_node&)
{
  return path_element{lhs.address +
        "\\/[" + ossia::net::name_characters().to_string() + "]*?"};
}

inline path_element operator/(const path_element& lhs, const any_path&)
{
  return path_element{lhs.address +
        "(\\/[" + ossia::net::name_characters().to_string() + "]*?)+?"};
}

inline path_element operator/(const any_path&, const path_element& rhs)
{
  const std::string sub = ossia::net::name_characters().to_string();
  std::string sub2 = "^([";
  sub2 += sub;
  sub2 += "]*?:)(\\/?[";
  sub2 += sub;
  sub2 += "]*?)+?\\/";
  sub2 += rhs.address;
  return path_element{std::move(sub2)};
}

inline path_element operator/(const path_element& lhs, const stop& rhs)
{
  return path_element{lhs.address + "$"};
}

}


/**
 * \namespace traversal
 * \brief Utilities to construct classes that will perform an action for nodes matching a path.
 *
 * The path has an user-friendly, OSC 1.1-like format :
 * foo:/bar/baz*
 * ../boo/bin.*
 * //bin/bo??o/bee
 * buz:/{bee,boo}*
 *
 * We have a translation phase :
 * Let [:ossia:] be the regex character class defined by ossia::net::name_characters()
 * "?" -> [:ossia:]?
 * "*" -> [:ossia:]*
 * "//" -> any_path() /
 * ".." -> getParent()
 * All given paths are ended by "$"
 *
 * Given a path in the "user" format :
 * First try to find the largest absolute part from the beginning.
 * Then apply regexes to each sub-path and child node by splitting :
 *
 * foo:/bar/baz / b*anana.?? / *.*
 * // bonkers / *
 * ../ plop / foo.* / ..
 */
namespace traversal
{
// Give iterator interface, or return a vector on which we can iterate ?
// Handle relative paths : "../foo"

struct OSSIA_EXPORT path
{
  /** A list of function for the location of elements.
   * Each function will be called on the next step.
   */
  std::vector<std::function<void(std::vector<ossia::net::node_base*>&)>> functions;
};

OSSIA_EXPORT path make_path(const std::string& address);

OSSIA_EXPORT void apply(
    const path& p,
    std::vector<ossia::net::node_base*>& nodes);
}
}
