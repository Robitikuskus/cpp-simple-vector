#pragma once

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve)
    : capacity_(capacity_to_reserve) {}

    size_t GetCapacityToReserve() const noexcept {
        return capacity_;
    }

private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) 
    : items_(size)
    , size_(size)
    , capacity_(size) {}

    explicit SimpleVector(size_t size, const Type& value)
    : items_(size, value)
    , size_(size)
    , capacity_(size) {}

    explicit SimpleVector(const SimpleVector& other)
    : items_(other.size_)
    , size_(other.size_)
    , capacity_(other.capacity_) {
        CopyAndSwap(other);
    }

    explicit SimpleVector(std::initializer_list<Type> init) {
        CopyAndSwap(init);
    }

    explicit SimpleVector(const ReserveProxyObj& capacity) {
        IncreaseCapacity(capacity.GetCapacityToReserve());
    }

    SimpleVector(SimpleVector&& other) noexcept
    : items_(std::move(other.items_))
    , size_(other.size_)
    , capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            Clear();
            SimpleVector t(std::move(other));
            swap(t);
        }
        return *this;
    }

    void Reserve(size_t capacity) {
        if (capacity_ < capacity) {
            IncreaseCapacity(capacity);
        }
    }

    void PushBack(const Type& item) {
        IsSizeEqualsCapacity();

        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        IsSizeEqualsCapacity();

        items_[size_] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_index = static_cast<size_t>(pos - begin());
        IsSizeEqualsCapacity();
        
        Iterator new_pos = begin() + pos_index;
        std::move_backward(new_pos, end(), end() + 1);
        *new_pos = value;
        ++size_;
        return new_pos;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_index = static_cast<size_t>(pos - begin());
        IsSizeEqualsCapacity();
        
        Iterator new_pos = begin() + pos_index;
        std::move_backward(new_pos, end(), end() + 1);
        *new_pos = std::move(value);
        ++size_;
        return new_pos;
    }

    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        if (size_ > 0) {
            auto index = pos - begin();
            std::move(begin() + index + 1, end(), begin() + index);
            --size_;
        }
        return begin() + (pos - begin());
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            IncreaseCapacity(std::max(new_size, capacity_ * 2));
        } else if (new_size > size_) {
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = std::move(Type());
            }
        }
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    size_t size() const {
        return size_;
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;

    void IncreaseCapacity(size_t new_capacity) {
        ArrayPtr<Type> new_items(new_capacity, Type());
            
        for (size_t i = 0; i < size_; ++i) {
            new_items[i] = std::move(items_[i]);
        }
        items_.swap(new_items);
        
        capacity_ = new_capacity;
    }

    template <typename Other>
    void CopyAndSwap(const Other& other) {
        SimpleVector c(other.size());
        std::copy(other.begin(), other.end(), c.begin());
        swap(c);
    }

    void IsSizeEqualsCapacity() {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            IncreaseCapacity(new_capacity);
        }
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 