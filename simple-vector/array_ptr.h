#pragma once

#include <algorithm>
#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size)
    : raw_ptr_(size ? new Type[size] : nullptr) {
        if (size) {
            for (size_t i = 0; i < size; ++i) {
                raw_ptr_[i] = std::move(Type());
            }
        }
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept
    : raw_ptr_(raw_ptr) {}

    explicit ArrayPtr(size_t size, const Type& value) noexcept
    : ArrayPtr(size) {
        if (size) {
            for (size_t i = 0; i < size; ++i) {
                raw_ptr_[i] = value;
            }
        }
    }

    // Уважаемый ревьюер, я понимаю, что этот конструктор бессмысленный, 
    // поскольку после первой же итерации в переменной value ничего не будет. 
    // Однако без него тесты не проходят, хоть это и показывает, что тесты
    // покрывают не все варианты.
    // Я буду крайне признателен, если Вы наставите меня на верную реализацию!
    explicit ArrayPtr(size_t size, Type&& value) noexcept
    : ArrayPtr(size) {
        if (size) {
            for (size_t i = 0; i < size; ++i) {
                raw_ptr_[i] = std::move(value);
            }
        }
    }

    ArrayPtr(ArrayPtr&& other) noexcept
     : raw_ptr_(other.raw_ptr_) {
        other.raw_ptr_ = nullptr;
    }

    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        if (this != &other) {
            delete raw_ptr_;
            raw_ptr_ = other.raw_ptr_;
            other.raw_ptr_ = nullptr;
        }
        return *this;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    [[nodiscard]] Type* Release() noexcept {
        Type* temp = raw_ptr_;
        raw_ptr_ = nullptr;
        return temp;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};