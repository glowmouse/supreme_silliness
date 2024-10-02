#ifndef __TEXT_PARSING_H__
#define __TEXT_PARSING_H__

#include <string_view>
#include <tuple>

/// @brief A set of utilities for parsing constant strings.  Everything
///        here is constexpr so it can be used to help create compile time
///        constants.

/// @brief Test Utility.  Returns true if func(input) == expected
///
template< typename F >
constexpr bool test_read( std::string_view input, std::string_view expected, F func )
{
  return func( input ) == expected;
}

/// @brief Test Utility.  Returns true if input == expected after func(input)
///
template< typename F >
constexpr bool test_rest( std::string_view input, std::string_view expected, F func )
{
  func( input );
  return input  == expected;
}

/// @brief Two string views in a pair 
using view_view_pair_t   = std::tuple< std::string_view, std::string_view >;

///
/// @brief Return front characters of a string_view based on some generic criteria
///
/// @param text   The text we're parsing.  After the function call this will
///               be the remainder of the string_view.
/// @param func   Function that contains the criteria we're testing for.
/// @return       A token where all characters match the critera in func
///
template< typename F>
constexpr std::string_view read_generic( std::string_view& text, F func )
{
  // Index is where we'll split the string
  std::string_view::size_type index = 0;

  // Find the first character that fails func()
  for( auto c: text) {
    if ( !func(c)) { break; }
    ++index;
  }

  // Compute string_view to index for return
  std::string_view token = text;
  token.remove_suffix( text.length() - index );

  // set new input to remainder of the string
  text.remove_prefix( index );

  return token;
}

///
/// @brief Return characters at the front of a string_view that are not whitespace.
///
/// @param text   The text we're parsing.  On return contains the remainder of the string
/// @return       Any characters at the front of input that are not whitspace
///
constexpr std::string_view read_non_whitespace( std::string_view& input)
{
  return read_generic( input, []( char c ) { return !(c == ' ' || c == '\n'); } );
}

static_assert( test_read(" this is a test", "",           [](auto& in) {return read_non_whitespace(in); }));
static_assert( test_read("this is a test",  "this",       [](auto& in) {return read_non_whitespace(in); }));
static_assert( test_rest("this is a test",  " is a test", [](auto& in) {return read_non_whitespace(in); }));
static_assert( test_read("",                "",           [](auto& in) {return read_non_whitespace(in); }));
static_assert( test_rest("",                "",           [](auto& in) {return read_non_whitespace(in); }));

///
/// @brief Return characters at the front of a string_view that are whitespace.
///
/// @param text   The text we're parsing.  On return contains the remainder of the string
/// @return       Any characters at the front of input that are whitspace
///
constexpr std::string_view read_whitespace( std::string_view& input)
{
  return read_generic( input, []( char c ) { return (c == ' ' || c == '\n'); } );
}

static_assert( test_read("is a test",   "",          [](auto& in) {return read_whitespace(in); }));
static_assert( test_read("  is a test", "  ",        [](auto& in) {return read_whitespace(in); }));
static_assert( test_rest("  is a test", "is a test", [](auto& in) {return read_whitespace(in); }));
static_assert( test_read("",            "",          [](auto& in) {return read_whitespace(in); }));
static_assert( test_rest("",            "",          [](auto& in) {return read_whitespace(in); }));

///
/// @brief Convert a string_view to an integer.
///
/// @param input  The string being converted.  Must be a valid integer.
/// @return       The integer value of input.
///
constexpr auto view_to_int( const std::string_view& input )
{
  size_t rval = 0;

  for ( auto c : input ) {
    rval = rval * 10 + c - '0';
  }

  return rval;
}

static_assert( view_to_int( std::string_view{"1234"} ) == 1234 );
static_assert( view_to_int( std::string_view{""} )     == 0 );

///
/// @brief Read an integer from input and advance to next non-whitespace token
///
/// @param text   The text we're parsing.  On return contains the remainder of the string
/// @return       the integer
///
/// Results are undefined if the front if input is not an integer.
/// 
constexpr size_t read_int( std::string_view& input )
{
  auto token = read_non_whitespace( input );
  read_whitespace( input );
  return view_to_int( token );
}

static_assert( []() { 
  std::string_view test("42 43 44");
  return read_int( test ) == 42; } () );
static_assert( []() { 
  std::string_view test("42 43 44");
  read_int( test );
  return test == "43 44"; } () );


// Wrapper for read_int.  Does not modify input string view
constexpr size_t read_int_v( std::string_view input )
{
  auto token = read_non_whitespace( input );
  read_whitespace( input );
  return view_to_int( token );
}

static_assert( read_int_v( "42 43 44" ) == 42 );

constexpr int count_words( std::string_view input )
{
  bool currently_in_a_word = false;
  int word_count = 0;

  for ( auto c : input ) {
    const bool c_is_whitespace = ( c == ' ' || c == '\n' );
    if( !c_is_whitespace && !currently_in_a_word ) {
      ++word_count;
    }
    currently_in_a_word = !c_is_whitespace;
  }
  return word_count;
}

static_assert( count_words( "this is a test" ) == 4 );

#endif

