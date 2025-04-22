#ifndef TOOMANYBLOCKS_OBSERVER_H
#define TOOMANYBLOCKS_OBSERVER_H

#include <algorithm>
#include <vector>

template <typename... Args>
class Observer {
public:
    virtual ~Observer() = default;
    virtual void notify(Args... args) = 0;
};

template <typename... Args>
class Observable {
private:
    std::vector<Observer<Args...>*> m_observers;

public:
    virtual ~Observable() = default;

    void attach(Observer<Args...>* observer) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
            m_observers.push_back(observer);
        }
    }

    void detach(Observer<Args...>* observer) {
        m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer), m_observers.end());
    }

    void notifyObservers(Args... args) {
        for (auto observer : m_observers) {
            observer->notify(args...);
        }
    }
};

#endif