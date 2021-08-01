#include "graph.hpp"
#include "graph_node_iterator.hpp"

struct RelationshipWeight{
    relationship::id_t id;
    double weight;

    RelationshipWeight(relationship::id_t input_id, double input_weight){
        id = input_id;
        weight = input_weight;
    }
};

double convertBoostAnyToDouble(boost::any input){
    try{
        return boost::any_cast<double>(input);
    } catch (boost::bad_any_cast &e){ //sollte boost::any vom Typ int sein, schlägt die obere Umwandlung mit boost::bad_any_cast fehl.
        return static_cast<double>(boost::any_cast<int>(input)); // boost::any wird erst in einen integer umgewandelt um dann in eine double-Typ überführt zu werden. 
    }
}

std::vector<node::id_t> determin_max_values_ids(std::vector<RelationshipWeight>& from_node){
    //initialisiere die benötigten Variablen
    std::vector<node::id_t> result = {};
    bool found_max_last_round = true;
    RelationshipWeight max = RelationshipWeight(0,std::numeric_limits<double>::min());
    int position_of_max_ele = 0;

    //solange wie in der letzten Runde ein Element >= dem maximalen Element gefunden wurde, wird nach einem weiteren maximalen Element gesucht. -> es werden alle maximale Elemente gefunden. 
    while ((found_max_last_round) && (!from_node.empty())){
        found_max_last_round = false;
        auto max_entry = std::max_element(from_node.begin(),from_node.end(),[] (const RelationshipWeight &a, const RelationshipWeight &b) {
            return a.weight < b.weight;
        });
        position_of_max_ele = std::distance(from_node.begin(), max_entry);
        if(max.weight <= from_node[position_of_max_ele].weight){
            max = from_node[position_of_max_ele];
            result.push_back(max.id);
            found_max_last_round = true;
            from_node.erase(from_node.begin() + position_of_max_ele);
        }
    }
    return result;
};

void init_nodes_in_graph(graph_db_ptr& graph, Graph& g){
    graph->begin_transaction();
    graph->nodes([&g](auto& n){
       g.add_node(n.id());
    });
    graph->commit_transaction();
}
/*
void init_rel_with_function(graph_db_ptr& graph, Graph& g){

    std::string property = "values";
    double default_value = 1.0;

    g.initialise_relationships(graph, [&graph, &property, &default_value](std::vector<std::unique_ptr<Node>>::iterator start){
        std::vector<RelationshipWeight> from_node = {};
        std::vector<node::id_t> ret = {};
                    node::id_t active_node = (*start)->get_id();
                    graph->foreach_from_relationship_of_node(graph->node_by_id(active_node), [&] (relationship& r) {
                        if(graph->get_rship_description(r.id()).has_property(property)){
                            from_node.push_back(RelationshipWeight(r.to_node_id(),convertBoostAnyToDouble(graph->get_rship_description(r.id()).properties.at(property))));
                        } else{
                            // sollte die Kante nicht die Property haben, wird der übergebene default_value verwendet 
                            from_node.push_back(RelationshipWeight(r.to_node_id(),default_value)); 
                        }
                     });
        
                    ret = determin_max_values_ids(from_node);
                    return ret;
    });
}
*/

void init_rel(graph_db_ptr& graph, Graph& g){

    std::string property = "values";
    double default_value = 1.0;

    auto tx = graph->begin_transaction();

    for_each(Graph_Node_Iterator(g.get_node_iterator_begin(), g.get_node_iterator_end()), [&graph, &g, &property, &default_value, &tx](Node* it){
        std::vector<RelationshipWeight> from_node = {};
        std::vector<node::id_t> ret = {};
                    node::id_t active_node = it->get_id();
                    current_transaction_ = tx;
                    graph->foreach_from_relationship_of_node(graph->node_by_id(active_node), [&] (relationship& r) {
                        if(graph->get_rship_description(r.id()).has_property(property)){
                            from_node.push_back(RelationshipWeight(r.to_node_id(),convertBoostAnyToDouble(graph->get_rship_description(r.id()).properties.at(property))));
                        } else{
                            // sollte die Kante nicht die Property haben, wird der übergebene default_value verwendet 
                            from_node.push_back(RelationshipWeight(r.to_node_id(),default_value)); 
                        }
                     });
        
                    ret = determin_max_values_ids(from_node);
                    for(size_t i = 0; i<ret.size(); i++){
                        g.add_relationship(0, it, (g.get_node_iterator_begin() + ret[i])->get());
                    }
    });
    graph->commit_transaction();
}

void init_labels(Graph& g){
    for_each(Graph_Node_Iterator(g.get_node_iterator_begin(), g.get_node_iterator_end()), [](Node* n){
        n->add_property("label", n->get_id());
    });
}

void label_prop(Graph& g){

    Graph_Node_Iterator iter(g.get_node_iterator_begin(), g.get_node_iterator_end());
    bool did_change = true;
    size_t turns = 0;
    size_t max_turns = 4000;

    std::random_device rd;  
    std::mt19937 rng(rd());

    while(did_change && turns < max_turns){
        did_change = false;
        for_each_random(iter, [&did_change, &rng](Node* n){
            if(n->get_outgoing_rel()->size() > 1){
                std::uniform_int_distribution<int> distrib(0,n->get_outgoing_rel()->size()-1);
                size_t pos = distrib(rng);
                if(boost::any_cast<node::id_t>(n->read_property("label")) != boost::any_cast<node::id_t>(n->get_outgoing_rel()->at(pos)->get_to_node()->read_property("label"))){
                    n->change_property("label", [&n, &pos](boost::any& a){
                        a = n->get_outgoing_rel()->at(pos)->get_to_node()->read_property("label");
                    });
                    did_change = true;
                }
            }else if(n->get_outgoing_rel()->size() == 1){
                if(boost::any_cast<node::id_t>(n->read_property("label")) != boost::any_cast<node::id_t>(n->get_outgoing_rel()->at(0)->get_to_node()->read_property("label"))){
                    n->change_property("label", [&n](boost::any& a){
                        a = n->get_outgoing_rel()->at(0)->get_to_node()->read_property("label");
                    });
                    did_change = true;
                }
            }
        });
        turns++;
    }
    std::cout << "Turns taken: " << turns << std::endl;
}

void label_prop_serial(Graph& g){

    Graph_Node_Iterator iter(g.get_node_iterator_begin(), g.get_node_iterator_end());
    bool did_change = true;
    size_t turns = 0;
    size_t max_turns = 4000;
    size_t distance = std::distance(g.get_node_iterator_begin(), g.get_node_iterator_end());

    std::random_device rd;  
    std::mt19937 rng(rd()); 

    std::vector<u_int64_t> prime_numbers={};

    auto p = primesieve::iterator(distance);

    for(int i=0; i<10; i++){
        prime_numbers.push_back(p.next_prime());
    }

    auto it_start = g.get_node_iterator_begin();

    while(did_change && turns < max_turns){
        did_change = false;
        std::uniform_int_distribution<int> distrib(0,prime_numbers.size()-1);
        u_int64_t current_prime = prime_numbers[distrib(rng)];
        u_int64_t offset = current_prime % distance;
        for(auto it = g.get_node_iterator_begin(); it != g.get_node_iterator_end(); it++){
            auto n = (it_start+offset)->get();
            if(n->get_outgoing_rel()->size() > 1){
                std::uniform_int_distribution<int> distrib(0,n->get_outgoing_rel()->size()-1);
                size_t pos = distrib(rng);
                if(boost::any_cast<node::id_t>(n->read_property("label")) != boost::any_cast<node::id_t>(n->get_outgoing_rel()->at(pos)->get_to_node()->read_property("label"))){
                    n->change_property("label", [&n, &pos](boost::any& a){
                        a = n->get_outgoing_rel()->at(pos)->get_to_node()->read_property("label");
                    });
                    did_change = true;
                }
            }else if(n->get_outgoing_rel()->size() == 1){
                if(boost::any_cast<node::id_t>(n->read_property("label")) != boost::any_cast<node::id_t>(n->get_outgoing_rel()->at(0)->get_to_node()->read_property("label"))){
                    n->change_property("label", [&n](boost::any& a){
                        a = n->get_outgoing_rel()->at(0)->get_to_node()->read_property("label");
                    });
                    did_change = true;
                }
            }
            offset = (offset + current_prime) % distance;
        }
        turns++;
    }
    std::cout << "Turns taken: " << turns << std::endl;
}

int main(){
    Graph g("Example.vec", 200000000);

    std::string path_Graphs = "../../../C++_Programme/Poseidon_GraphAnalytics/test/graph/";

    auto pool = graph_pool::open(path_Graphs + "20000nodeGraph");
    auto graph = pool->open_graph("20000nodeGraph");

    auto start_init = std::chrono::high_resolution_clock::now();

    init_nodes_in_graph(graph, g);

    auto stop_init = std::chrono::high_resolution_clock::now();

    auto duration_init = std::chrono::duration_cast<std::chrono::microseconds>(stop_init - start_init); 
  
    std::cout << "Time taken by init: " 
      << duration_init.count() << " microseconds" << std::endl;

    auto start_rel = std::chrono::high_resolution_clock::now();

    init_rel(graph, g);

    auto stop_rel = std::chrono::high_resolution_clock::now();

    auto duration_rel = std::chrono::duration_cast<std::chrono::microseconds>(stop_rel - start_rel); 
  
    std::cout << "Time taken by rel: " 
      << duration_rel.count() << " microseconds" << std::endl;

    auto start_init_label= std::chrono::high_resolution_clock::now();

    init_labels(g);

    auto stop_init_label = std::chrono::high_resolution_clock::now();

    auto duration_init_label = std::chrono::duration_cast<std::chrono::microseconds>(stop_init_label - start_init_label); 
  
    std::cout << "Time taken by init_label: " 
      << duration_init_label.count() << " microseconds" << std::endl;

    auto start_label_prop= std::chrono::high_resolution_clock::now();

    label_prop(g);
    //label_prop_serial(g);

    auto stop_label_prop = std::chrono::high_resolution_clock::now();

    auto duration_label_prop = std::chrono::duration_cast<std::chrono::microseconds>(stop_label_prop - start_label_prop); 
  
    std::cout << "Time taken by label_prop: " 
      << duration_label_prop.count() << " microseconds" << std::endl;

    graph->begin_transaction();
    
    /*
    for(auto it=g.get_node_iterator_begin(); it != g.get_node_iterator_end();it++){
        std::cout << "Knoten " << graph->get_node_description((*it)->get_id()).properties.at("name") << " hat Label: " << (*it)->read_property("label") << std::endl;
    }
    */

    /*
    for(auto it=g.get_node_iterator_begin(); it != g.get_node_iterator_end();it++){
        std::cout << graph->get_node_description((*it)->get_id()).properties.at("name") << "   :";
        for(int s = 0; s<(*it)->get_outgoing_rel().size(); s++){
            try{std::cout << graph->get_node_description((*it)->get_outgoing_rel()[s]->get_to_node()->get_id()).properties.at("name") << ",     ";}catch(...){std::cout << (*it)->get_outgoing_rel()[s]->get_id() << std::endl;}
        }
        std::cout << std::endl;
    }
    */

    /*
    for(auto it = g.get_rel_iterator_begin(); it != g.get_rel_iterator_end(); it++){
        std::cout << graph->get_node_description((*it)->get_from_node()->get_id()).properties.at("name") << "  nach: " << graph->get_node_description((*it)->get_to_node()->get_id()).properties.at("name") << std::endl;
    }*/

    graph->commit_transaction();
}