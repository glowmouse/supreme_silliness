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

  constexpr size_t value() const
  {
    return id;
  }

  private:
  size_t id = 0;
};

#endif

