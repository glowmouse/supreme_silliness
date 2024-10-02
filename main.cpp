#include <string_view>
#include <iostream>
#include <array>
#include <tuple>
#include <type_traits>

#include "graph.h"
#include "text_parsing.h"
#include "numeric_id.h"

struct node_id_tag_t {};
struct edge_id_tag_t {};

using node_id_t = numeric_id_t< node_id_tag_t >; 
using edge_id_t = numeric_id_t< edge_id_tag_t >;
using optional_edge_id_t = std::optional<numeric_id_t< edge_id_tag_t >>;

class edge_t {
  public: 

  constexpr edge_t() {}
  constexpr edge_t(node_id_t arg_dst_node, optional_edge_id_t arg_next_edge) : dst_node{arg_dst_node}, next_edge{arg_next_edge} {}

  constexpr optional_edge_id_t get_next_edge() const {
    return next_edge;
  }

  constexpr node_id_t get_dst_node() const {
    return dst_node;
  }

  private:
  node_id_t dst_node;
  optional_edge_id_t next_edge;
};

template< size_t max_edges >
class edge_storage_t {
  public:

  constexpr edge_storage_t() : edge_memory{} {}

  constexpr edge_id_t alloc_edge( node_id_t node, optional_edge_id_t next ) {
    auto candidate = next_available;
    next_available += 1;
    edge_memory.at( candidate ) = edge_t(node, next );
    return edge_id_t{candidate};
  }

  constexpr const edge_t& get_edge( edge_id_t index ) const
  {
    return edge_memory.at( index.value() );
  }

  private:
  std::array< edge_t, max_edges > edge_memory;
  size_t next_available = 0;
};

class node_t {
  public:

  constexpr node_t( size_t node_id_arg ) : node_id{ node_id_arg } {}
  constexpr node_t() = default;

  constexpr node_id_t get_id() const { return node_id; }
  constexpr optional_edge_id_t get_edge_begin() const { return edge_begin_id; }
  constexpr void set_edge_begin( optional_edge_id_t edge_begin_id_arg ) { edge_begin_id = edge_begin_id_arg; }

  private:

  node_id_t node_id;
  optional_edge_id_t edge_begin_id;
};

template< size_t max_nodes, size_t max_edges >
class graph_raw {
  public:

  using node_array_t = std::array<node_t, max_nodes >;
  using edge_pool_t = edge_storage_t< max_edges >;
  using storage_t = std::tuple< node_array_t, edge_pool_t >;

  // Standard container like C++ Interfaces
  using value_type       = typename node_array_t::value_type;
  using size_type        = typename node_array_t::size_type;
  using reference        = typename node_array_t::reference;
  using const_reference  = typename node_array_t::const_reference;
  using iterator         = typename node_array_t::iterator;
  using const_iterator   = typename node_array_t::const_iterator;
  
  // Support iterators
  constexpr iterator begin()              noexcept { return nodes().begin(); }  
  constexpr const_iterator begin()  const noexcept { return nodes().begin(); }  
  constexpr const_iterator cbegin() const noexcept { return nodes().cbegin(); }  
  constexpr iterator end()                noexcept { return begin() + used_nodes; }
  constexpr const_iterator end()    const noexcept { return begin() + used_nodes; }
  constexpr const_iterator cend()   const noexcept { return cbegin() + used_nodes; }

  graph_raw() = delete;

  constexpr graph_raw( size_t used_nodes_arg ) : storage{}, used_nodes{ used_nodes_arg } {
    int idx = 0;
    for( auto& node: *this ) {
      node = node_t(idx);
      ++idx;
    }
  }

  constexpr void add_edge( node_id_t src_node, node_id_t dst_node ) {
    node_t& node = nodes().at( src_node.value() );
    auto old_first_edge = node.get_edge_begin();
    auto new_edge_idx = edges().alloc_edge( dst_node, old_first_edge );
    node.set_edge_begin( new_edge_idx );
  }

  constexpr size_t get_num_nodes() const {
    return used_nodes;
  }

  constexpr optional_edge_id_t first_edge( node_id_t node_idx )  const {
    return nodes().at( node_idx.value() ).get_edge_begin();
  }

  constexpr optional_edge_id_t next_edge( edge_id_t edge_idx ) const {
    return edges().get_edge( edge_idx ).get_next_edge();
  }

  constexpr node_id_t dst_node( edge_id_t edge_idx )  const {
    return edges().get_edge( edge_idx ).get_dst_node();
  }

  void print() const {
    for( const auto& node : *this ) {
      std::cout << node.get_id().value() << " -> ";
      for ( auto edge_idx = node.get_edge_begin(); edge_idx.has_value(); ) {
        const auto& edge = edges().get_edge( edge_idx.value() );
        std::cout << edge.get_dst_node().value() << " (" << edge_idx.value().value() << ") ";
        edge_idx = edge.get_next_edge();
      }
      std::cout << "\n";
    }
  }

  private:

  constexpr node_array_t& nodes() { return std::get<0>(storage); }  
  constexpr edge_pool_t& edges() { return std::get<1>(storage); }  
  constexpr const node_array_t& nodes() const { return std::get<0>(storage); }  
  constexpr const edge_pool_t& edges() const { return std::get<1>(storage); }  

  const size_t used_nodes;
  storage_t storage;
};

constexpr std::string_view graph_text{ input };

constexpr size_t max_graph_nodes = read_int_v( graph_text );  
constexpr size_t max_graph_edges = count_words( graph_text );

using graph_t = graph_raw< max_graph_nodes, max_graph_edges >;

constexpr graph_t read_graph( std::string_view text ) 
{
  auto num_nodes = read_int( text ); 
  graph_t graph{ num_nodes };

  while( text.size() > 0 ) {
    auto src_node = read_int( text ); 
    auto dst_node = read_int( text ); 
    graph.add_edge( node_id_t{src_node}, node_id_t{dst_node} );
  }

  return graph;
}

constexpr graph_t double_up_edges( const graph_t graph )
{
  const auto orig_nodes = graph.get_num_nodes();
  graph_t new_graph{ orig_nodes } ;

  for( size_t i= 0; i < orig_nodes; ++i ) {
    const auto node_idx = node_id_t{ i };
    for( auto edge_idx = graph.first_edge(node_idx);
        edge_idx.has_value();
        edge_idx = graph.next_edge( edge_idx.value() )) {
      auto dst_node = graph.dst_node( edge_idx.value() );
      new_graph.add_edge( node_idx, dst_node );
      new_graph.add_edge( dst_node, node_idx );
    }
  }
  return new_graph;
}

constexpr void mark_connected( const graph_t& graph, node_id_t node_idx, std::array<int, max_graph_nodes> &visited, int color )
{
  visited.at( node_idx.value() ) = color;

  for( auto edge_idx = graph.first_edge(node_idx);
    edge_idx.has_value(); 
    edge_idx = graph.next_edge( edge_idx.value() )) 
  {
    auto dst_node = graph.dst_node( edge_idx.value() );
    if ( visited.at( dst_node.value() ) == -1 ) {
      mark_connected( graph, dst_node, visited, color );
    }
  }
}

constexpr int count_connected( const graph_t& graph )
{
  std::array< int, max_graph_nodes> visited = {};
  for ( auto itr = visited.begin(); itr != visited.end(); ++itr ) {
    *itr = -1;
  }

  int color = 0;
  for ( size_t i = 0; i < graph.get_num_nodes(); ++i ) {
    const auto node_idx = node_id_t{i};
    if ( visited.at( node_idx.value() ) == -1 ) {
      mark_connected( graph, node_idx, visited, color );
      ++color;
    }
  }
  return color;
}

constexpr graph_t::storage_t storage;
constexpr graph_t graph = read_graph( graph_text );
constexpr graph_t bidir_graph = double_up_edges( graph );
static_assert( bidir_graph.get_num_nodes() == 10000 );
constexpr int connected_subgraphs = count_connected( bidir_graph );
static_assert( connected_subgraphs == 12 );

int main( int argc, const char *argv[] ) {
//  graph.print();
  std::cout << connected_subgraphs << "\n";
}

