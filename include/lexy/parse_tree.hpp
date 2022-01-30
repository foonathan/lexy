// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/memory_resource.hpp>
#include <lexy/grammar.hpp>
#include <lexy/token.hpp>

//=== internal: pt_node ===//
namespace lexy::_detail
{
template <typename Reader>
struct pt_node;
template <typename Reader>
struct pt_node_token;
template <typename Reader>
struct pt_node_production;

template <typename Reader>
class pt_node_ptr
{
public:
    static constexpr auto type_token      = 0b0u;
    static constexpr auto type_production = 0b1u;

    static constexpr auto role_sibling = 0b0u;
    static constexpr auto role_parent  = 0b1u;

    // nullptr is a parent pointer to a non-existing parent.
    // This means that it is automatically an empty child range.
    pt_node_ptr() noexcept : pt_node_ptr(nullptr, type_token, role_parent) {}

    void set_sibling(pt_node<Reader>* ptr, unsigned type)
    {
        *this = pt_node_ptr(ptr, type, role_sibling);
    }
    void set_sibling(pt_node_token<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_token, role_sibling);
    }
    void set_sibling(pt_node_production<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_production, role_sibling);
    }

    void set_parent(pt_node_production<Reader>* ptr)
    {
        *this = pt_node_ptr(ptr, type_production, role_parent);
    }

    //=== access ===//
    explicit operator bool() const noexcept
    {
        return base() != nullptr;
    }

    unsigned type() const noexcept
    {
        return _value & 0b1;
    }

    auto* base() const noexcept
    {
        // NOLINTNEXTLINE: We need pointer conversion.
        return reinterpret_cast<pt_node<Reader>*>(_value & ~std::uintptr_t(0b11));
    }

    auto token() const noexcept
    {
        return type() == type_token ? static_cast<pt_node_token<Reader>*>(base()) : nullptr;
    }
    auto production() const noexcept
    {
        return type() == type_production ? static_cast<pt_node_production<Reader>*>(base())
                                         : nullptr;
    }

    bool is_sibling_ptr() const noexcept
    {
        return ((_value & 0b10) >> 1) == role_sibling;
    }
    bool is_parent_ptr() const noexcept
    {
        return ((_value & 0b10) >> 1) == role_parent;
    }

private:
    explicit pt_node_ptr(pt_node<Reader>* ptr, unsigned type, unsigned role)
    : _value(reinterpret_cast<std::uintptr_t>(ptr))
    {
        LEXY_PRECONDITION((reinterpret_cast<std::uintptr_t>(ptr) & 0b11) == 0);

        _value |= (role & 0b1) << 1;
        _value |= (type & 0b1);
    }

    std::uintptr_t _value;
};

template <typename Reader>
struct pt_node
{
    // Either points back to the next child of the parent node (the sibling),
    // or back to the parent node if it is its last child.
    // It is only null for the root node.
    pt_node_ptr<Reader> ptr;
};

template <typename Reader>
struct pt_node_token : pt_node<Reader>
{
    // If it's random access, we store size instead of end.
    static constexpr auto _optimize_end
        = _detail::is_random_access_iterator<typename Reader::iterator>;
    using _end_t
        // If we can optimize it, we store the size as a uint32_t, otherwise the iterator.
        = std::conditional_t<_optimize_end, std::uint_least32_t, typename Reader::iterator>;

    typename Reader::iterator begin;
    _end_t                    end_impl;
    ::uint_least16_t          kind;

    explicit pt_node_token(std::uint_least16_t kind, typename Reader::iterator begin,
                           typename Reader::iterator end) noexcept
    : begin(begin), kind(kind)
    {
        update_end(end);
    }

    typename Reader::iterator end() const noexcept
    {
        if constexpr (_optimize_end)
            return begin + end_impl;
        else
            return end_impl;
    }

    void update_end(typename Reader::iterator end) noexcept
    {
        if constexpr (_optimize_end)
        {
            static_assert(sizeof(pt_node_token) == 3 * sizeof(void*));

            auto size = std::size_t(end - begin);
            LEXY_PRECONDITION(size <= UINT_LEAST32_MAX);
            end_impl = std::uint_least32_t(size);
        }
        else
        {
            static_assert(sizeof(pt_node_token) <= 4 * sizeof(void*));

            end_impl = end;
        }
    }
};

template <typename Reader>
struct pt_node_production : pt_node<Reader>
{
    static constexpr std::size_t child_count_bits = sizeof(std::size_t) * CHAR_BIT - 3;

    const char* name;
    std::size_t child_count : child_count_bits;
    std::size_t token_production : 1;
    std::size_t first_child_adjacent : 1;
    std::size_t first_child_type : 1;

    template <typename Production>
    explicit pt_node_production(Production) noexcept
    : child_count(0), token_production(lexy::is_token_production<Production>),
      first_child_adjacent(true), first_child_type(pt_node_ptr<Reader>::type_token)
    {
        static_assert(sizeof(pt_node_production) == 3 * sizeof(void*));

        name = lexy::production_name<Production>();
    }

    pt_node_ptr<Reader> first_child()
    {
        auto memory = static_cast<void*>(this + 1);
        if (child_count == 0)
        {
            // We don't have a child at all.
            pt_node_ptr<Reader> result;
            result.set_parent(this);
            return result;
        }
        else if (first_child_adjacent)
        {
            // The first child is stored immediately afterwards.
            pt_node_ptr<Reader> result;
            result.set_sibling(static_cast<pt_node<Reader>*>(memory), first_child_type);
            return result;
        }
        else
        {
            // We're only storing a pointer to the first child immediately afterwards.
            return *static_cast<pt_node_ptr<Reader>*>(memory);
        }
    }
};
} // namespace lexy::_detail

//=== internal: pt_buffer ===//
namespace lexy::_detail
{
// Basic stack allocator to store all the nodes of a tree.
template <typename MemoryResource>
class pt_buffer
{
    using resource_ptr = _detail::memory_resource_ptr<MemoryResource>;

    static constexpr std::size_t block_size = 4096 - sizeof(void*);

    struct block
    {
        block*        next;
        unsigned char memory[block_size];

        static block* allocate(resource_ptr resource)
        {
            auto memory = resource->allocate(sizeof(block), alignof(block));
            auto ptr    = ::new (memory) block; // Don't initialize array!
            ptr->next   = nullptr;
            return ptr;
        }

        static block* deallocate(resource_ptr resource, block* ptr)
        {
            auto next = ptr->next;
            resource->deallocate(ptr, sizeof(block), alignof(block));
            return next;
        }

        unsigned char* end() noexcept
        {
            return &memory[block_size];
        }
    };

public:
    //=== constructors/destructors/assignment ===//
    explicit constexpr pt_buffer(MemoryResource* resource) noexcept
    : _resource(resource), _head(nullptr), _cur_block(nullptr), _cur_pos(nullptr)
    {}

    pt_buffer(pt_buffer&& other) noexcept
    : _resource(other._resource), _head(other._head), _cur_block(other._cur_block),
      _cur_pos(other._cur_pos)
    {
        other._head = other._cur_block = nullptr;
        other._cur_pos                 = nullptr;
    }

    ~pt_buffer() noexcept
    {
        auto cur = _head;
        while (cur != nullptr)
            cur = block::deallocate(_resource, cur);
    }

    pt_buffer& operator=(pt_buffer&& other) noexcept
    {
        lexy::_detail::swap(_resource, other._resource);
        lexy::_detail::swap(_head, other._head);
        lexy::_detail::swap(_cur_block, other._cur_block);
        lexy::_detail::swap(_cur_pos, other._cur_pos);
        return *this;
    }

    //=== allocation ===//
    // Allocates the first block for the buffer.
    // Must be called before everything else.
    // (If done in the constructor, it would require a move that does allocation which we don't
    // want).
    // If called after being initialized, destroys all nodes without releasing memory.
    void reset()
    {
        if (!_head)
            _head = block::allocate(_resource);

        _cur_block = _head;
        _cur_pos   = &_cur_block->memory[0];
    }

    void reserve(std::size_t size)
    {
        if (remaining_capacity() < size)
        {
            auto next        = block::allocate(_resource);
            _cur_block->next = next;
            _cur_block       = next;
            _cur_pos         = &_cur_block->memory[0];
        }
    }

    template <typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(alignof(T) == alignof(void*));
        LEXY_PRECONDITION(_cur_block);                        // Forgot to call .init().
        LEXY_PRECONDITION(remaining_capacity() >= sizeof(T)); // Forgot to call .reserve().

        auto memory = _cur_pos;
        _cur_pos += sizeof(T);
        return ::new (static_cast<void*>(memory)) T(LEXY_FWD(args)...);
    }

    void unwind(void* marker) noexcept
    {
        auto pos = static_cast<unsigned char*>(marker);

        // Note: this is not guaranteed to work by the standard;
        // We'd have to go through std::less instead.
        // However, on all implementations I care about, std::less just does < anyway.
        if (_cur_block->memory <= pos && pos < _cur_block->end())
            // We're still in the same block, just reset position.
            _cur_pos = pos;
        else
            // Reset to the beginning of the current block only.
            // This can waste memory, but this is not a problem here:
            // unwind() is only used to backtrack a production, which happens after a couple of
            // tokens only; the memory waste is directly proportional to the lookahead length.
            _cur_pos = _cur_block->memory;
    }

private:
    std::size_t remaining_capacity() const noexcept
    {
        return std::size_t(_cur_block->end() - _cur_pos);
    }

    LEXY_EMPTY_MEMBER resource_ptr _resource;
    block*                         _head;

    block*         _cur_block;
    unsigned char* _cur_pos;
};
} // namespace lexy::_detail

//=== parse_tree ===//
namespace lexy
{
template <typename Reader, typename TokenKind = void, typename MemoryResource = void>
class parse_tree
{
public:
    //=== construction ===//
    class builder;

    constexpr parse_tree() : parse_tree(_detail::get_memory_resource<MemoryResource>()) {}
    constexpr explicit parse_tree(MemoryResource* resource)
    : _buffer(resource), _root(nullptr), _size(0), _depth(0)
    {}

    //=== container access ===//
    bool empty() const noexcept
    {
        return _root == nullptr;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }

    std::size_t depth() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return _depth;
    }

    void clear() noexcept
    {
        _buffer.reset();
        _root = nullptr;
    }

    //=== node access ===//
    class node;
    class node_kind;

    node root() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return node(_root);
    }

    //=== traverse ===//
    class traverse_range;

    traverse_range traverse(const node& n) const noexcept
    {
        return traverse_range(n);
    }
    traverse_range traverse() const noexcept
    {
        if (empty())
            return traverse_range();
        else
            return traverse_range(root());
    }

private:
    _detail::pt_buffer<MemoryResource>   _buffer;
    _detail::pt_node_production<Reader>* _root;
    std::size_t                          _size;
    std::size_t                          _depth;
};

template <typename Input, typename TokenKind = void, typename MemoryResource = void>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
public:
    template <typename Production>
    explicit builder(parse_tree&& tree, Production production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._size = 1;

        // Begin construction at the root.
        _cur = marker(_result._root, 0);
    }
    template <typename Production>
    explicit builder(Production production) : builder(parse_tree(), production)
    {}

    struct marker
    {
        // The current production all tokens are appended to.
        _detail::pt_node_production<Reader>* prod = nullptr;
        // The depth of the current production.
        std::size_t depth = 0;
        // The last child of the current production.
        _detail::pt_node_ptr<Reader> last_child;

        marker() = default;

        explicit marker(_detail::pt_node_production<Reader>* prod, std::size_t depth)
        : prod(prod), depth(depth)
        {}

        template <typename T>
        void append(T* child)
        {
            ++prod->child_count;

            if (last_child)
            {
                // Add a sibling to the last child.
                last_child.base()->ptr.set_sibling(child);
                // child is now the last child.
                last_child.set_sibling(child);
            }
            else
            {
                // We're adding the first child of a node, which is also the last child.
                last_child.set_sibling(child);

                if (last_child.base() == prod + 1)
                {
                    // The first child is stored adjacent.
                    prod->first_child_adjacent = true;
                    prod->first_child_type     = last_child.type() & 0b1;
                }
                else
                {
                    // The child is not stored immediately afterwards.
                    // This only happens when a new block had to be started.
                    // In that case, we've saved enough space after the production to add a pointer.
                    auto memory = static_cast<void*>(prod + 1);
                    ::new (memory) _detail::pt_node_ptr<Reader>(last_child);

                    prod->first_child_adjacent = false;
                }
            }
        }

        void finish(std::size_t& size, std::size_t& max_depth)
        {
            if (last_child)
                // The pointer of the last child needs to point back to prod.
                last_child.base()->ptr.set_parent(prod);

            // Update the size.
            size += prod->child_count;

            // And update the depth.
            auto local_max_depth = prod->child_count > 0 ? depth + 1 : depth;
            if (max_depth < local_max_depth)
                max_depth = local_max_depth;
        }
    };

    template <typename Production>
    auto start_production(Production production)
    {
        if constexpr (lexy::is_transparent_production<Production>)
            // Don't need to add a new node for a transparent production.
            return marker();

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node_ptr<Reader>));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        // Note: don't append the node yet, we might still backtrack.

        // Subsequent inertions are to the new node, so update marker and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(node, old.depth + 1);
        return old;
    }

    void token(token_kind<TokenKind> _kind, typename Reader::iterator begin,
               typename Reader::iterator end)
    {
        if (_kind.ignore_if_empty() && begin == end)
            return;

        auto kind = token_kind<TokenKind>::to_raw(_kind);

        if (auto token = _cur.last_child.token();
            // We merge error tokens.
            token && token->kind == kind && kind == lexy::error_token_kind)
        {
            // No need to allocate a new node, just extend the previous node.
            token->update_end(end);
        }
        else
        {
            // Allocate and append.
            _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
            auto node
                = _result._buffer.template allocate<_detail::pt_node_token<Reader>>(kind, begin,
                                                                                    end);
            _cur.append(node);
        }
    }

    void finish_production(marker&& m)
    {
        if (!m.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        // We're done with the current production.
        _cur.finish(_result._size, _result._depth);
        // Append to previous production.
        m.append(_cur.prod);
        // Continue with the previous production.
        _cur = LEXY_MOV(m);
    }

    void cancel_production(marker&& m)
    {
        if (!m.prod)
            // We're backtracking a transparent production, do nothing.
            return;

        // Deallocate everything from the backtracked production.
        _result._buffer.unwind(_cur.prod);
        // Continue with previous production.
        _cur = LEXY_MOV(m);
    }

    parse_tree&& finish() &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);
        _cur.finish(_result._size, _result._depth);
        return LEXY_MOV(_result);
    }

private:
    parse_tree _result;
    marker     _cur;
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::node_kind
{
public:
    bool is_token() const noexcept
    {
        return _ptr.token() != nullptr;
    }
    bool is_production() const noexcept
    {
        return _ptr.production() != nullptr;
    }

    bool is_root() const noexcept
    {
        // Root node has no next pointer.
        return !_ptr.base()->ptr;
    }
    bool is_token_production() const noexcept
    {
        return is_production() && _ptr.production()->token_production;
    }

    const char* name() const noexcept
    {
        if (auto prod = _ptr.production())
            return prod->name;
        else if (auto token = _ptr.token())
            return token_kind<TokenKind>::from_raw(token->kind).name();
        else
        {
            LEXY_ASSERT(false, "unreachable");
            return nullptr;
        }
    }

    friend bool operator==(node_kind lhs, node_kind rhs)
    {
        if (lhs.is_token() && rhs.is_token())
            return lhs._ptr.token()->kind == rhs._ptr.token()->kind;
        else
            // See the `operator==` for productions for rationale why this works.
            return lhs._ptr.production()->name == rhs._ptr.production()->name;
    }
    friend bool operator!=(node_kind lhs, node_kind rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(node_kind nk, token_kind<TokenKind> tk)
    {
        if (auto token = nk._ptr.token())
            return token_kind<TokenKind>::from_raw(token->kind) == tk;
        else
            return false;
    }
    friend bool operator==(token_kind<TokenKind> tk, node_kind nk)
    {
        return nk == tk;
    }
    friend bool operator!=(node_kind nk, token_kind<TokenKind> tk)
    {
        return !(nk == tk);
    }
    friend bool operator!=(token_kind<TokenKind> tk, node_kind nk)
    {
        return !(nk == tk);
    }

    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator==(node_kind nk, Production)
    {
        auto name = lexy::production_name<Production>();
        // We can safely compare pointers, strings are necessarily interned:
        // if Production::name exists: same address for all types,
        // otherwise we use __PRETTY_FUNCTION__ (or equivalent), which is a function-local static.
        //
        // This only fails if we have different productions with the same name and the compiler does
        // string interning. But as the production name corresponds to the qualified C++ name (by
        // default), this is only possible if the user does something weird.
        return nk.is_production() && nk._ptr.production()->name == name;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator==(Production p, node_kind nk)
    {
        return nk == p;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator!=(node_kind nk, Production p)
    {
        return !(nk == p);
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    friend bool operator!=(Production p, node_kind nk)
    {
        return !(nk == p);
    }

private:
    explicit node_kind(_detail::pt_node_ptr<Reader> ptr) : _ptr(ptr) {}

    _detail::pt_node_ptr<Reader> _ptr;

    friend parse_tree::node;
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::node
{
public:
    void* address() const noexcept
    {
        return _ptr.base();
    }

    auto kind() const noexcept
    {
        return node_kind(_ptr);
    }

    auto parent() const noexcept
    {
        if (kind().is_root())
            // The root has itself as parent.
            return *this;

        // If we follow the sibling pointer, we reach a parent pointer.
        auto cur = _ptr.base()->ptr;
        while (cur.is_sibling_ptr())
            cur = cur.base()->ptr;
        return node(cur);
    }

    class children_range
    {
    public:
        class iterator;
        struct sentinel : _detail::sentinel_base<sentinel, iterator>
        {};

        class iterator : public _detail::forward_iterator_base<iterator, node, node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            node deref() const noexcept
            {
                LEXY_PRECONDITION(*this != sentinel{});
                return node(_cur);
            }

            void increment() noexcept
            {
                LEXY_PRECONDITION(*this != sentinel{});
                _cur = _cur.base()->ptr;
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur.base() == rhs._cur.base();
            }
            bool is_end() const noexcept
            {
                // We're at the end of the children, if the current pointer is a parent pointer.
                // For a default constructed iterator, nullptr is a parent pointer, so this works as
                // well.
                return _cur.is_parent_ptr();
            }

        private:
            explicit iterator(_detail::pt_node_ptr<Reader> ptr) noexcept : _cur(ptr) {}

            _detail::pt_node_ptr<Reader> _cur;

            friend children_range;
        };

        bool empty() const noexcept
        {
            return _count == 0;
        }

        std::size_t size() const noexcept
        {
            return _count;
        }

        iterator begin() const noexcept
        {
            return iterator(_begin);
        }
        sentinel end() const noexcept
        {
            return {};
        }

    private:
        explicit children_range(_detail::pt_node_ptr<Reader> begin, std::size_t count)
        : _begin(begin), _count(count)
        {}

        _detail::pt_node_ptr<Reader> _begin;
        std::size_t                  _count;

        friend node;
    };

    auto children() const noexcept
    {
        if (auto prod = _ptr.production())
            return children_range(prod->first_child(), prod->child_count);
        else
            return children_range(_detail::pt_node_ptr<Reader>{}, 0);
    }

    class sibling_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, node, node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            node deref() const noexcept
            {
                return node(_cur);
            }

            void increment() noexcept
            {
                if (_cur.base()->ptr.is_parent_ptr())
                    // We're pointing to the parent, go to first child instead.
                    _cur = _cur.base()->ptr.production()->first_child();
                else
                    // We're pointing to a sibling, go there.
                    _cur = _cur.base()->ptr;
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur.base() == rhs._cur.base();
            }

        private:
            explicit iterator(_detail::pt_node_ptr<Reader> ptr) noexcept : _cur(ptr) {}

            _detail::pt_node_ptr<Reader> _cur;

            friend sibling_range;
        };

        bool empty() const noexcept
        {
            return begin() == end();
        }

        iterator begin() const noexcept
        {
            // We begin with the next node after ours.
            // If we don't have siblings, this is our node itself.
            return ++iterator(_node);
        }
        iterator end() const noexcept
        {
            // We end when we're back at the node.
            return iterator(_node);
        }

    private:
        explicit sibling_range(_detail::pt_node_ptr<Reader> node) noexcept : _node(node) {}

        _detail::pt_node_ptr<Reader> _node;

        friend node;
    };

    auto siblings() const noexcept
    {
        return sibling_range(_ptr);
    }

    bool is_last_child() const noexcept
    {
        // We're the last child if our pointer points to the parent.
        return _ptr.base()->ptr.is_parent_ptr();
    }

    auto lexeme() const noexcept
    {
        if (auto token = _ptr.token())
            return lexy::lexeme<Reader>(token->begin, token->end());
        else
            return lexy::lexeme<Reader>();
    }

    auto token() const noexcept
    {
        LEXY_PRECONDITION(kind().is_token());

        auto token = _ptr.token();
        auto kind  = token_kind<TokenKind>::from_raw(token->kind);
        return lexy::token<Reader, TokenKind>(kind, token->begin, token->end());
    }

    friend bool operator==(node lhs, node rhs) noexcept
    {
        return lhs._ptr.base() == rhs._ptr.base();
    }
    friend bool operator!=(node lhs, node rhs) noexcept
    {
        return lhs._ptr.base() != rhs._ptr.base();
    }

private:
    explicit node(_detail::pt_node_ptr<Reader> ptr) noexcept : _ptr(ptr) {}
    explicit node(_detail::pt_node_production<Reader>* ptr) noexcept
    {
        // It doesn't matter whether the pointer is a parent or sibling.
        _ptr.set_parent(ptr);
    }

    _detail::pt_node_ptr<Reader> _ptr;

    friend parse_tree;
};

enum class traverse_event
{
    /// We're visiting a production node before all its children.
    enter,
    /// We're visiting a production node after all its children.
    exit,
    /// We're visiting a token.
    leaf,
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::traverse_range
{
public:
    using event = traverse_event;

    struct _value_type
    {
        traverse_event   event;
        parse_tree::node node;
    };

    class iterator : public _detail::forward_iterator_base<iterator, _value_type, _value_type, void>
    {
    public:
        iterator() noexcept = default;

        _value_type deref() const noexcept
        {
            if (_cur.token())
                // We're only visiting tokens once.
                return {traverse_event::leaf, node(_cur)};
            else if (_cur.is_sibling_ptr())
                // If it's a sibling pointer, we're entering the production for the first time.
                return {traverse_event::enter, node(_cur)};
            else if (_cur.is_parent_ptr())
                // If it's a parent pointer, we're revisiting the production after all the children.
                return {traverse_event::exit, node(_cur)};
            else
            {
                LEXY_ASSERT(false, "unreachable");
                return {{}, node(_cur)};
            }
        }

        void increment() noexcept
        {
            if (_cur.token() || _cur.is_parent_ptr())
                // We're currently pointing to a token or back to the parent production.
                // Continue with its sibling.
                _cur = _cur.base()->ptr;
            else if (_cur.is_sibling_ptr())
                // We're currently pointing to a production for the first time.
                // Continue to the first child.
                _cur = _cur.production()->first_child();
            else
                LEXY_ASSERT(false, "unreachable");
        }

        bool equal(iterator rhs) const noexcept
        {
            // We need to point to the same node and in the same role.
            return _cur.base() == rhs._cur.base()
                   && _cur.is_parent_ptr() == rhs._cur.is_parent_ptr();
        }

    private:
        _detail::pt_node_ptr<Reader> _cur;

        friend traverse_range;
    };

    bool empty() const noexcept
    {
        return _begin == _end;
    }

    iterator begin() const noexcept
    {
        return _begin;
    }

    iterator end() const noexcept
    {
        return _end;
    }

private:
    traverse_range() noexcept = default;
    traverse_range(node n) noexcept
    {
        if (n.kind().is_token())
        {
            _begin._cur.set_sibling(n._ptr.token());
            _end = _begin;
        }
        else
        {
            _begin._cur.set_sibling(n._ptr.production());
            _end._cur.set_parent(n._ptr.production());
        }

        // Turn it into a half-open range.
        ++_end;
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

#endif // LEXY_PARSE_TREE_HPP_INCLUDED

