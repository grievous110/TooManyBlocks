#ifndef KEYOBSERVER_H
#define KEYOBSERVER_H

#include <vector>

enum class KeyEvent {
	ButtonDown,
	ButtonUp
};

struct KeyEventData {
	int keycode;
	int mods;
};

class KeyObserver {
public:
	virtual void notify(KeyEvent event, KeyEventData data) = 0;
	virtual ~KeyObserver() = default;
};

class KeyObservable {
private:
	std::vector<KeyObserver*> m_observers;
public:
	virtual void attach(KeyObserver* observer);
	virtual void detach(KeyObserver* observer);
	virtual void notifyObservers(KeyEvent event, KeyEventData data);
};

#endif