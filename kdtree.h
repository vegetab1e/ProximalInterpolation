#pragma once

#include <cmath>

#include <queue>
#include <vector>
#include <memory>
#include <utility>

#include <algorithm>

#include <iostream>

#include "utils.h"

template<class>
class KdTree;

template<class Item>
std::ostream& operator<<(std::ostream&, const KdTree<Item>&);

template<class Item>
class KdTree final
{
    friend
    std::ostream& operator<< <>(std::ostream& out, const KdTree& tree);

    struct Node
    {
        explicit Node(Item&& item,
                      std::shared_ptr<Node>&& left = nullptr,
                      std::shared_ptr<Node>&& right = nullptr) noexcept
            : item_(std::move(item))
            , left_(std::move(left))
            , right_(std::move(right))
        {
        }

        Item item_;
        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
    };

    using Pair = std::pair<decltype(std::declval<Item>().getDistance(std::declval<Item>())), const Item*>;

    using Container = std::vector<Pair>;

    struct CompareLess
    {
        bool operator()(const Pair& lhs,
                        const Pair& rhs) noexcept
        {
            return lhs.first < rhs.first;
        }
    };

    using PriorityQueue = std::priority_queue<Pair, Container, CompareLess>;

public:
    KdTree(std::vector<Item>&& items) noexcept;

    KdTree(const KdTree&) = delete;
    KdTree(KdTree&&) = default;
    
    KdTree& operator=(const KdTree&) = delete;
    KdTree& operator=(KdTree&&) = default;

    bool isEmpty() const noexcept;

    bool insert(Item&& item
#ifndef ALLOW_DUPLICATE_POINTS
                , bool update = false
#endif
                ) noexcept;

    bool remove(const Item& item) noexcept;

    std::vector<Item> searchNeighbors(const Item& item,
                                      std::size_t num_neighbors,
                                      bool reverse_search) const;

    std::vector<Item> shepardInterpolation(Item& item,
                                           std::size_t num_neighbors,
                                           bool reverse_search,
                                           double idw_power) const;

private:
    std::shared_ptr<Node> buildTree(std::vector<Item>&& items,
                                    std::size_t depth) const;

    void printTree(std::ostream& out,
                   const Node* node,
                   std::size_t depth) const;

    bool insertItem(std::shared_ptr<Node>& node,
                    Item&& item,
                    std::size_t depth
#ifndef ALLOW_DUPLICATE_POINTS
                    , bool update = false
#endif
                    );

    void getMinItem(const Node* node,
                    const Item*& min_item,
                    std::size_t depth) const;

    bool removeItem(std::shared_ptr<Node>& node,
                    const Item& item,
                    std::size_t depth);

    void directSearchNeighbors(const Item& item,
                               const Node* node,
                               std::size_t depth,
                               std::size_t num_neighbors,
                               PriorityQueue& neighbors) const;

    void reverseSearchNeighbors(const Item& item,
                                const Node* node,
                                std::size_t depth,
                                std::size_t num_neighbors,
                                PriorityQueue& neighbors) const;

    std::shared_ptr<Node> root_;
};


template<class Item>
KdTree<Item>::KdTree(std::vector<Item>&& items) noexcept
{
    try
    {
        root_ = buildTree(std::move(items), 0);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

template<class Item>
bool KdTree<Item>::isEmpty() const noexcept
{
    return !root_;
}

template<class Item>
bool KdTree<Item>::insert(Item&& item
#ifndef ALLOW_DUPLICATE_POINTS
                          , bool update
#endif
                          ) noexcept
try
{
    return insertItem(root_,
                      std::move(item),
                      0
#ifndef ALLOW_DUPLICATE_POINTS
                      , update
#endif
                      );
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;

    return false;
}

template<class Item>
bool KdTree<Item>::remove(const Item& item) noexcept
try
{
    return removeItem(root_,
                      item,
                      0);
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;

    return false;
}

template<class Item>
std::vector<Item> KdTree<Item>::searchNeighbors(const Item& item,
                                                std::size_t num_neighbors,
                                                bool reverse_search) const
{
    if (!root_ || !num_neighbors)
        return {};

    Container container;
    container.reserve(num_neighbors);

    PriorityQueue neighbors{CompareLess(), std::move(container)};
    if (reverse_search)
        reverseSearchNeighbors(item, root_.get(), 0, num_neighbors, neighbors);
    else
        directSearchNeighbors(item, root_.get(), 0, num_neighbors, neighbors);

    std::vector<Item> out;
    out.reserve(neighbors.size());
    while (!neighbors.empty())
    {
        out.push_back(*neighbors.top().second);
        neighbors.pop();
    }

    return out;
}

template<class Item>
std::vector<Item> KdTree<Item>::shepardInterpolation(Item& item,
                                                     std::size_t num_neighbors,
                                                     bool reverse_search,
                                                     double idw_power) const
{
    if (!root_ || !num_neighbors)
        return {};

    Container container;
    container.reserve(num_neighbors);

    PriorityQueue neighbors{CompareLess(), std::move(container)};
    if (reverse_search)
        reverseSearchNeighbors(item, root_.get(), 0, num_neighbors, neighbors);
    else
        directSearchNeighbors(item, root_.get(), 0, num_neighbors, neighbors);

    std::vector<Item> out;
    out.reserve(neighbors.size());

    long double num = 0.0L, den = 0.0L;
    while (!neighbors.empty())
    {
        const auto& neighbor = neighbors.top();

#ifdef ZERO_DISTANCE_HANDLING
        if (isZero(neighbor.first)) [[unlikely]]
        {
            item.setValue(neighbor.second->getValue());

            return {*neighbor.second};
        }

        const auto weight = 1.0 / std::pow(neighbor.first, idw_power);
#else
        const auto weight = 1.0 / std::pow(isZero(neighbor.first) ? EPSILON<decltype(neighbor.first)>
                                                                  : neighbor.first,
                                           idw_power);
#endif
        num += weight * neighbor.second->getValue();
        den += weight;

        out.push_back(*neighbor.second);
        neighbors.pop();
    }

    item.setValue(num / den);

    return out;
}

template<class Item>
std::ostream& operator<<(std::ostream& out, const KdTree<Item>& tree)
{
    if (!tree.root_)
        return out << "The tree is empty.\n";

    out << "\x1b[1;41mKdTree:\x1b[0m\n";
    tree.printTree(out, tree.root_.get(), 0);

    return out;
}

template<class Item>
std::shared_ptr<typename KdTree<Item>::Node> KdTree<Item>::buildTree(std::vector<Item>&& items,
                                                                     std::size_t depth) const
{
    if (items.empty())
        return nullptr;

    if (items.size() == 1)
        return std::make_shared<Node>(std::move(items[0]));

    std::sort(items.begin(), items.end(),
              [dimension = depth % Item::getNumAxes()](const auto& lhs, const auto& rhs)->bool{
                  return lhs.compareLess(rhs, dimension);});

    const auto median = items.size() / 2;
    return std::make_shared<Node>(std::move(items[median]),
                                  buildTree({items.begin(), items.begin() + median}, depth + 1),
                                  buildTree({items.begin() + median + 1, items.end()}, depth + 1));
}

template<class Item>
void KdTree<Item>::printTree(std::ostream& out,
                             const Node* node,
                             std::size_t depth) const
{
    if (!node)
        return;

    printTree(out, node->left_.get(), depth + 1);

    out << "\x1b[1;31m" << depth << "\x1b[0m\t"
        << "\x1b[1;32m" << node->item_ << "\x1b[0m\n";

    printTree(out, node->right_.get(), depth + 1);
}

template<class Item>
bool KdTree<Item>::insertItem(std::shared_ptr<Node>& node,
                              Item&& item,
                              std::size_t depth
#ifndef ALLOW_DUPLICATE_POINTS
                              , bool update
#endif
                              )
{
    if (!node)
    {
        node = std::make_shared<Node>(std::move(item));
        
        return true;
    }

#ifndef ALLOW_DUPLICATE_POINTS
    if (item.compareEqual(node->item_))
    {
        if (update)
            node->item_.setValue(item);
        
        return false;
    }
#endif

    if (item.compareLess(node->item_, depth % Item::getNumAxes()))
        return insertItem(node->left_, std::move(item), depth + 1);
    
    return insertItem(node->right_, std::move(item), depth + 1);
}

template<class Item>
void KdTree<Item>::getMinItem(const Node* node,
                              const Item*& min_item,
                              std::size_t dimension) const
{
    if (node->left_)
        getMinItem(node->left_.get(), min_item, dimension);

    if (not min_item or node->item_.compareLess(*min_item, dimension))
        min_item = &node->item_;

    if (node->right_)
        getMinItem(node->right_.get(), min_item, dimension);
}

template<class Item>
bool KdTree<Item>::removeItem(std::shared_ptr<Node>& node,
                              const Item& item,
                              std::size_t depth)
{
    if (!node)
        return false;

    if (item.compareEqual(node->item_))
    {
        // BST для демонстрации отличий от KdT.
        if constexpr (Item::getNumAxes() == 1)
        {
            if (!node->left_ && !node->right_)
            {
                node = nullptr;
            }
            else if (!node->left_)
            {
                node = std::move(node->right_);
            }
            else if (!node->right_)
            {
                node = std::move(node->left_);
            }
            else
            {
                auto min_node = &node->right_;
                while ((*min_node)->left_)
                    min_node = &(*min_node)->left_;

                node->item_ = std::move((*min_node)->item_);

                *min_node = std::move((*min_node)->right_);
            }
        }
        else
        {
            if (!node->left_ && !node->right_)
            {
                node = nullptr;
            }
            else if (node->right_)
            {
                const Item* min_item = nullptr;
                getMinItem(node->right_.get(), min_item, depth % Item::getNumAxes());

                node->item_ = *min_item;

                removeItem(node->right_, *min_item, depth + 1);
            }
            else
            {
                const Item* min_item = nullptr;
                getMinItem(node->left_.get(), min_item, depth % Item::getNumAxes());

                node->item_ = *min_item;

                removeItem(node->left_, *min_item, depth + 1);

                node->right_ = std::move(node->left_);
            }
        }

        return true;
    }

    if (item.compareLess(node->item_, depth % Item::getNumAxes()))
        return removeItem(node->left_, item, depth + 1);
    
    return removeItem(node->right_, item, depth + 1);
}

template<class Item>
void KdTree<Item>::directSearchNeighbors(const Item& item,
                                         const Node* node,
                                         std::size_t depth,
                                         std::size_t num_neighbors,
                                         PriorityQueue& neighbors) const
{
    if (!node)
        return;

    const auto distance = item.getDistance(node->item_);
    if (neighbors.size() < num_neighbors)
    {
        neighbors.push({distance, &node->item_});
    }
    else if (distance < neighbors.top().first)
    {
        neighbors.pop();
        neighbors.push({distance, &node->item_});
    }

    const auto dimension = depth % Item::getNumAxes();
    decltype(node) next_node, auxiliary_node;
    if (item.compareLess(node->item_, dimension))
    {
        next_node = node->left_.get();
        auxiliary_node = node->right_.get();
    }
    else
    {
        next_node = node->right_.get();
        auxiliary_node = node->left_.get();
    }
        
    directSearchNeighbors(item, next_node, depth + 1, num_neighbors, neighbors);

    if (neighbors.size() < num_neighbors)
    {
        directSearchNeighbors(item, auxiliary_node, depth + 1, num_neighbors, neighbors);
    }
    else
    {
        const auto distance = static_cast<decltype(neighbors.top().first)>(item.getDistance(node->item_, dimension));
        if (std::fabs(distance) < neighbors.top().first)
            directSearchNeighbors(item, auxiliary_node, depth + 1, num_neighbors, neighbors);
    }
}

template<class Item>
void KdTree<Item>::reverseSearchNeighbors(const Item& item,
                                          const Node* node,
                                          std::size_t depth,
                                          std::size_t num_neighbors,
                                          PriorityQueue& neighbors) const
{
    const auto distance = item.getDistance(node->item_);
    if (!node->left_ && !node->right_)
    {
        if (neighbors.size() < num_neighbors)
        {
            neighbors.push({distance, &node->item_});
        }
        else if (distance < neighbors.top().first)
        {
            neighbors.pop();
            neighbors.push({distance, &node->item_});
        }

        return;
    }

    const auto dimension = depth % Item::getNumAxes();
    decltype(node) next_node, auxiliary_node = nullptr;
    if (!node->left_)
    {
        next_node = node->right_.get();
    }
    else if (!node->right_)
    {
        next_node = node->left_.get();
    }
    else if (item.compareLess(node->item_, dimension))
    {
        next_node = node->left_.get();
        auxiliary_node = node->right_.get();
    }
    else
    {
        next_node = node->right_.get();
        auxiliary_node = node->left_.get();
    }

    reverseSearchNeighbors(item, next_node, depth + 1, num_neighbors, neighbors);

    if (neighbors.size() < num_neighbors)
    {
        neighbors.push({distance, &node->item_});
    }
    else if (distance < neighbors.top().first)
    {
        neighbors.pop();
        neighbors.push({distance, &node->item_});
    }

    if (!auxiliary_node)
        return;

    if (neighbors.size() < num_neighbors)
    {
        reverseSearchNeighbors(item, auxiliary_node, depth + 1, num_neighbors, neighbors);
    }
    else
    {
        const auto distance = static_cast<decltype(neighbors.top().first)>(item.getDistance(node->item_, dimension));
        if (std::fabs(distance) < neighbors.top().first)
            reverseSearchNeighbors(item, auxiliary_node, depth + 1, num_neighbors, neighbors);
    }
}
