#ifndef __TREEWINDOWFITTER_H
#define __TREEWINDOWFITTER_H

#include "tree/tree.h"
#include "tree/node.h"
#include <iostream>

template<typename T>
class TreeWindowFitter
{
public:
    int get_nr_levels_that_fit_in_window(treelib::Tree<T> tree,
                                         int window_size,
                                         std::shared_ptr<treelib::Node<T>> selected);
};

template<typename T>
int TreeWindowFitter<T>::get_nr_levels_that_fit_in_window(treelib::Tree<T> tree,
        int window_size,
        std::shared_ptr<treelib::Node<T>> selected) {
    using NodeP = std::shared_ptr<treelib::Node<T>>;

    using NodeDepth = std::pair<NodeP, int>;
    std::queue<NodeDepth> bfs_queue;
    bfs_queue.push(std::make_pair(selected, 0));
    int nr_nodes_to_print_until_previous_level = 0;
    int nr_nodes_in_current_depth = 0;
    int nr_depths_to_print = 0;
    while(not bfs_queue.empty() and
          nr_nodes_to_print_until_previous_level + nr_nodes_in_current_depth <= window_size + 1) {

        auto node_depth_pair = bfs_queue.front();
        auto node = std::get<0>(node_depth_pair);
        auto depth = std::get<1>(node_depth_pair);
        bfs_queue.pop();
        for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
            if ((*iter)->is_matching_search or (*iter)->is_ancestor_of_matching_search) {
                auto child_node_depth_pair = std::make_pair(*iter, depth + 1);
                bfs_queue.push(child_node_depth_pair);
            }
        }

        // Add the previous fully-scanned level as a printed
        ++nr_nodes_in_current_depth;
        int previous_depth = nr_depths_to_print;
        const bool first_in_new_depth = previous_depth != depth;
        if (first_in_new_depth or bfs_queue.empty()) {
            if (nr_nodes_to_print_until_previous_level <= window_size) {
                ++nr_depths_to_print;
                nr_nodes_to_print_until_previous_level += nr_nodes_in_current_depth;
            }
            nr_nodes_in_current_depth = 1;
            if (bfs_queue.empty() and first_in_new_depth) {
                if (nr_nodes_to_print_until_previous_level + 1 <= window_size) {
                    ++nr_depths_to_print;
                }
            }
        }
    }
    return nr_depths_to_print;
}

#endif
