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

/*
    class implementing a graph
*/
class Graph{

    /*
        stores the Nodes
    */
    Node_Ptr_Vec* node_vec;

    /*
        stores the Relationships
    */
    Rel_Ptr_Vec* rel_vec;

    /*
        name of the memory mapped file
    */
    std::string name_file;

    /*
        managed file object to create and delete components like Nodes etc.
    */
    bi::managed_mapped_file segment;

    sharable_mutex_type write_nodes;
    sharable_mutex_type write_rel;

    public:

        Graph(std::string name, size_t size);
        //Graph(graph_db_ptr& graph, std::function<void()> f_nodes, std::function<std::vector<node::id_t>(std::vector<std::unique_ptr<Node>>::iterator)> f_rels, size_t thread_count = std::thread::hardware_concurrency());
        ~Graph();
        void initialise_relationships(graph_db_ptr& graph, std::function<std::vector<node::id_t>(boost::interprocess::offset_ptr<Node>)> f_rels, size_t thread_count = std::thread::hardware_concurrency());

        /*
            adds the Node with the given id
        */
        bi::offset_ptr<Node> add_node(node::id_t input);

        /*
            adds the Relationship between the given nodes.
        */
        bi::offset_ptr<Relationship> add_relationship(size_t id, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  from_node, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  to_node);

        /*
            adds the Relationship between the given nodes.
        */
        bi::offset_ptr<Relationship> add_relationship(size_t id, bi::offset_ptr<Node> from_node, bi::offset_ptr<Node> to_node);

        /*
            deletes the given node. Note that all Relationships leaving and pointing to this node must be deleted before
        */
        void delete_node(boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false> it);

        /*
            deletes the given Relationship
        */
        void delete_rel(boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> it);

        /*
            get an iterator to the first node
        */
        boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  get_node_iterator_begin();

        /*
            get an iterator to one element behind the last node
        */
        boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  get_node_iterator_end();

        /*
            get an iterator to the first relationship
        */
        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_rel_iterator_begin();

        /*
            get an iterator to one element behind the last relationship
        */
        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_rel_iterator_end();
};

#endif