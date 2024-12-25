#ifndef RENDERAPIOBJECT_H
#define RENDERAPIOBJECT_H

// Base class wich prevents copying, but allows moving
class RenderApiObject {
protected:
	unsigned int m_rendererId;

	RenderApiObject(unsigned int id = 0) : m_rendererId(id) {};
	RenderApiObject(RenderApiObject&& other) noexcept : m_rendererId(other.m_rendererId) { other.m_rendererId = 0; }
	RenderApiObject(const RenderApiObject&) = delete;
	virtual ~RenderApiObject() {};
	
public:
	inline unsigned int rendererId() const { return m_rendererId; }

	RenderApiObject& operator=(RenderApiObject&& other) noexcept {
		if (this != &other) {
			m_rendererId = other.m_rendererId;
			other.m_rendererId = 0;
		}
		return *this;
	}
	RenderApiObject& operator=(const RenderApiObject&) = delete;
};

#endif