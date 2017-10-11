#pragma once

#include <glad/glad.h>

template<typename ElementType>
class Buffer {
public:
    typedef ElementType  ElementType;
    typedef ElementType* ElementPtrType;

    Buffer(std::initializer_list<ElementType> il) {
        size_t size = il.size() * sizeof(ElementType);
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, (GLsizei)size, nullptr, GL_DYNAMIC_DRAW);
        Update(0, il);
    }

    Buffer(size_t size) {
        glGenBuffer(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(ElementType) * size, nullptr, GL_DYNAMIC_DRAW);
    }

    ~Buffer() {
        glDeleteBuffers(1, &_id);
    }

    template<typename IteratorT>
    void Update(size_t offset, IteratorT begin, IteratorT end) {
        std::vector<ElementType> data(begin, end);
        Update(offset, data.data(), data.size() * sizeof(ElementType));
    }

    template<typename ValueT>
    void Update(size_t offset, std::initializer_list<ValueT> il) {
        Update(offset, il.begin(), il.end());
    }
    
    void Update(size_t offset, ElementPtrType begin, ElementPtrType end) {
        size_t size = static_cast<size_t>(end - begin) * sizeof(ElementType);
        Update(offset, begin, size);
    }

    void Update(size_t offset, ElementPtrType ptr, size_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferSubData(GL_ARRAY_BUFFER, GLsizei(offset), size, ptr);
    }

    GLuint GetId() const { return _id; }
private:
    GLuint _id;
};