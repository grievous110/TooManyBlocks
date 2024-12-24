#ifndef RENDERAPIOBJECT_H
#define RENDERAPIOBJECT_H

class RenderApiObject {
protected:
	unsigned int m_rendererId;

	RenderApiObject(unsigned int id = 0) : m_rendererId(id) {};
	virtual ~RenderApiObject() {};
	
public:
	inline unsigned int rendererId() const { return m_rendererId; }
};

#endif