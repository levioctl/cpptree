#ifndef __TREE_PRINTER_H
#define __TREE_PRINTER_H

#include <ostream>
#include <vector>

#include "tree/node.h"
#include "tree/tree.h"
#include "pre_print_tree_analysis.h"
#include "paginator.h"
#include "treewindowfitter.h"

namespace treelib {

template<typename T>
class Tree;

template<typename T>
class TreePrinter
{
public:
    // Today i learned: 'using' keyword (same as typedef)
    using node_t = const std::shared_ptr< Node<T> >;
    TreePrinter(Tree<T> &tree);
    void print(std::ostream &out, bool filter_search_nodes = false,
               node_t selection = nullptr, int window_height = 100);
    std::shared_ptr<treelib::Node<T>> get_next_printed_node_after_selected(void);
    std::shared_ptr<treelib::Node<T>> get_printed_node_before_selected(void);

private:
    void print_node(node_t node,
                    node_t previous_node,
                    int depth,
                    int previous_depth,
                    std::ostream &out,
                    std::shared_ptr<treelib::Node<T>> selection,
                    node_t _printed_subtree_root);

    void init_pre_dfs_state(std::shared_ptr<treelib::Node<T>> selection,
                            int window_height);
    void update_mid_dfs_state(std::shared_ptr<Node<T>> node,
                              std::shared_ptr<Node<T>> selection);
    void expand_dfs_to_children_of_node(std::shared_ptr<Node<T>> node,
                                   std::shared_ptr<Node<T>> selection,
                                   int window_height,
                                   int depth);
    void update_depth_to_next_sibling_map(int previous_depth,
                                          int depth,
                                          std::shared_ptr<Node<T>> previous_node);
    void populate_stack_with_paginated_children_of_node(node_t node,
                                       node_t selection,
                                       int child_depth,
                                       int window_height);
    void populate_stack_with_all_children_of_node(std::shared_ptr<Node<T>> node,
                                                  int children_depth);

    std::stack<std::pair<node_t, int>> _dfs_stack;
    TreeWindowFitter<T> _tree_window_fitter;
    void choose_printed_tree_root(std::shared_ptr<treelib::Node<T>> selection,
                                  int window_height);
    Tree<T>& _tree;
    TreeAnalysisInfo info;
    std::shared_ptr< Node<T> > _printed_node_before_selected;
    std::shared_ptr< Node<T> > _next_printed_node_after_selected;
    std::shared_ptr< Node<T> > _printed_subtree_root;
    int _printed_subtree_root_depth;
    int _nr_levels_that_fit_in_window;
    static constexpr std::size_t MAX_DEPTH = 512;
    int _selection_depth;
    bool depth_to_next_sibling[MAX_DEPTH];
    std::shared_ptr<Node<T>> _previously_printed_node;
    bool _was_previously_printed_node_selected;
};

template <typename T>
TreePrinter<T>::TreePrinter(Tree<T>& tree): _tree(tree),
                                            info(analyze_tree_for_printing(_tree.get_root())),
                                            _printed_subtree_root(tree.get_root())
{}

template <typename T>
void TreePrinter<T>::init_pre_dfs_state(std::shared_ptr<treelib::Node<T>> selection,
                                        int window_height) {
    _next_printed_node_after_selected.reset();
    _printed_node_before_selected.reset();
    _previously_printed_node.reset();

    _selection_depth = _tree.get_node_depth(selection);

    // Determine the printed subtree root
    choose_printed_tree_root(selection, window_height);

    _was_previously_printed_node_selected = false;
}

template <typename T>
void TreePrinter<T>::print(std::ostream &out,
                           bool filter_search_nodes,
                           std::shared_ptr<Node<T>> selection,
                           int window_height) {
    if (nullptr == _tree.get_root()) {
        return;
    }

    init_pre_dfs_state(selection, window_height);

    // Prepare variables for DFS scan for printing
    int depth = 0;
    auto node = _printed_subtree_root;
    _dfs_stack.push(std::make_pair(node, depth));

    // DFS scan
    while (not _dfs_stack.empty()) {
        // Store previous values before reassigning new ones
        auto previous_depth = depth;
        auto previous_node = node;

        // Extract node from DFS stack
        node = std::get<0>(_dfs_stack.top());
        depth = std::get<1>(_dfs_stack.top());
        _dfs_stack.pop();

        // Skip nodes that don't match an ongoing search
        const bool skip_due_to_search_mismatch = (filter_search_nodes and
            not node->is_ancestor_of_matching_search and not node->is_matching_search);
        if (skip_due_to_search_mismatch) {
            continue;
        }

        // Print node
        print_node(node, previous_node, depth, previous_depth, out, selection,
                   _printed_subtree_root);

        // Add children to DFS stack
        expand_dfs_to_children_of_node(node, selection, depth, window_height);

        // Update state that requires update during iteration
        update_mid_dfs_state(node, selection);
    }
}

template <typename T>
void TreePrinter<T>::update_depth_to_next_sibling_map(int previous_depth,
                                                      int depth,
                                                      std::shared_ptr<Node<T>> previous_node) {
    // Maintain the depth-to-next-sibnling map
    const bool is_first_node_of_parent = previous_depth < depth;
    if (is_first_node_of_parent) {
        const bool next_sibling_of_parent_exists =
            info.m_node_to_next_sibling_existance[previous_node->identifier];
        depth_to_next_sibling[depth - 1] = next_sibling_of_parent_exists;
    }
}

template<typename T>
void TreePrinter<T>::expand_dfs_to_children_of_node(std::shared_ptr<Node<T>> node,
        std::shared_ptr<Node<T>> selection,
        int depth,
        int window_height) {
    int children_depth = depth + 1;
    const int highest_depth_that_fits_window = _nr_levels_that_fit_in_window - 1;
    const bool depth_fits_window = children_depth <= highest_depth_that_fits_window;
    int selection_rel_depth = _selection_depth - _printed_subtree_root_depth;
    const bool should_paginate = not depth_fits_window
        and selection_rel_depth == highest_depth_that_fits_window + 1
        and children_depth == highest_depth_that_fits_window + 1;

    if (depth_fits_window) {
        populate_stack_with_all_children_of_node(node, children_depth);
    } else if (should_paginate) {
        populate_stack_with_paginated_children_of_node(node,
                                                       selection,
                                                       children_depth,
                                                       window_height);
    }
}

template <typename T>
void TreePrinter<T>::update_mid_dfs_state(std::shared_ptr<Node<T>> node,
                                          std::shared_ptr<Node<T>> selection) {

    // Update next printed node after selected
    const bool is_selection = node == selection;
    if (is_selection) {
        _was_previously_printed_node_selected = true;
        _printed_node_before_selected = _previously_printed_node;
    } else if (_was_previously_printed_node_selected) {
        _next_printed_node_after_selected = node;
        _was_previously_printed_node_selected = false;
    }

    // Update previously printed node
    _previously_printed_node = node;
}

template <typename T>
void TreePrinter<T>::populate_stack_with_all_children_of_node(std::shared_ptr<Node<T>> node,
                                                              int children_depth) {
    // Add children of current node to DFS stack.
    // Iterate in reverse to cancel the stack's reverse effect
    for (auto iter = node->children.rbegin(); iter != node->children.rend(); ++iter) {
        auto current_node_depth_pair = std::make_pair(*iter, children_depth);
        _dfs_stack.push(current_node_depth_pair);
    }
}

template <typename T>
void TreePrinter<T>::populate_stack_with_paginated_children_of_node(
        std::shared_ptr<Node<T>> node,
        std::shared_ptr<Node<T>> selection,
        int child_depth,
        int window_height) {
    std::size_t nr_children = node->children.size();

    // Find index of selected child
    std::size_t selection_idx_in_level = 0;
    for (auto iter = node->children.begin();
             iter != node->children.end() and (*iter)->identifier != selection->identifier;
             ++iter, ++selection_idx_in_level);

    // Calculate number of children to hide from start and from end
    std::size_t nr_items_removed_at_the_beginning = 0;
    std::size_t nr_items_removed_at_the_end = 0;
    std::tie(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end) =
        paginate(nr_children, window_height - 1, selection_idx_in_level);

    // Skip invisible children
    std::size_t counter = 0;
    auto iter = node->children.rbegin();
    for (;iter != node->children.rend() and counter < nr_items_removed_at_the_end;
             ++iter, ++counter);

    // Iterate on visible children and push them to stack
    std::size_t nr_items_to_print = nr_children - nr_items_removed_at_the_beginning
        - nr_items_removed_at_the_end;
    for (counter = 0; counter < nr_items_to_print and iter != node->children.rend();
             ++counter, ++iter) {
        _dfs_stack.push(std::make_pair(*iter, child_depth));
    }

    // Update the next printed (_next_printed_node_after_selected), in case
    // selected node is the last printed node (in which case it won't be
    // updated in update_mid_dfs_state)
    const bool is_selecting_last_item = selection_idx_in_level >=
        nr_items_removed_at_the_beginning + nr_items_to_print - 1;
    if (is_selecting_last_item) {
        std::size_t next_item_idx = selection_idx_in_level + 1;
        const bool next_item_exists = next_item_idx < nr_children;
        if (next_item_exists) {
            _next_printed_node_after_selected = node->children[selection_idx_in_level + 1];
        }
    }
}
// --
//
#define HORIZONTAL_TREE_LINE ("\xe2\x94\x80")
// |
// |
// |
#define VERTICAL_TREE_LINE ("\xe2\x94\x82")
// |
// |--
// |
#define MIDDLE_CHILD_CONNECTOR ("\xe2\x94\x9c")
// |
// |--
//
#define LAST_CHILD_CONNECTOR  ("\xe2\x94\x94")

template <typename T>
void TreePrinter<T>::print_node(node_t node,
                                node_t previous_node,
                                int depth,
                                int previous_depth,
                                std::ostream &out,
                                std::shared_ptr<treelib::Node<T>> selection,
                                node_t _printed_subtree_root) {

    // Before printing, maintain the array of depth to -
    // [whether a next (=not printed yet) sibling exists]
    // in order to know whether to print connecting lines to next sibling of that
    // depth (node depth corresponds to indentation level), if siblint such exists
    update_depth_to_next_sibling_map(previous_depth, depth, previous_node);

    // Print selection marker if node is selected node
    const bool is_selection = node == selection;
    if (is_selection) {
        out << ">";
    } else {
        out << " ";
    }
    if (node->identifier != _printed_subtree_root->identifier) {
        out << " ";
    }

    // Print connecting lines for each depth (node depth is manifested by the indentation
    // level
    for (int depth_idx = 1; depth_idx < depth; ++depth_idx) {
        if (depth_to_next_sibling[depth_idx])
            out << VERTICAL_TREE_LINE << "   ";
        else
            out << "    ";
    }
    // Connecting lines special case - if node is last node in level
    if (node->identifier != _printed_subtree_root->identifier) {
        const auto is_last_child = info.m_node_to_next_sibling_existance[node->identifier];
        if (is_last_child)
            out << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            out << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }

    // Print node text
    out << " " << node->tag << std::endl;
}

template<typename T>
void TreePrinter<T>::choose_printed_tree_root(std::shared_ptr<treelib::Node<T>> selection,
                                              int window_height) {
    // Update state regarding printed subtree root
    _nr_levels_that_fit_in_window = _tree_window_fitter.get_nr_levels_that_fit_in_window(
        _tree,
        window_height,
        _printed_subtree_root);
    _printed_subtree_root_depth = _tree.get_node_depth(_printed_subtree_root);

    // Adjust printed subtree root to make sure selection is visible
    // Check if selection is within visible (printed) range. If not,
    // change printed subtree root to parent of selection
    int max_visible_depth = _printed_subtree_root_depth + _nr_levels_that_fit_in_window - 1;
    if (_selection_depth > max_visible_depth
        or _selection_depth <= _printed_subtree_root_depth) {
        _printed_subtree_root = _tree.get_node(selection->parent);
        _printed_subtree_root_depth = _tree.get_node_depth(_printed_subtree_root);
        _nr_levels_that_fit_in_window = _tree_window_fitter.get_nr_levels_that_fit_in_window(
            _tree,
            window_height,
            _printed_subtree_root);
    }
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_next_printed_node_after_selected(void) {
    return _next_printed_node_after_selected;
}

template<typename T>
std::shared_ptr<treelib::Node<T>> TreePrinter<T>::get_printed_node_before_selected(void) {
    return _printed_node_before_selected;
}

} // namespace treelib
#endif
