/*! \file gap.h
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#ifndef GAP_H_
#define GAP_H_

#include <boost/config.hpp>
#include <boost/utility.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

#include <set>
#include <vector>

typedef boost::property < boost::vertex_name_t, std::string > VertexName;
typedef boost::property < boost::vertex_index_t, size_t,
        VertexName > VertexIndex;
typedef VertexIndex VertexProperty;
typedef boost::property < boost::edge_weight_t, double >EdgeWeight;
typedef boost::property < boost::edge_name_t, std::string,
        EdgeWeight > EdgeName;
typedef boost::property < boost::edge_index_t, size_t, EdgeName > EdgeIndex;
typedef EdgeIndex EdgeProperty;

#if defined ADJACENCY_LIST
typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::undirectedS,
        VertexProperty, EdgeProperty > Graph;
#else /* if defined  ADJACENCY_MATRIX */
typedef boost::adjacency_matrix < boost::directedS, VertexProperty, EdgeProperty > GraphH;
typedef boost::adjacency_matrix < boost::undirectedS, VertexProperty, EdgeProperty > GraphC;
#endif
typedef boost::graph_traits < GraphH >::edge_descriptor EdgeDescriptorH;
typedef boost::graph_traits < GraphH >::vertex_descriptor VertexDescriptorH;
typedef boost::graph_traits < GraphC >::edge_descriptor EdgeDescriptorC;
typedef boost::graph_traits < GraphC >::vertex_descriptor VertexDescriptorC;

#endif /* GAP_H_ */
