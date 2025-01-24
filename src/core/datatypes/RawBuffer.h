#ifndef RAWBUFFER_H
#define RAWBUFFER_H

#include <stdexcept>

template<typename T>
class RawBuffer {
private:
    size_t m_capacity;
    size_t m_size;
    T* m_data;

public:
    RawBuffer() : m_capacity(0), m_size(0), m_data(nullptr) {}
    RawBuffer(size_t capacity) : m_capacity(capacity), m_size(0), m_data(new T[capacity]) {};
    RawBuffer(const RawBuffer& other) = delete;
    RawBuffer(RawBuffer&& other) noexcept
        : m_capacity(other.m_capacity), m_size(other.m_size), m_data(other.m_data) {        
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }
    ~RawBuffer() { delete[] m_data; }

    void push_back(const T& value) {
        if (m_size >= m_capacity) {
            throw std::runtime_error("Buffer overflow!");
        }
        m_data[m_size++] = value;
    }

    void push_back(T&& value) {
        if (m_size >= m_capacity) {
            throw std::runtime_error("Buffer overflow!");
        }
        m_data[m_size++] = std::move(value);
    }

    void clear() { m_size = 0; }

    size_t size() const { return m_size; }

    size_t capacity() const { return m_capacity; }

    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range!");
        }
        return m_data[index];
    }

    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range!");
        }
        return m_data[index];
    }

    T& operator[](size_t index) { return m_data[index]; }

    const T& operator[](size_t index) const { return m_data[index]; }

    T* data() { return m_data; }

    const T* data() const { return m_data; }

    T* begin() { return m_data; }

    T* end() { return m_data + m_size; }

    const T* begin() const { return m_data; }

    const T* end() const { return m_data + m_size; }

    RawBuffer& operator=(const RawBuffer& other) = delete;
    
    RawBuffer& operator=(RawBuffer&& other) noexcept {
        if (this != &other) {
            if (m_data) {
                delete[] m_data;
            }

            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_data = other.m_data;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }
};

#endif