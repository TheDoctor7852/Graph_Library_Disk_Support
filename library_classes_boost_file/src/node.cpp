#include "../header/node.hpp"

Node::Node(node::id_t input, bi::managed_mapped_file& segment){
    id = input;
    const Rel_Pointer_Allocator incomming_rel_alloc(segment.get_segment_manager());
    const Rel_Pointer_Allocator outgoing_rel_alloc(segment.get_segment_manager());
    const PropMapAllocator property_alloc(segment.get_segment_manager());

    incomming_rel = segment.find_or_construct<Rel_Pointer_Vec>(("incommig_rel"+std::to_string(id)).c_str())(incomming_rel_alloc);
    outgoing_rel = segment.find_or_construct<Rel_Pointer_Vec>(("outgoing_rel"+std::to_string(id)).c_str())(outgoing_rel_alloc);
    propertys = segment.find_or_construct<Property_Map>(("node_propertys"+std::to_string(id)).c_str())(property_alloc);

    //segment.construct<sharable_mutex_type>("test")(segment.get_segment_manager());
}

Node::Node(Node&& n){
    bi::scoped_lock<sharable_mutex_type> lock_prop(n.write_property);
    bi::scoped_lock<sharable_mutex_type> lock_inc(n.write_inc);
    bi::scoped_lock<sharable_mutex_type> lock_out(n.write_out);
    id = std::move(n.id);
    incomming_rel = std::move(n.incomming_rel);
    outgoing_rel = std::move(n.outgoing_rel);
    propertys = std::move(n.propertys);
}
/*
Node::~Node(){
    incomming_rel->~vector();
    outgoing_rel->~vector();
    propertys->clear();
}
*/
void Node::add_property(bi::string key, boost::any value){
    bi::scoped_lock<sharable_mutex_type> lock(write_property);
    propertys->insert(std::make_pair(key, value));
}

void Node::remove_property(bi::string key){
    bi::scoped_lock<sharable_mutex_type> lock(write_property);
    propertys->erase(key);
}

const boost::any Node::read_property(bi::string key){
    bi::sharable_lock<sharable_mutex_type> lock(write_property);
    return propertys->at(key);
}

bool Node::change_property(bi::string key, std::function<void(boost::any&)> f){ //idee, übergebe eine Funktion die verändert und return true, wenn fertig
    bi::scoped_lock<sharable_mutex_type> lock(write_property);
        try{
            f(propertys->at(key));
            return true;
        }catch(...){
            return false;
        }
}

void Node::add_incomming_rel(bi::offset_ptr<Relationship> input){
    bi::scoped_lock<sharable_mutex_type> lock(write_inc);
    incomming_rel->push_back(input);
}

void Node::remove_incomming_rel(bi::offset_ptr<Relationship> input){
    bi::scoped_lock<sharable_mutex_type> lock(write_inc);
    for(auto start=incomming_rel->begin(); start != incomming_rel->end(); start++){
        if((*start)==input){
            incomming_rel->erase(start);
            break;
        }
    }
}

const bi::offset_ptr<Rel_Pointer_Vec> Node::get_incomming_rel(){
    bi::sharable_lock<sharable_mutex_type> lock(write_inc);
    return incomming_rel;
}

void Node::add_outgoing_rel(bi::offset_ptr<Relationship> input){
    bi::scoped_lock<sharable_mutex_type> lock(write_out);
    outgoing_rel->push_back(input);
}

void Node::remove_outgoing_rel(bi::offset_ptr<Relationship> input){
    bi::scoped_lock<sharable_mutex_type> lock(write_out);
    for(auto start=outgoing_rel->begin(); start != outgoing_rel->end(); start++){
        if((*start)==input){
            outgoing_rel->erase(start);
            break;
        }
    }
}

const bi::offset_ptr<Rel_Pointer_Vec> Node::get_outgoing_rel(){
    bi::sharable_lock<sharable_mutex_type> lock(write_out);
    return outgoing_rel;
}

node::id_t Node::get_id(){
    return id;
}

Node& Node::operator=(const Node& n){
    //bi::scoped_lock<const sharable_mutex_type> lock_prop(n.write_property);
    //bi::scoped_lock<const sharable_mutex_type> lock_inc(n.write_inc);
    //bi::scoped_lock<const sharable_mutex_type> lock_out(n.write_out);
    id = std::move(n.id);
    incomming_rel = n.incomming_rel;
    outgoing_rel = n.outgoing_rel;
    propertys = n.propertys;
    return *this;
}