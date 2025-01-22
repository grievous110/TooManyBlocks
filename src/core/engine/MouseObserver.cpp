#include "MouseObserver.h"
#include <algorithm>

void MouseObservable::attach(MouseObserver* observer) {
	m_observers.push_back(observer);
}

void MouseObservable::detach(MouseObserver* observer) {
	m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

void MouseObservable::notifyObservers(MousEvent event, MouseEventData data) {
    for (MouseObserver* observer : m_observers) {
        if (observer) {
            observer->notify(event, data);
        }
    }
}
