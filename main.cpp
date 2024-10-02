#include <string_view>
#include <iostream>
#include <array>
#include <tuple>
#include <type_traits>

#include "graph.h"
#include "text_parsing.h"



class edge_t {
  public: 

  constexpr edge_t() {}

  size_t dst_node = 0;
  size_t next_edge = -1;
};

template< size_t max_edges >
class edge_storage_t {
  public:

  constexpr edge_storage_t() : edge_memory{} {}

  constexpr size_t alloc_edge( size_t node, size_t next ) {
    auto candidate = next_available;
    next_available += 1;
    edge_memory.at( candidate ).dst_node = node;
    edge_memory.at( candidate ).next_edge = next;
    return candidate;
  }

  constexpr edge_t& get_edge( size_t index )
  {
    return edge_memory.at( index );
  }

  constexpr const edge_t& get_edge( size_t index ) const
  {
    return edge_memory.at( index );
  }

  private:
  std::array< edge_t, max_edges > edge_memory;
  size_t next_available = 0;
};

class node_t {
  public:

  constexpr node_t( int node_id_arg ) : node_id{ node_id_arg } {}
  constexpr node_t() : node_id{ -1 } {}

  constexpr int get_id() const { return node_id; }
  constexpr int get_edge_begin() const { return edge_begin_id; }
  constexpr void set_edge_begin( int edge_begin_id_arg ) { edge_begin_id = edge_begin_id_arg; }

  private:

  int node_id = -1;
  int edge_begin_id = -1;
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

  constexpr void add_edge( size_t src_node, size_t dst_node ) {
    node_t& node = nodes().at( src_node );
    auto old_first_edge = node.get_edge_begin();
    auto new_edge_idx = edges().alloc_edge( dst_node, old_first_edge );
    node.set_edge_begin( new_edge_idx );
  }

  constexpr size_t get_nodes() const {
    return used_nodes;
  }

  constexpr size_t first_edge( size_t node_idx )  const {
    return nodes().at( node_idx ).get_edge_begin();
  }

  constexpr size_t next_edge( size_t edge_idx ) const {
    return edges().get_edge( edge_idx ).next_edge;
  }

  constexpr size_t dst_node( size_t edge_idx )  const {
    return edges().get_edge( edge_idx ).dst_node;
  }

  constexpr size_t count_subgraphs() const {
    
    for( size_t node_idx = 0; node_idx < used_nodes; ++node_idx ) {
    
    }
    return 0;
  }

  void print() const {
    for( const auto& node : *this ) {
      std::cout << node.get_id() << " -> ";
      for ( size_t edge_idx = node.get_edge_begin(); edge_idx != -1; ) {
        const auto& edge = edges().get_edge( edge_idx );
        std::cout << edge.dst_node << " (" << edge_idx << ") ";
        edge_idx = edge.next_edge;
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
    graph.add_edge( src_node, dst_node );
  }

  return graph;
}

constexpr graph_t double_up_edges( const graph_t graph )
{
  const auto orig_nodes = graph.get_nodes();
  graph_t new_graph{ orig_nodes } ;

  for( size_t node_idx = 0; node_idx < orig_nodes; ++node_idx ) {
    for( size_t edge_idx = graph.first_edge(node_idx);
        edge_idx != -1; 
        edge_idx = graph.next_edge( edge_idx )) {
      auto dst_node = graph.dst_node( edge_idx );
      new_graph.add_edge( node_idx, dst_node );
      new_graph.add_edge( dst_node, node_idx );
    }
  }
  return new_graph;
}

constexpr void mark_connected( const graph_t& graph, size_t node_idx, std::array<int, max_graph_nodes> &visited, int color )
{
  visited.at( node_idx ) = color;

  for( size_t edge_idx = graph.first_edge(node_idx);
    edge_idx != -1; 
    edge_idx = graph.next_edge( edge_idx )) 
  {
    auto dst_node = graph.dst_node( edge_idx );
    if ( visited.at( dst_node ) == -1 ) {
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
  for ( size_t node_idx = 0; node_idx < graph.get_nodes(); ++node_idx ) {
    if ( visited.at( node_idx ) == -1 ) {
      mark_connected( graph, node_idx, visited, color );
      ++color;
    }
  }
  return color;
}

constexpr graph_t::storage_t storage;
constexpr graph_t graph = read_graph( graph_text );
constexpr graph_t bidir_graph = double_up_edges( graph );
static_assert( bidir_graph.get_nodes() == 10000 );
constexpr int connected_subgraphs = count_connected( bidir_graph );
static_assert( connected_subgraphs == 12 );

int main( int argc, const char *argv[] ) {
//  graph.print();
  std::cout << connected_subgraphs << "\n";
}

