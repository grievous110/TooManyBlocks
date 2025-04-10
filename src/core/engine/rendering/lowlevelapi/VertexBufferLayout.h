#ifndef VERTEXBUFFERLAYOUT_H
#define VERTEXBUFFERLAYOUT_H

#include <vector>

struct BufferLayoutElement {
	unsigned int type;
    unsigned int typeSize;
	unsigned int count;
	bool normalized;
};

class VertexBufferLayout {
private:
	std::vector<BufferLayoutElement> m_elements;
    unsigned int m_stride;
    
public:
    VertexBufferLayout() : m_stride(0) {};

	void push(unsigned int type, unsigned int typeSize, unsigned int count, bool normalized = false) {
        m_elements.push_back({ type , typeSize, count, normalized });
        m_stride += count * typeSize;
	}
    
    inline const std::vector<BufferLayoutElement>& elements() const { return m_elements; }

    inline unsigned int stride() const { return m_stride; }
};

#endif