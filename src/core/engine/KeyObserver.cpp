#include "engine/KeyObserver.h"
#include <algorithm>

void KeyObservable::attach(KeyObserver* observer) {
    m_observers.push_back(observer);
}

void KeyObservable::detach(KeyObserver* observer) {
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
}

void KeyObservable::notifyObservers(KeyEvent event, KeyEventData data) {
    for (KeyObserver* observer : m_observers) {
        if (observer) {
            observer->notify(event, data);
        }
    }
}
