#include "graph_relationship_iterator.hpp"

Graph_Rel_Iterator::Graph_Rel_Iterator(boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> begin_input, boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> end_input, size_t thread_count){
    begin = begin_input;
    end = end_input;
    size_t distance = std::distance(begin, end);

    thread_limits = {0};

    for(size_t i=0; i<thread_count; i++){
        if(i<(distance % thread_count)){
            thread_limits.push_back(thread_limits[i]+1+(distance/thread_count));
        } else{
            thread_limits.push_back(thread_limits[i]+(distance/thread_count));
        }
    }

    prime_numbers={};

    auto p = primesieve::iterator(thread_limits[1]-thread_limits[0]);

    for(int i=0; i<10; i++){
        prime_numbers.push_back(p.next_prime());
    }
}

boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> Graph_Rel_Iterator::get_begin(){
    return begin;
}

boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> Graph_Rel_Iterator::get_end(){
    return end;
}

const std::vector<size_t>& Graph_Rel_Iterator::get_thread_limits(){
    return thread_limits;
}

const std::vector<u_int64_t>& Graph_Rel_Iterator::get_prime_numbers(){
    return prime_numbers;
}

void for_each(Graph_Rel_Iterator g_it, FunctionTypeRel f){
    thread_pool pool;
    std::vector<std::future<bool>> futures;
    for(size_t i=0;i< g_it.get_thread_limits().size()-1;i++){
        futures.emplace_back(pool.submit([&g_it,f, i](){
            auto it_start = g_it.get_begin() + g_it.get_thread_limits()[i];
            auto it_end = g_it.get_begin() + g_it.get_thread_limits()[i+1];
       
            for(auto it = it_start; it != it_end; it++){
                f(it->get().get());
            }
          return true;
        }));
    }
    for(auto it = futures.begin(); it != futures.end(); it++){
        it->get();
    }
}

void for_each_random(Graph_Rel_Iterator g_it, FunctionTypeRel f){
    thread_pool pool;
    std::vector<std::future<bool>> futures;
    for(size_t i=0;i< g_it.get_thread_limits().size()-1;i++){
        futures.emplace_back(pool.submit([&g_it,f, i](){
            std::random_device rd;  
            std::mt19937 rng(rd()); 
            std::uniform_int_distribution<int> distrib(0,g_it.get_prime_numbers().size()-1);
            u_int64_t current_prime = g_it.get_prime_numbers()[distrib(rng)];
            auto start = g_it.get_thread_limits()[i];
            auto end = g_it.get_thread_limits()[i+1];
            auto iter = g_it.get_begin() + g_it.get_thread_limits()[i];
            u_int64_t offset = current_prime % (end-start);
            for(auto it = start; it < end; it++){
                f((iter+offset)->get().get());
                offset = (offset + current_prime) % (end-start);
            }
          return true;
        }));
    }
    for(auto it = futures.begin(); it != futures.end(); it++){
        it->get();
    }
}