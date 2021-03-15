#include "graph.hpp"
#include <primesieve.hpp>

#include <future>
#include "thread_pool.hpp"
#include <random>

#ifndef GRAPH_REL_ITER_HPP
#define GRAPH_REL_ITER_HPP

class Graph_Rel_Iterator{
    boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> begin;
    boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> end;
    std::vector<size_t> thread_limits;
    std::vector<u_int64_t> prime_numbers;

    public:
        Graph_Rel_Iterator(boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> begin_input, boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> end_input, size_t thread_count = std::thread::hardware_concurrency());

        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_begin();

        boost::container::vec_iterator<bi::offset_ptr<rel_unique_ptr_type>, false> get_end();

        const std::vector<size_t>& get_thread_limits();

        const std::vector<u_int64_t>& get_prime_numbers();
};

using FunctionTypeRel = std::function<void(Relationship*)>;

void for_each(Graph_Rel_Iterator g_it, FunctionTypeRel f);

void for_each_random(Graph_Rel_Iterator g_it, FunctionTypeRel f);

#endif