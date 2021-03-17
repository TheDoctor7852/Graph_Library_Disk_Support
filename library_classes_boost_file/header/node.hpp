#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

#include <vector>
#include <map>

#include "nodes.hpp"

namespace bi = boost::interprocess;

class Relationship;
class Graph;
//typedef bi::managed_unique_ptr<Node, bi::managed_mapped_file>::type unique_pointer_type;

typedef bi::allocator<bi::offset_ptr<Relationship>, bi::managed_mapped_file::segment_manager> Rel_Pointer_Allocator;
typedef bi::vector<bi::offset_ptr<Relationship>, Rel_Pointer_Allocator> Rel_Pointer_Vec;

typedef bi::allocator<std::pair<const std::string, boost::any>, bi::managed_mapped_file::segment_manager> PropMapAllocator;
typedef bi::flat_map<std::string, boost::any, std::less<std::string>, PropMapAllocator> Property_Map;

typedef bi::interprocess_sharable_mutex sharable_mutex_type;


/*
    Idee: wenn man schneller Knoten löschen oder umhängen möchte, dann wie folgt ändern: 
            - statt Relationship* wird size_t gespeichert. Dies ist die Position der Relationship im Vektor. 
            - damit das funktioniert, dürfen gelöschte Relationen nicht direkt gelöscht werden sondern mit nullptr ersetzen. 
              zusätzlich speichere die ids in priortity queue um diese dann beim einfügen wieder neu zu vergeben.
            ---> man kann keine direkten Pointer auf relationships mehr benutzen, dafür muss man um einen Knoten zu löschen oder zu ersetzen nun nicht mehr alle Relationships durchgehen sondern nur noch an die positionen schauen.
                Nachteil: der vector kann nicht mehr verkleinert werden 
*/

/*
  Bemerkung: wenn man boost::any benutzt, dann kann die map nicht wieder geladen werden. Somit muss man die Datei nach jeder Benutzung löschen.
*/

#ifndef NODE_HPP
#define NODE_HPP

/*
  class implementing Nodes of a Graph
*/
class Node{
  private:
        node::id_t id;
        sharable_mutex_type write_property;
        sharable_mutex_type write_inc;
        sharable_mutex_type write_out;

        /*
          keep track of Realtionships pointing to this Node
        */
        bi::offset_ptr<Rel_Pointer_Vec> incomming_rel;

        /*
          keep track of Realtionships leaving this Node
        */
        bi::offset_ptr<Rel_Pointer_Vec> outgoing_rel;

        /*
          maintain Propertys added to this Node
        */
        bi::offset_ptr<Property_Map> propertys;

        friend Relationship;
        friend Graph;

        /*
          adds an Relationship to the incomming Relationships vector.
        */
        void add_incomming_rel(bi::offset_ptr<Relationship> input);

        /*
          removes an Relationship from the incomming Relationships vector.
        */
        void remove_incomming_rel(bi::offset_ptr<Relationship> input);

        /*
          adds an Relationship to the outgoing Relationships Vector.
        */
        void add_outgoing_rel(bi::offset_ptr<Relationship> input);

        /*
          removes an Relationship from the outgoing Relationships Vector.
        */
        void remove_outgoing_rel(bi::offset_ptr<Relationship> input);


    public:

        Node(node::id_t input, bi::managed_mapped_file& segment);
        Node(Node&& n);

        /*
          add an property with any value to the Node. The given string identfies the added Property. If the key already exists, nothing will happen.
        */
        void add_property(std::string key, boost::any value);

        /*
          removes the property identified by the given string. 
        */
        void remove_property(std::string key);

        /*
          changes the property identified by the given string. A function is required if for example an vector was stored and needs to be modified without overwriting. 
        */
        bool change_property(std::string key, std::function<void(boost::any&)> f);

        /*
          read the property identified by the given string. If no value identified by the given string exists an exception will be thrown.
        */
        const boost::any read_property(std::string key);
        
        /*
          get the vector keeping track of Realtionships pointing to this Node
        */
        const bi::offset_ptr<Rel_Pointer_Vec> get_incomming_rel();

        /*
          get the vector keeping track of Realtionships leaving this Node
        */
        const bi::offset_ptr<Rel_Pointer_Vec> get_outgoing_rel();

        /*
          get the Node id
        */
        node::id_t get_id();

        Node& operator=(const Node& n);
};

#endif