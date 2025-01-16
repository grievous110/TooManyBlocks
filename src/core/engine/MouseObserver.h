#ifndef MOUSEOBSERVER_H
#define MOUSEOBSERVER_H

#include <vector>

enum class MousEvent {
	Move,
	ButtonDown,
	ButtonUp,
	Scroll
};

struct MouseEventData {
	union {
		struct {
			double x;
			double y;
		} delta;
		struct {
			int code;
		} key;
	};
};

class MouseObserver {
public:
	virtual void notify(MousEvent event, MouseEventData data) = 0;
	virtual ~MouseObserver() = default;
};

class MouseObservable {
private:
	std::vector<MouseObserver*> m_observers;
public:
	virtual void attach(MouseObserver* observer);
	virtual void detach(MouseObserver* observer);
	virtual void notifyObservers(MousEvent event, MouseEventData data);
};

#endif