#include "../header/graph.hpp"

#include <future>
#include "thread_pool.hpp"

Graph::Graph(std::string name, size_t size){

    try{
       std::remove(name.c_str());
    }catch(...){}

    name_file = name;
    segment = bi::managed_mapped_file(bi::create_only, name.c_str(), size);

    const Node_Ptr_Allocator node_alloc(segment.get_segment_manager());
    const Rel_Ptr_Allocator rel_alloc(segment.get_segment_manager());


    node_vec = segment.find_or_construct<Node_Ptr_Vec>("Nodes")(node_alloc);
    rel_vec = segment.find_or_construct<Rel_Ptr_Vec>("Relationships")(rel_alloc);
}

Graph::~Graph(){
    try{
       std::remove(name_file.c_str());
    }catch(...){}
}
/*
//statt das in constructor zu machen, könnte stadessen auch eine extra function für relationships machen -> sowas wie init_rel_ships -> sollte lieber erstmal diese funktion machen und zum laufen bekommen
Graph::Graph(graph_db_ptr& graph, std::function<void()> f_nodes, std::function<std::vector<node::id_t>(std::vector<std::unique_ptr<Node>>::iterator)> f_rels, size_t thread_count){
    auto tx = graph->begin_transaction();
        f_nodes();
    graph->commit_transaction();

    std::vector<bool> futures = {};
    thread_pool pool = thread_pool(thread_count);
    std::vector<size_t> thread_startAndEnd = {0};

    for(size_t i=0; i<thread_count; i++){
      if(i<(node_vec.size() % thread_count)){
        thread_startAndEnd.push_back(thread_startAndEnd[i]+1+(node_vec.size()/thread_count));
      } else{
        thread_startAndEnd.push_back(thread_startAndEnd[i]+(node_vec.size()/thread_count));
      }
    }
    
    auto it = node_vec.begin();

    for(size_t i=0;i<thread_startAndEnd.size()-1;i++){
          std::future<bool> f = pool.submit([it, i, &graph, &thread_startAndEnd, &f_rels, this](){
              auto it_start = it + thread_startAndEnd[i];
              auto it_end = it + thread_startAndEnd[i+1];
              graph->begin_transaction();
              std::vector<std::unique_ptr<Relationship>> adding_neighbours;
              std::vector<node::id_t> ret = {};
                for(auto start = it_start; start != it_end; start++){
                    ret = f_rels(start);
                    for(size_t i= 0; i<ret.size(); i++){
                        adding_neighbours.emplace_back(std::make_unique<Relationship>(Relationship(0, start->get(), node_vec[ret[i]].get())));
                    }
                }
                graph->commit_transaction();
                std::unique_lock<std::shared_mutex> lock(write_rel);
                rel_vec.insert(rel_vec.end(), std::make_move_iterator(adding_neighbours.begin()), std::make_move_iterator(adding_neighbours.end()));
              return true;
          });
    }
}
*/
/*
void Graph::initialise_relationships(graph_db_ptr& graph, std::function<std::vector<node::id_t>(boost::interprocess::offset_ptr<Node>)> f_rels, size_t thread_count){
    std::vector<std::future<bool>> futures = {};
    thread_pool pool = thread_pool(thread_count);
    std::vector<size_t> thread_startAndEnd = {0};

    for(size_t i=0; i<thread_count; i++){
      if(i<(node_vec->size() % thread_count)){
        thread_startAndEnd.push_back(thread_startAndEnd[i]+1+(node_vec->size()/thread_count));
      } else{
        thread_startAndEnd.push_back(thread_startAndEnd[i]+(node_vec->size()/thread_count));
      }
    }

    bi::sharable_lock<sharable_mutex_type> l(write_nodes);

    auto it = node_vec->begin();

    for(size_t i=0;i<thread_startAndEnd.size()-1;i++){
          std::future<bool> f = pool.submit([it, i, &graph, &thread_startAndEnd, &f_rels, this](){
              auto it_start = it + thread_startAndEnd[i];
              auto it_end = it + thread_startAndEnd[i+1];
              graph->begin_transaction();
              std::vector<Relationship> adding_neighbours;
              std::vector<node::id_t> ret = {};
                for(auto start = it_start; start != it_end; start++){
                    ret = f_rels(start.get_ptr());
                    for(size_t i= 0; i<ret.size(); i++){
                        auto h = adding_neighbours.emplace_back((Relationship(0, start.get_ptr(), bi::offset_ptr<Node>(node_vec->at(ret[i])))).get();
                        h->get_from_node()->add_outgoing_rel(h);//baue noch eine private funktion ein, die das übernehmen kann -> die kann dann mit this->func aufgerufen werden -> man umgeht, dass man das public machen muss
                        h->get_to_node()->add_incomming_rel(h);
                    }
                }
                graph->commit_transaction();
                std::unique_lock<std::shared_mutex> lock(write_rel);
                rel_vec->insert(rel_vec.end(), std::make_move_iterator(adding_neighbours.begin()), std::make_move_iterator(adding_neighbours.end()));
              return true;
          });
        futures.push_back(std::move(f));
    }

    for(size_t i = 0; i<futures.size(); i++){
        futures[i].get();
    }
}
*/
bi::offset_ptr<Node> Graph::add_node(node::id_t input){
    bi::scoped_lock<sharable_mutex_type> lock(write_nodes);
    auto p = node_vec->emplace_back(bi::make_managed_unique_ptr(segment.construct<Node>(bi::anonymous_instance)(input, segment),segment)).get();
    return p.get();
}

bi::offset_ptr<Relationship> Graph::add_relationship(size_t id, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  from_node, boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  to_node){
    bi::sharable_lock<sharable_mutex_type> lock_node(write_nodes);
    bi::scoped_lock<sharable_mutex_type> lock_rel(write_rel);
    try{
        //if(rel_id_q.empty()){
        auto h = rel_vec->emplace_back(bi::make_managed_unique_ptr(segment.construct<Relationship>(bi::anonymous_instance)(id, from_node.get_ptr().get()->get(), to_node.get_ptr().get()->get(), segment), segment)).get();
        //lock_rel.unlock();
        from_node->get()->add_outgoing_rel(h);
        to_node->get()->add_incomming_rel(h);
        return h;
        /*}else{
            size_t id = rel_id_q.top();
            rel_id_q.pop();
            rel_vec[id] =  std::move(std::make_unique<Relationship>(Relationship(id, from_node->get(), to_node->get())));
            //lock_rel.unlock();
            (*from_node)->add_outgoing_rel(rel_vec.back().get());
            (*to_node)->add_incomming_rel(rel_vec.back().get());
            return rel_vec[id].get();
        }   */
    }catch(...){
        return nullptr;
    }
}

bi::offset_ptr<Relationship> Graph::add_relationship(size_t id, bi::offset_ptr<Node> from_node, bi::offset_ptr<Node> to_node){
    bi::sharable_lock<sharable_mutex_type> lock_node(write_nodes);
    bi::scoped_lock<sharable_mutex_type> lock_rel(write_rel);
        //if(rel_id_q.empty()){
        auto h = rel_vec->emplace_back(bi::make_managed_unique_ptr(segment.construct<Relationship>(bi::anonymous_instance)(id, from_node, to_node, segment), segment)).get();
        //lock_rel.unlock();
        from_node->add_outgoing_rel(h);
        to_node->add_incomming_rel(h);
        return h;
        /*}else{
            size_t id = rel_id_q.top();
            rel_id_q.pop();
            rel_vec[id] =  std::move(std::make_unique<Relationship>(Relationship(id, from_node, to_node)));
            //lock_rel.unlock();
            from_node->add_outgoing_rel(rel_vec.back().get());
            to_node->add_incomming_rel(rel_vec.back().get());
            return rel_vec[id].get();
        }   */
}

void Graph::delete_node(boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  it){
    bi::scoped_lock<sharable_mutex_type> lock(write_nodes);
    if(it->get()->get_incomming_rel()->empty() && it->get()->get_outgoing_rel()->empty()){
        node_vec->erase(it);
    }
}

void Graph::delete_rel(boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false>  it){
    bi::scoped_lock<sharable_mutex_type> lock(write_rel);
    it->get()->get_from_node()->remove_outgoing_rel(it.get_ptr().get()->get());
    it->get()->get_to_node()->remove_incomming_rel(it.get_ptr().get()->get());
    rel_vec->erase(it);
}

boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false>  Graph::get_node_iterator_begin(){
    return node_vec->begin();
}

boost::container::vec_iterator<bi::offset_ptr<node_unique_ptr_type>, false> Graph::get_node_iterator_end(){
    return node_vec->end();
}

boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> Graph::get_rel_iterator_begin(){
    return rel_vec->begin();
}

boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> Graph::get_rel_iterator_end(){
    return rel_vec->end();
}