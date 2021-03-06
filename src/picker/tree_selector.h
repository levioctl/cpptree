#ifndef __TREESELECTOR_H
#define __TREESELECTOR_H

#include "tree/tree.h"
#include "treeprinter/treeprinter.h"

namespace picker {

template <typename T>
class TreeSelector
{
public:
    using NodePtr = typename treelib::Tree<T>::node_t;

    TreeSelector(treelib::Tree<T>& tree, treelib::TreePrinter<T>& tree_printer);

    NodePtr get_selection(void);

    void explore_children_of_selection(void);
    void move_to_next(void);
    void move_to_prev(void);
    void move_one_up(void);

private:
    treelib::Tree<T>& _tree;
    treelib::TreePrinter<T>& _tree_printer;
    NodePtr _selection;

    void _advance_selection_at_same_tree_level(int direction);
    void select_node(std::shared_ptr<treelib::Node<T>> node);
};

template<typename T>
TreeSelector<T>::TreeSelector(treelib::Tree<T>& tree, treelib::TreePrinter<T>& tree_printer) :
    _tree(tree),
    _tree_printer(tree_printer),
    _selection(tree.get_root())
{
}

template<typename T>
typename TreeSelector<T>::NodePtr TreeSelector<T>::get_selection(void) {
    return _selection;
}

template<typename T>
void TreeSelector<T>::explore_children_of_selection(void) {
    if (_selection == nullptr) {
    } else if (_selection->children.size() == 0) {
    } else {
        _selection = _selection->children[0];
        if ((not (_selection)->is_matching_search
            and not (_selection)->is_ancestor_of_matching_search)) {
            _advance_selection_at_same_tree_level(1);
        }
    }
}

//void TreeSelector<T>::go_next_until_reaching_a_matching_node(void) {
//    auto position = _selection->children.begin();
//    for (; position != _selection->children.end() &&
//            (not (*position)->is_matching_search
//             and not (*position)->is_ancestor_of_matching_search);
//            ++position);
//
//    if (position != _selection->children.end()) {
//        _selection = (*position);
//    }
//}

template<typename T>
void TreeSelector<T>::move_one_up(void) {
    if (nullptr == _selection) {
        return;
    }
    auto parent = _tree.get_node(_selection->parent);
    select_node(parent);
}

template<typename T>
void TreeSelector<T>::move_to_next(void) {
    auto next_node = _tree_printer.get_next_printed_node_after_selected();
    select_node(next_node);
}

template<typename T>
void TreeSelector<T>::move_to_prev(void) {
    auto prev_node = _tree_printer.get_printed_node_before_selected();
    select_node(prev_node);
}

template<typename T>
void TreeSelector<T>::_advance_selection_at_same_tree_level(int difference) {
    if (nullptr == _selection.get()) {
        select_node(nullptr);
        return;
    }
    if (_tree.get_root() == _selection) {
        select_node(nullptr);
        return;
    }
    NodePtr parent = _tree.get_node(_selection->parent);
    if (parent.get() == nullptr) {
        select_node(nullptr);
        return;
    }
    auto& children = parent->children;
    auto position = children.begin();
    // Find the index of the current selection in the array of paren'ts children
    // TODO: optimize this (to something better than O(n))
    for (; position != children.end() &&
        _selection->identifier != (*position)->identifier; ++position);
    assert(position != children.end());

    // Advance
    int direction = 0;
    if (difference > 0) {
        direction = 1;
    } else if (difference < 0) {
        direction = -1;
    }
    int nr_steps_to_take = abs(difference);
    int nr_steps_taken = 0;
    auto target = position;
    while (nr_steps_taken < nr_steps_to_take) {
        target += direction;
        if (target == children.end()) {
            break;
        } else if (position == children.begin() and direction == -1) {
            break;
        } else if ((*target)->is_matching_search or (*target)->is_ancestor_of_matching_search) {
            position = target;
            ++nr_steps_taken;
        }
    }

    // Set selection
    select_node(*position);
}

template<typename T>
void TreeSelector<T>::select_node(std::shared_ptr<treelib::Node<T>> node) {
    const bool is_new_selection_valid = node != nullptr
            and _tree_printer.was_selection_printed();
    if (is_new_selection_valid) {
        _selection = node;
    } else {
        auto first_printed_node = _tree_printer.get_first_printed_node();
        if (first_printed_node) {
            _selection = first_printed_node;
        }
    }
}

}

#endif
