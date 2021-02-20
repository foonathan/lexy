// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/memory_resource.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/input/base.hpp>
#include <lexy/production.hpp>
#include <lexy/result.hpp>
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
    // If it's not a pointer, we store size instead of end.
    static constexpr auto _optimize_end = std::is_pointer_v<typename Reader::iterator>;
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

    typename Reader::iterator end() const noexcept
    {
        if constexpr (_optimize_end)
            return begin + end_impl;
        else
            return end_impl;
    }
};

template <typename Reader>
struct pt_node_production : pt_node<Reader>
{
    static constexpr std::size_t child_count_bits = sizeof(std::size_t) * CHAR_BIT - 2;

    const char* name;
    std::size_t child_count : child_count_bits;
    std::size_t first_child_adjacent : 1;
    std::size_t first_child_type : 1;

    template <typename Production>
    explicit pt_node_production(Production) noexcept
    : child_count(0), first_child_adjacent(true), first_child_type(pt_node_ptr<Reader>::type_token)
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
template <typename Reader, typename TokenKind = void,
          typename MemoryResource = _detail::default_memory_resource>
class parse_tree
{
public:
    //=== construction ===//
    class builder;

    constexpr parse_tree() : parse_tree(_detail::get_memory_resource<MemoryResource>()) {}
    constexpr explicit parse_tree(MemoryResource* resource) : _buffer(resource), _root(nullptr) {}

    //=== container access ===//
    bool empty() const noexcept
    {
        return _root == nullptr;
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
};

template <typename Input, typename TokenKind = void,
          typename MemoryResource = _detail::default_memory_resource>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
    struct state;

public:
    template <typename Production>
    explicit builder(parse_tree&& tree, Production production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node and begin construction there.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _cur = state(_result._root);
    }
    template <typename Production>
    explicit builder(Production production) : builder(parse_tree(), production)
    {}

    using production_state = state;

    template <typename Production>
    auto start_production(Production production)
    {
        if constexpr (lexy::is_transparent_production<Production>)
            // Don't need to add a new node for a transparent production.
            return state();

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node_ptr<Reader>));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _cur.append(node);

        // Subsequent insertions are to the new node, so update state and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = state(node);
        return old;
    }

    void token(token_kind<TokenKind> _kind, typename Reader::iterator begin,
               typename Reader::iterator end)
    {
        if (begin == end)
            // Don't add empty tokens to the tree.
            return;

        auto kind = token_kind<TokenKind>::to_raw(_kind);

        // Allocate and append.
        _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_token<Reader>>(kind, begin, end);
        _cur.append(node);
    }

    void finish_production(state&& s)
    {
        if (!s.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        // We're done with the current production.
        _cur.finish();
        // Continue with the previous production.
        _cur = LEXY_MOV(s);
    }

    parse_tree finish() &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);
        _cur.finish();
        return LEXY_MOV(_result);
    }

private:
    parse_tree _result;
    struct state
    {
        // The current production all tokens are appended to.
        _detail::pt_node_production<Reader>* prod;
        // The first child of the current production.
        _detail::pt_node_ptr<Reader> first_child;
        // This pointer is updated on insert.
        // It initially points to first_child.
        _detail::pt_node_ptr<Reader>* last_child_ptr;

        state() = default;

        explicit state(_detail::pt_node_production<Reader>* prod)
        : prod(prod), last_child_ptr(&first_child)
        {}

        state(state&& other) noexcept
        {
            *this = LEXY_MOV(other);
        }

        ~state() noexcept = default;

        state& operator=(state&& other) noexcept
        {
            prod        = other.prod;
            first_child = other.first_child;

            if (other.last_child_ptr == &other.first_child)
                last_child_ptr = &first_child;
            else
                last_child_ptr = other.last_child_ptr;
            return *this;
        }

        template <typename T>
        void append(T* child)
        {
            ++prod->child_count;

            // Add a sibling to the last child.
            last_child_ptr->set_sibling(child);
            last_child_ptr = &child->ptr;
        }

        void finish()
        {
            if (prod->child_count == 0)
                return;

            // The pointer of the last child needs to point back to prod.
            last_child_ptr->set_parent(prod);

            // Now we need to store the first child pointer value in the node.
            if (first_child.base() == prod + 1)
            {
                // The first child is stored adjacent.
                prod->first_child_adjacent = true;
                prod->first_child_type     = first_child.type() & 0b1;
            }
            else
            {
                // The child is not stored immediately afterwards.
                // This only happens when a new block had to be started.
                // In that case, we've saved enough space after the production to add a pointer.
                auto memory = static_cast<void*>(prod + 1);
                ::new (memory) _detail::pt_node_ptr<Reader>(first_child);

                prod->first_child_adjacent = false;
            }
        }
    } _cur;
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::node_kind
{
public:
    bool is_root() const noexcept
    {
        // Root node has no next pointer.
        return !_ptr.base()->ptr;
    }

    bool is_token() const noexcept
    {
        return _ptr.token() != nullptr;
    }
    bool is_production() const noexcept
    {
        return _ptr.production() != nullptr;
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
    struct _value_type
    {
        traverse_event   event;
        parse_tree::node node;
    };

    class iterator : public _detail::forward_iterator_base<iterator, _value_type, _value_type, void>
    {
    public:
        iterator() noexcept = default;
        iterator(traverse_event ev, node n) noexcept
        {
            LEXY_PRECONDITION(!n.kind().is_token() || ev == traverse_event::leaf);
            if (ev == traverse_event::exit)
                _cur.set_parent(n._ptr.production());
            else
                _cur.set_sibling(n._ptr.base(), n._ptr.type());
        }

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
            if (_cur.token())
                // We're currently pointing to a token.
                // Continue with its sibling.
                _cur = _cur.base()->ptr;
            else if (_cur.is_sibling_ptr())
                // We're currently pointing to a production for the first time.
                // Continue to the first child.
                _cur = _cur.production()->first_child();
            else if (_cur.is_parent_ptr())
                // We're currently pointing back to the parent production.
                // We continue with its sibling.
                _cur = _cur.base()->ptr;
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
            _begin = iterator(traverse_event::leaf, n);
            _end   = _begin;
            ++_end;
        }
        else
        {
            _begin = iterator(traverse_event::enter, n);
            _end   = iterator(traverse_event::exit, n);
            ++_end;
        }
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

//=== parse_as_tree ===//
namespace lexy
{
template <typename Tree, typename Input, typename Callback>
class _pt_handler
{
public:
    explicit _pt_handler(Tree& tree, const Input& input, Callback&& cb)
    : _root(), _tree(&tree), _depth(0), _input(&input), _callback(LEXY_MOV(cb))
    {}

    ~_pt_handler() noexcept
    {
        if (_depth > 0)
            _builder.~builder();
    }

    _pt_handler(const _pt_handler&) = delete;
    _pt_handler& operator=(const _pt_handler&) = delete;

    template <typename Production>
    using result_type_for = lexy::result<void, typename Callback::return_type>;

    template <typename Production>
    constexpr auto get_sink(Production)
    {
        return noop.sink();
    }

    struct _state_t
    {
        typename Tree::builder::production_state     builder_state;
        typename lexy::input_reader<Input>::iterator pos;
    };

    template <typename Production, typename Iterator>
    constexpr _state_t start_production(Production prod, Iterator pos)
    {
        if (_depth++ == 0)
        {
            ::new (&_builder) typename Tree::builder(LEXY_MOV(*_tree), prod);
            return {{}, pos};
        }
        else
        {
            return {_builder.start_production(prod), pos};
        }
    }

    template <typename Kind, typename Iterator>
    constexpr void token(Kind kind, Iterator begin, Iterator end)
    {
        LEXY_PRECONDITION(_depth > 0);
        _builder.token(kind, begin, end);
    }

    template <typename Production, typename... Args>
    constexpr auto finish_production(Production, _state_t&& state, Args&&...)
    {
        if (--_depth == 0)
            // Finish tree instead of production.
            *_tree = LEXY_MOV(_builder).finish();
        else
            _builder.finish_production(LEXY_MOV(state.builder_state));

        return result_type_for<Production>(lexy::result_value);
    }

    template <typename Production, typename Error>
    constexpr auto error(Production p, _state_t&& state, Error&& error)
    {
        // Clear the tree to prevent an incomplete one from showing up.
        _tree->clear();

        lexy::error_context err_ctx(p, *_input, state.pos);
        return lexy::invoke_as_result<result_type_for<Production>>(lexy::result_error, _callback,
                                                                   err_ctx, LEXY_FWD(error));
    }

private:
    // We need to lazily construct a builder.
    union
    {
        typename Tree::builder _builder;
        char                   _root;
    };
    Tree* _tree;
    int   _depth; // To check whether the builder is initialized.

    const Input*               _input;
    LEXY_EMPTY_MEMBER Callback _callback;
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename Callback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, Callback callback)
{
    auto                handler = _pt_handler(tree, input, LEXY_MOV(callback));
    auto                reader  = input.reader();
    lexy::parse_context context(Production{}, handler, reader.cur());

    using rule = lexy::production_rule<Production>;
    return lexy::rule_parser<rule, lexy::context_value_parser>::parse(context, reader);
}
} // namespace lexy

#endif // LEXY_PARSE_TREE_HPP_INCLUDED

