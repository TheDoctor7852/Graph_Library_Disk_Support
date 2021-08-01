#include "../header/relationship.hpp"

Relationship::Relationship(relationship::id_t input, bi::offset_ptr<Node> from_node, bi::offset_ptr<Node> to_node, bi::managed_mapped_file& segment){
    id = input;
    from = from_node.get();
    to = to_node.get();
    const PropMapAllocator property_alloc(segment.get_segment_manager());
    propertys = segment.find_or_construct<Property_Map>(("rel_propertys"+std::to_string(id)).c_str())(property_alloc);
}

Relationship::Relationship(const Relationship& r){
    id = std::move(r.id);
    to = std::move(r.to);
    from = std::move(r.from);
    propertys = std::move(r.propertys);
}

/*
Relationship::Relationship(Relationship&& r){
    std::unique_lock<std::shared_mutex> lock(r.write);
    id = std::move(r.id);
    to = std::move(r.to);
    from = std::move(r.from);
    propertys = std::move(r.propertys);
}
*/
/*Relationship::~Relationship(){ //wird durch den Graphen gelöscht
    //from->remove_outgoing_rel(this);
    //to->remove_incomming_rel(this);
}*/

void Relationship::add_property(bi::string key, boost::any value){
    bi::scoped_lock<sharable_mutex_type> lock(write);
    propertys->insert(std::make_pair(key, value));
}

void Relationship::remove_property(bi::string key){
    bi::scoped_lock<sharable_mutex_type> lock(write);
    propertys->erase(key);
}

bool Relationship::change_property(bi::string key, std::function<void(boost::any&)> f){
    bi::scoped_lock<sharable_mutex_type> lock(write);
        try{
            f(propertys->at(key));
            return true;
        }catch(...){
            return false;
        }
}

const boost::any Relationship::read_property(bi::string key){
    bi::sharable_lock<sharable_mutex_type> lock(write);
    return propertys->at(key);
}

relationship::id_t Relationship::get_id(){
    return id;
}

bi::offset_ptr<Node> Relationship::get_from_node(){
     return from;
}

bi::offset_ptr<Node> Relationship::get_to_node(){
    return to;
}

Relationship& Relationship::operator=(const Relationship& r){
    //bi::scoped_lock<const sharable_mutex_type> lock(r.write);
    id = std::move(r.id);
    to = std::move(r.to);
    from = std::move(r.from);
    propertys = std::move(r.propertys);
    return *this;
}