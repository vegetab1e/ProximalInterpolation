#pragma once

#include <cmath>

#include <queue>
#include <vector>
#include <memory>
#include <utility>
#include <type_traits>

#include <algorithm>

#include <ostream>

#include <exception>

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
        Node(Item&& item,
             std::size_t depth,
             std::shared_ptr<Node>&& left = nullptr,
             std::shared_ptr<Node>&& right = nullptr) noexcept;

        Node(const std::shared_ptr<Node>& node,
             std::shared_ptr<Node>&& left,
             std::shared_ptr<Node>&& right) noexcept;

        static auto getComparator(std::size_t depth) noexcept;

        template<class Type>
        static Type getDistance(const Item& item, const Node* node)
        noexcept(noexcept(item.getDistance(node->item, node->dimension)))
        // Если вынести определение из тела, то noexcept это ^^^^^^^^^ не
        // сможет видеть, потому что Node - закрытый (!) вложенный класс.
        // Для decltype(KdTree<Item>::Node::dimension) будет то же самое.
        {
            decltype(auto) distance = item.getDistance(node->item, node->dimension);

            if constexpr (not std::is_same_v<Type, decltype(distance)>)
                return static_cast<Type>(distance);
            else
                return distance;
        }

        static auto getDistance(const Item& item, const Node* node)
        noexcept(noexcept(std::declval<Item>().getDistance(std::declval<Item>())));

        static bool compareEqual(const Item& item,
                                 const std::shared_ptr<Node>& node) noexcept;

        // node может получиться ссылкой на константный указатель на
        // константу: const Node * const & - это нормально. Главное,
        // чтобы не получился std::shared_ptr<Node>, т.е. изменяемый
        // и по значению умный указатель, именно в этом был замысел.
        static bool compareLess(const Item& item, const auto& node);

        bool compareLess(const Item* item, std::size_t dimension) const;

        void setValue(const Item& item) noexcept;

        bool isLeaf() const noexcept;

        Item item;
        const std::size_t dimension;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
    };

    // Nearest Neighbors Search
    // Session Properties
    struct NnsSessProps
    {
        // Вообще это необязательное условие, не более чем для
        // повышения эффективности работы с контейнерами STL.
        static_assert(std::is_trivially_destructible_v<Item>);

        using Pair = std::pair<decltype(std::declval<Item>().getDistance(std::declval<Item>())),
                               const Item*>;

        using Container = std::vector<Pair>;

        struct CompareLess
        {
            bool operator()(const Pair& lhs,
                            const Pair& rhs) const noexcept
            {
                return lhs.first < rhs.first;
            }
        };

        using PriorityQueue = std::priority_queue<Pair, Container, CompareLess>;

        NnsSessProps(const Item& item,
                     std::size_t num_neighbors);

        NnsSessProps(const NnsSessProps&) = delete;
        NnsSessProps& operator=(const NnsSessProps&) = delete;

        auto makeQueue();

        void updateQueue(const Node* node);

        bool isAuxRequired(const Node* node) const;

        const Item& item;
        const std::size_t num_neighbors;
        PriorityQueue neighbors;
    };

public:
    KdTree() = default;

    KdTree(std::vector<Item>&& items) noexcept;

    KdTree(const KdTree&) noexcept;
    KdTree(KdTree&&) noexcept;
    
    KdTree& operator=(const KdTree&) noexcept;
    KdTree& operator=(KdTree&&) noexcept;

    bool isEmpty() const noexcept;

    bool isBusy() const noexcept;

    bool insert(Item&& item
#ifndef ALLOW_DUPLICATE_POINTS
                , bool update = false
#endif
                ) noexcept;

    bool remove(const Item& item) noexcept;

    std::vector<Item> neighborsSearch(const Item& item,
                                      std::size_t num_neighbors,
                                      bool reverse_search) const;

    std::vector<Item> shepardInterpolation(Item& item,
                                           std::size_t num_neighbors,
                                           bool reverse_search,
                                           double idw_power) const;

private:
    std::shared_ptr<Node> buildTree(std::vector<Item>&& items,
                                    std::size_t depth) const;

    std::shared_ptr<Node> copyTree(const std::shared_ptr<Node>& node) const noexcept;

    void printTree(std::ostream& out,
                   const std::shared_ptr<Node>& node,
                   std::size_t depth) const;

    bool insertItem(std::shared_ptr<Node>& node,
                    Item&& item,
                    std::size_t depth
#ifndef ALLOW_DUPLICATE_POINTS
                    , bool update = false
#endif
                    );

    void getMinItem(const std::shared_ptr<Node>& node,
                    const Item*& min_item,
                    std::size_t dimension) const;

    bool removeItem(std::shared_ptr<Node>& node,
                    const Item& item);

    void forwardSearch(const Node* node) const;

    void reverseSearch(const Node* node) const;

    std::shared_ptr<Node> root_;
    mutable std::unique_ptr<NnsSessProps> search_session_;
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
KdTree<Item>::KdTree(const KdTree& tree) noexcept
    : root_(copyTree(tree.root_))
{
}

template<class Item>
KdTree<Item>::KdTree(KdTree&& tree) noexcept
{
    if (tree.search_session_)
        root_ = copyTree(tree.root_);
    else
        root_ = std::move(tree.root_);
}

template<class Item>
KdTree<Item>& KdTree<Item>::operator=(const KdTree& tree) noexcept
{
    root_ = copyTree(tree.root_);

    return *this;
}

template<class Item>
KdTree<Item>& KdTree<Item>::operator=(KdTree&& tree) noexcept
{
    if (tree.search_session_)
        root_ = copyTree(tree.root_);
    else
        root_ = std::move(tree.root_);

    return *this;
}

template<class Item>
bool KdTree<Item>::isEmpty() const noexcept
{
    return !root_;
}

template<class Item>
bool KdTree<Item>::isBusy() const noexcept
{
    return !!search_session_;
}

template<class Item>
bool KdTree<Item>::insert(Item&& item
#ifndef ALLOW_DUPLICATE_POINTS
                          , bool update
#endif
                          ) noexcept
try
{
    if (search_session_)
        return false;

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
    if (not root_
        or search_session_)
        return false;

    return removeItem(root_, item);
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;

    return false;
}

template<class Item>
std::vector<Item> KdTree<Item>::neighborsSearch(const Item& item,
                                                std::size_t num_neighbors,
                                                bool reverse_search) const
{
    if (not root_
        or search_session_
        or num_neighbors == 0)
        return {};

    search_session_ = std::make_unique<NnsSessProps>(item, num_neighbors);

    try
    {
        if (reverse_search)
            reverseSearch(root_.get());
        else
            forwardSearch(root_.get());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        search_session_.reset();

        return {};
    }

    auto& neighbors = search_session_->neighbors;

    std::vector<Item> out;
    out.reserve(neighbors.size());
    while (!neighbors.empty())
    {
        out.push_back(*neighbors.top().second);
        neighbors.pop();
    }

    search_session_.reset();

    return out;
}

template<class Item>
std::vector<Item> KdTree<Item>::shepardInterpolation(Item& item,
                                                     std::size_t num_neighbors,
                                                     bool reverse_search,
                                                     double idw_power) const
{
    if (not root_
        or search_session_
        or num_neighbors == 0)
        return {};

    search_session_ = std::make_unique<NnsSessProps>(item, num_neighbors);

    try
    {
        if (reverse_search)
            reverseSearch(root_.get());
        else
            forwardSearch(root_.get());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;

        search_session_.reset();

        return {};
    }

    auto& neighbors = search_session_->neighbors;

    std::vector<Item> out;
    out.reserve(neighbors.size());

    long double num = 0.0L, den = 0.0L;
    while (!neighbors.empty())
    {
        const auto& neighbor = neighbors.top();

#ifdef ZERO_DISTANCE_HANDLING
        if (isZero(neighbor.first)) [[unlikely]]
        {
            out.clear();
            out.push_back(*neighbor.second);

            item.setValue(neighbor.second->getValue());

            search_session_.reset();

            return out;
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

    search_session_.reset();

    return out;
}

template<class Item>
std::shared_ptr<typename KdTree<Item>::Node>
KdTree<Item>::buildTree(std::vector<Item>&& items,
                        std::size_t depth) const
{
    if (items.empty())
        return nullptr;

    if (items.size() == 1)
        return std::make_shared<Node>(std::move(items[0]), depth);

    std::sort(items.begin(), items.end(), Node::getComparator(depth));

    const auto median = items.size() / 2;
    return std::make_shared<Node>(std::move(items[median]), depth,
                                  buildTree({items.begin(), items.begin() + median}, depth + 1),
                                  buildTree({items.begin() + median + 1, items.end()}, depth + 1));
}

template<class Item>
std::shared_ptr<typename KdTree<Item>::Node>
KdTree<Item>::copyTree(const std::shared_ptr<Node>& node) const noexcept
{
    if (!node)
        return nullptr;

    return std::make_shared<Node>(node,
                                  copyTree(node->left),
                                  copyTree(node->right));
}

template<class Item>
void KdTree<Item>::printTree(std::ostream& out,
                             const std::shared_ptr<Node>& node,
                             std::size_t depth) const
{
    if (node->left)
        printTree(out, node->left, depth + 1);

    out << "\x1b[1;31m" << depth << "\x1b[0m\t"
        << "\x1b[1;32m" << node->item << "\x1b[0m\n";

    if (node->right)
        printTree(out, node->right, depth + 1);
}

template<class Item>
std::ostream& operator<<(std::ostream& out, const KdTree<Item>& tree)
{
    if (!tree.root_)
        return out << "The tree is empty.\n";

    out << "\x1b[1;41mKdTree:\x1b[0m\n";
    tree.printTree(out, tree.root_, 0);

    return out;
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
        node = std::make_shared<Node>(std::move(item), depth);
        
        return true;
    }

#ifndef ALLOW_DUPLICATE_POINTS
    if (Node::compareEqual(item, node))
    {
        if (update)
            node->setValue(item);
        
        return false;
    }
#endif

    if (Node::compareLess(item, node))
        return insertItem(node->left, std::move(item), depth + 1);
    
    return insertItem(node->right, std::move(item), depth + 1);
}

template<class Item>
void KdTree<Item>::getMinItem(const std::shared_ptr<Node>& node,
                              const Item*& min_item,
                              std::size_t dimension) const
{
    if (node->left)
        getMinItem(node->left, min_item, dimension);

    if (not min_item or node->compareLess(min_item, dimension))
        min_item = &node->item;

    if (node->right and node->dimension != dimension)
        getMinItem(node->right, min_item, dimension);
}

template<class Item>
bool KdTree<Item>::removeItem(std::shared_ptr<Node>& node,
                              const Item& item)
{
    if (!node)
        return false;

    if (Node::compareEqual(item, node))
    {
        // BST для демонстрации отличий от KdT
        if constexpr (Item::getNumAxes() == 1)
        {
            if (node->isLeaf())
            {
                node = nullptr;
            }
            else if (!node->left)
            {
                node = std::move(node->right);
            }
            else if (!node->right)
            {
                node = std::move(node->left);
            }
            else
            {
                auto min_node = &node->right;
                while ((*min_node)->left)
                    min_node = &(*min_node)->left;

                node->item = std::move((*min_node)->item);
                // Работает благодаря идиоме move-and-swap
                *min_node = std::move((*min_node)->right);
            }
        }
        else
        {
            if (node->isLeaf())
            {
                node = nullptr;
            }
            else
            {
                if (!node->right)
                    node->right = std::move(node->left);

                const Item* min_item = nullptr;
                getMinItem(node->right, min_item, node->dimension);

                node->item = *min_item;

                removeItem(node->right, *min_item);
            }
        }

        return true;
    }

    if (Node::compareLess(item, node))
        return removeItem(node->left, item);
    
    return removeItem(node->right, item);
}

template<class Item>
void KdTree<Item>::forwardSearch(const Node* node) const
{
    search_session_->updateQueue(node);

    decltype(node) next_node, aux_node;
    if (Node::compareLess(search_session_->item, node))
    {
        next_node = node->left.get();
        aux_node = node->right.get();
    }
    else
    {
        next_node = node->right.get();
        aux_node = node->left.get();
    }

    if (next_node)
        forwardSearch(next_node);

    if (aux_node && search_session_->isAuxRequired(node))
        forwardSearch(aux_node);
}

template<class Item>
void KdTree<Item>::reverseSearch(const Node* node) const
{
    if (node->isLeaf())
    {
        search_session_->updateQueue(node);

        return;
    }

    decltype(node) next_node, aux_node = nullptr;
    if (!node->left)
    {
        next_node = node->right.get();
    }
    else if (!node->right)
    {
        next_node = node->left.get();
    }
    else if (Node::compareLess(search_session_->item, node))
    {
        next_node = node->left.get();
        aux_node = node->right.get();
    }
    else
    {
        next_node = node->right.get();
        aux_node = node->left.get();
    }

    reverseSearch(next_node);

    search_session_->updateQueue(node);

    if (aux_node && search_session_->isAuxRequired(node))
        reverseSearch(aux_node);
}


template<class Item>
KdTree<Item>::Node::Node(Item&& item,
                         std::size_t depth,
                         std::shared_ptr<Node>&& left,
                         std::shared_ptr<Node>&& right) noexcept
    : item(std::move(item))
    , dimension(depth % Item::getNumAxes())
    , left(std::move(left))
    , right(std::move(right))
{
}

template<class Item>
KdTree<Item>::Node::Node(const std::shared_ptr<Node>& node,
                         std::shared_ptr<Node>&& left,
                         std::shared_ptr<Node>&& right) noexcept
    : item(node->item)
    , dimension(node->dimension)
    , left(std::move(left))
    , right(std::move(right))
{
}

template<class Item>
auto KdTree<Item>::Node::getComparator(std::size_t depth) noexcept
{
    return [dimension = depth % Item::getNumAxes()](const Item& lhs, const Item& rhs)->bool{
               return lhs.compareLess(rhs, dimension); };
}

template<class Item>
auto KdTree<Item>::Node::getDistance(const Item& item, const Node* node)
noexcept(noexcept(std::declval<Item>().getDistance(std::declval<Item>())))
{
    // Написать так в noexcept не выйдет
    return item.getDistance(node->item);
    // из-за того, что он вот это ^^^^
    // вне тела вложенного закрытого (!)
    // класса Node не сможет видеть, как
    // и поле dimension, необходимое для
    // другого варианта этой же функции,
    // которая определена в теле класса.
}

template<class Item>
bool KdTree<Item>::Node::compareEqual(const Item& item,
                                      const std::shared_ptr<Node>& node) noexcept
{
    return item.compareEqual(node->item);
}

template<class Item>
bool KdTree<Item>::Node::compareLess(const Item& item, const auto& node)
{
    return item.compareLess(node->item, node->dimension);
}

template<class Item>
bool KdTree<Item>::Node::compareLess(const Item* item, std::size_t dimension) const
{
    return this->item.compareLess(*item, dimension);
}

template<class Item>
void KdTree<Item>::Node::setValue(const Item& item) noexcept
{
    this->item.setValue(item);
}

template<class Item>
bool KdTree<Item>::Node::isLeaf() const noexcept
{
    return (!left && !right);
}


template<class Item>
KdTree<Item>::NnsSessProps::NnsSessProps(const Item& item,
                                         std::size_t num_neighbors)
    : item(item)
    , num_neighbors(num_neighbors)
    , neighbors(makeQueue())
{
}

template<class Item>
auto KdTree<Item>::NnsSessProps::makeQueue()
{
    Container container;
    container.reserve(num_neighbors);

    return PriorityQueue{CompareLess(), std::move(container)};
}

template<class Item>
void KdTree<Item>::NnsSessProps::updateQueue(const Node* node)
{
    const auto distance = Node::getDistance(item, node);
    if (neighbors.size() < num_neighbors)
    {
        neighbors.push({distance, &node->item});
    }
    else if (distance < neighbors.top().first)
    {
        neighbors.pop();
        neighbors.push({distance, &node->item});
    }
}

template<class Item>
bool KdTree<Item>::NnsSessProps::isAuxRequired(const Node* node) const
{
    if (neighbors.size() < num_neighbors)
        return true;

    const auto distance = Node::template getDistance<decltype(neighbors.top().first)>(item, node);
    if (ABS_EX(distance) < neighbors.top().first)
        return true;

    return false;
}
