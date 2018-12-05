#include "assert.h"
#include "gtest/gtest.h"

#include "tree/tree.h"
#include "tree/pre_print_tree_analysis.h"
#include "common.h"

TEST(pre_print_tree_analysis, simple_tree_sibling_existance_map) {
    treelib::Tree<int> tree = get_simple_tree_informative_nodes();
    auto root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_node_to_next_sibling_existance["child1"]);
    assert(info.m_node_to_next_sibling_existance["child1.1"]);
    assert(not info.m_node_to_next_sibling_existance["child1.2"]);
    assert(not info.m_node_to_next_sibling_existance["root"]);
    assert(not info.m_node_to_next_sibling_existance["child2"]);
    assert(not info.m_node_to_next_sibling_existance["child2.1"]);
}

TEST(pre_print_tree_analysis, test_simple_tree_max_depth) {
    treelib::Tree<int> tree = get_simple_tree_informative_nodes();
    auto root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(info.m_max_depth == 2);
}

TEST(pre_print_tree_analysis, test_info_not_copied) {
    treelib::Tree<int> tree = get_simple_tree_informative_nodes();
    auto root = tree.get_root();
    treelib::TreeAnalysisInfo info = treelib::analyze_tree_for_printing(root);
    assert(not info.m_copied);
}
