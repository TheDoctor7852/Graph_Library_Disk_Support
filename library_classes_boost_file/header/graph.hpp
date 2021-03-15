#include "./node.hpp"
#include "./relationship.hpp"

#include "graph_pool.hpp"

#include <boost/interprocess/smart_ptr/unique_ptr.hpp>

#ifndef GRAPH_HPP
#define GRAPH_HPP

typedef bi::managed_unique_ptr<Node, bi::managed_mapped_file>::type node_unique_ptr_type;
typedef bi::allocator<node_unique_ptr_type, bi::managed_mapped_file::segment_manager> Node_Ptr_Allocator;
typedef bi::vector<node_unique_ptr_type, Node_Ptr_Allocator> Node_Ptr_Vec;

typedef bi::managed_unique_ptr<Relationship, bi::managed_mapped_file>::type rel_unique_ptr_type;
typedef bi::allocator<rel_unique_ptr_type, bi::managed_mapped_file::segment_manager> Rel_Ptr_Allocator;
typedef bi::vector<rel_unique_ptr_type, Rel_Ptr_Allocator> Rel_Ptr_Vec;

class Graph{
    Node_Ptr_Vec* node_vec;
    Rel_Ptr_Vec* rel_vec;

    std::string name_file;
    bi::managed_mapped_file segment;

    sharable_mutex_type write_nodes;
    sharable_mutex_type write_rel;

    public:
        Graph(std::string name, size_t size);
        //Graph(graph_db_ptr& graph, std::function<void()> f_nodes, std::function<std::vector<node::id_t>(std::vector<std::unique_ptr<Node>>::iterator)> f_rels, size_t thread_count = std::thread::hardware_concurrency());
        ~Graph();
        void initialise_relationships(graph_db_ptr& graph, std::function<std::vector<node::id_t>(boost::interprocess::offset_ptr<Node>)> f_rels, size_t thread_count = std::thread::hardware_concurrency());
        bi::offset_ptr<Node> add_node(node::id_t input);
        bi::offset_ptr<Relationship> add_relationship(size_t id, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  from_node, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  to_node);
        bi::offset_ptr<Relationship> add_relationship(size_t id, bi::offset_ptr<Node> from_node, bi::offset_ptr<Node> to_node);
        void delete_node(boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false> it); //um einen Knoten zu löschen, darf keine Kante mehr auf ihn zeigen oder von ihm weggehen
        void delete_rel(boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> it);

        boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  get_node_iterator_begin();
        boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  get_node_iterator_end();

        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_rel_iterator_begin();
        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_rel_iterator_end();
};

#endif