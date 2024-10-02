#ifndef __NODE_ID_H__
#define __NODE_ID_H__

#include<optional>

/// @brief 
template < typename tag >
class numeric_id_t 
{
  public:

  constexpr explicit numeric_id_t( size_t arg_id ) noexcept : id{arg_id} {}
  constexpr numeric_id_t() noexcept = default;

  constexpr bool has_value() const noexcept
  {
    return id.has_value();
  }
  constexpr size_t value() const
  {
    return id.value();
  }

  private:
  std::optional<size_t> id;
};

#endif

