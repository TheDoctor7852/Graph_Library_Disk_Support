#include "relationships.hpp"
#include "./node.hpp"

#ifndef REL_HPP
#define REL_HPP
class Relationship{
        relationship::id_t id;
        sharable_mutex_type write;

        bi::offset_ptr<Node> from;
        bi::offset_ptr<Node> to;

        Property_Map* propertys;
    public:
        Relationship(relationship::id_t input, bi::offset_ptr<Node> from_node, bi::offset_ptr<Node> to_node, bi::managed_mapped_file& segment);
        //Relationship(Relationship&& r);
        Relationship(const Relationship& r);
        //~Relationship() = default;

        void add_property(std::string key, boost::any value);
        void remove_property(std::string key);
        bool change_property(std::string key, std::function<void(boost::any&)> f);
        const boost::any read_property(std::string key);

        relationship::id_t get_id();
        bi::offset_ptr<Node> get_from_node();
        bi::offset_ptr<Node> get_to_node();

        Relationship& operator=(const Relationship& r);
};

#endif