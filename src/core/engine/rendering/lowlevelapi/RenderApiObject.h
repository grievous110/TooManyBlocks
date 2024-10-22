#ifndef RENDERAPIOBJECT_H
#define RENDERAPIOBJECT_H

class RenderApiObject {
protected:
	unsigned int m_rendererId;

	RenderApiObject();
	RenderApiObject(unsigned int id);
	virtual ~RenderApiObject() = 0;
public:
	inline unsigned int rendererId() const { return m_rendererId; }
};

#endif