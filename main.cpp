#include <string_view>
#include <iostream>
#include <array>
#include <tuple>
#include <type_traits>

#include "graph.h"
#include "text_parsing.h"
#include "numeric_id.h"

// Dummy tags for the node_t and edge_t
struct node_id_tag_t {};
struct edge_id_tag_t {};

/// @brief Numeric (size_t) id for a graph node.
using node_id_t = numeric_id_t< node_id_tag_t >;

/// @brief Numeric (size_t) id for a graph edge
using edge_id_t = numeric_id_t< edge_id_tag_t >;

/// @brief optional graph_id_d
/// 
/// The edge fanout for a graph node is represented by a linked list -
/// optional_edge_id_t is how the next field in the linked list is stored.
///
/// If a method takes or returns edge_id_t, a legal edge is gauranteed.
/// If a method takes optional_edge_id_t, the edge may not exist
///  
using optional_edge_id_t = std::optional<numeric_id_t< edge_id_tag_t >>;

///
/// @brief Graph edge class
/// 
/// The edge fanout of any graph node is represented as a linked list.
/// The edge_t class are the nodes on that list.
/// 
class edge_t {
  public: 

  ///
  /// @brief Edge constructor 
  ///
  /// Creates a node in the edge fanout linked list.
  /// 
  /// arg_dst_node - The destination node of the edge.  The source node
  ///   is the node that owns the start of the list.
  /// arg_next_edge - The next edge in the edge fanout linked list.  If the
  ///   optional has no value then we're at the end of the list.
  /// 
  constexpr edge_t(
    node_id_t arg_dst_node, 
    optional_edge_id_t arg_next_edge
  ) : dst_node{arg_dst_node}, next_edge{arg_next_edge} {}

  /// @brief Get the next edge in the edge fanout linked list
  ///
  constexpr optional_edge_id_t get_next_edge() const {
    return next_edge;
  }

  /// @brief Get the destination node for this edge.
  /// 
  constexpr node_id_t get_dst_node() const {
    return dst_node;
  }

  /// @brief default constructor for un-initialized edges
  ///
  /// Used to create the edge allocator class, edge_storage_t
  ///
  constexpr edge_t() = default;

  private:
  node_id_t dst_node;
  optional_edge_id_t next_edge;
};

/// @brief A pool of graph edges (edge_t class) that can be allocated from
/// 
template< size_t max_edges >
class edge_storage_t {
  public:

  constexpr edge_storage_t() = default;

  /// @brief Allocate and initialize an edge
  ///
  /// @param dst_node - The destination node ID the edge is connecting to
  /// @param next_edge - The node edge in the source node's edge fanout linked list
  ///
  /// @return The newly allocated edge identifier
  /// 
  constexpr edge_id_t alloc_edge( 
    node_id_t dst_node, 
    optional_edge_id_t next_edge 
  ) {
    auto candidate = next_available;
    next_available += 1;
    edge_memory.at( candidate ) = edge_t(dst_node, next_edge );
    return edge_id_t{candidate};
  }

  /// @brief Get a reference to the actual edge data given the edge's identifier
  ///
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

  /// @brief Node constructor
  ///
  /// @brief node_id_arg - The unque ID of the node
  ///
  constexpr node_t( node_id_t node_id_arg ) : node_id{ node_id_arg } {}

  /// @brief get the node ID
  constexpr node_id_t get_id() const { 
    return node_id; 
  }

  /// @brief Gets the beginning of the edge fanout list
  constexpr optional_edge_id_t get_edge_head() const { 
    return edge_head; 
  }

  /// @brief Add a new edge to the node.
  ///
  /// dst_node  Destination node.  Creates a node_id -> dst_node edge
  /// storage   Storage pool to get the new edge from
  ///
  template< size_t storage_max_edges >
  constexpr void add_edge( node_id_t dst_node, edge_storage_t<storage_max_edges>& storage)
  {
    edge_id_t new_head = storage.alloc_edge( dst_node, edge_head );
    edge_head = optional_edge_id_t{new_head};
  } 

  /// @brief default constructor for un-initialized nodes
  ///
  /// Used to create the edge allocator class, edge_storage_t
  ///
  constexpr node_t() = default;

  private:

  node_id_t node_id;
  optional_edge_id_t edge_head;
};

/// 
/// @brief Graph with variable storage
///
///
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

  /// Constructs a graph with "used_nodes_arg" nodes and no edges.
  ///
  /// @param used_nodes_arg - Number of nodes in the graph.
  ///
  constexpr graph_raw( size_t used_nodes_arg ) : storage{}, used_nodes{ used_nodes_arg } {
    // Initialized each used node with a unique id
    size_t idx = 0;
    for( auto& node: *this ) {
      node = node_t( node_id_t{idx} );
      ++idx;
    }
  }

  /// @brief Add an edge to the graph
  ///
  /// src_node - edge source node
  /// dst_node - edge destination node
  ///
  constexpr void add_edge( node_id_t src_node, node_id_t dst_node ) {
    node_t& node = nodes().at( src_node.value() );
    node.add_edge( dst_node, edges() );
  }

  /// @brief Gets the number of nodes in the graph
  constexpr size_t get_num_nodes() const {
    return used_nodes;
  }

  /// @brief Gets the head of the edge linked list.
  ///
  /// Note - most of the time this is the only thing we want from a node.  We
  ///    already have the node id
  ///
  constexpr optional_edge_id_t edge_head( node_id_t node_idx )  const {
    return nodes().at( node_idx.value() ).get_edge_head();
  }

  /// @brief Get an edge given an edge_id
  constexpr const edge_t& get_edge( edge_id_t edge_idx ) const {
    return edges().get_edge( edge_idx );
  }

  /// @brief Print the graph by walking nodes and edges.
  ///
  void print() const {
    for( const auto& node : *this ) {
      std::cout << node.get_id().value() << " -> ";
      for ( auto edge_idx = node.get_edge_head(); edge_idx.has_value(); ) {
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
    auto edge_itr = graph.edge_head( node_idx );

    while( edge_itr.has_value() ) {
      const edge_t& edge = graph.get_edge( edge_itr.value() );

      auto dst_node = edge.get_dst_node();
      new_graph.add_edge( node_idx, dst_node );
      new_graph.add_edge( dst_node, node_idx );

      edge_itr = edge.get_next_edge();
    }
  }
  return new_graph;
}

constexpr void mark_connected( const graph_t& graph, node_id_t node_idx, std::array<int, max_graph_nodes> &visited, int color )
{
  visited.at( node_idx.value() ) = color;

  auto edge_itr = graph.edge_head( node_idx );

  while( edge_itr.has_value() ) {
    const edge_t& edge = graph.get_edge( edge_itr.value() );
    auto dst_node = edge.get_dst_node();
    if ( visited.at( dst_node.value() ) == -1 ) {
      mark_connected( graph, dst_node, visited, color );
    }
    edge_itr = edge.get_next_edge();
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
  graph.print();
  std::cout << connected_subgraphs << "\n";
}

