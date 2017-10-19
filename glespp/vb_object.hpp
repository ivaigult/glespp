#pragma once

#include <glad/glad.h>

namespace glespp {

template<typename element_t>
class buffer_object {
public:
    typedef element_t  element_type;
    typedef element_t* element_pointer;

    template<typename IteratorT>
    buffer_object(IteratorT begin, IteratorT end) {
        size_t size = static_cast<size_t>(std::distance(begin, end));
        _size = size;
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(element_type) * size, nullptr, GL_DYNAMIC_DRAW);
        update(0, begin, end);
    }

    buffer_object(std::initializer_list<element_type> il) {
        _size = il.size();
        size_t gpubuf_size = _size * sizeof(element_type);
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, (GLsizei)gpubuf_size, nullptr, GL_DYNAMIC_DRAW);
        update(0, il);
    }

    buffer_object(size_t size) {
        _size = size;
        glGenBuffers(1, &_id);
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(element_type) * size, nullptr, GL_DYNAMIC_DRAW);
    }

    ~buffer_object() {
        glDeleteBuffers(1, &_id);
    }

    template<typename IteratorT>
    void update(size_t offset, IteratorT begin, IteratorT end) {
        std::vector<element_type> data(begin, end);
        update(offset, data.data(), data.size() * sizeof(element_type));
    }

    template<typename ValueT>
    void update(size_t offset, std::initializer_list<ValueT> il) {
        update(offset, il.begin(), il.end());
    }
    
    void update(size_t offset, element_pointer begin, element_pointer end) {
        size_t size = static_cast<size_t>(end - begin) * sizeof(element_type);
        update(offset, begin, size);
    }

    void update(size_t offset, element_pointer ptr, size_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, _id);
        glBufferSubData(GL_ARRAY_BUFFER, GLsizei(offset), size, ptr);
    }

    GLuint get_id() const { return _id;   }
    size_t size()   const { return _size; }
private:
    GLuint _id;
    size_t _size = 0;
};
} // glespp
