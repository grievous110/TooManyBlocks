#ifndef TOOMANYBLOCKS_FUTURE_H
#define TOOMANYBLOCKS_FUTURE_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>

/**
 * Advanced future class for async tasks. Once the task finished, the future is no longer responsible
 * for the result. It will just cleanly provide the value to all consumers that hold an future instance
 * pointing to the result value. All futures can only be executed once.
 */
template <typename T>
class Future {
private:
    struct TaskState {
        std::atomic<bool> executed = false;
        std::atomic<bool> ready = false;
        std::atomic<bool> failed = false;

        std::function<T()> task;

        std::mutex mtx;
        std::condition_variable cv;

        std::conditional_t<!std::is_void_v<T>, std::optional<T>, char> value;
        std::exception_ptr exception;
    };

    std::shared_ptr<TaskState> state;

    void completeSuccess(std::conditional_t<std::is_void_v<T>, char, T>&& v = 0) {
        if constexpr (!std::is_void_v<T>) {
            state->value.emplace(std::move(v));
        }
        state->failed.store(false);
        state->ready.store(true);
        state->cv.notify_all();
    }

    void completeFailure(const std::exception_ptr& e) {
        state->exception = e;
        state->failed.store(true);
        state->ready.store(true);
        state->cv.notify_all();
    }

public:
    Future() = default;

    Future(std::function<T()> fn) : state(std::make_shared<TaskState>()) { state->task = std::move(fn); }

    void operator()() {
        if (isEmpty()) throw std::runtime_error("Cannot execute empty future");

        bool expected = false;
        if (!state->executed.compare_exchange_strong(expected, true)) {
            return;  // already executed by someone else
        }

        // Move task out of member into local variable, so captures are released after execute
        std::function<T()> tmpTask = std::move(state->task);
        state->task = {};  // Force release of captures

        try {
            if constexpr (std::is_void_v<T>) {
                tmpTask();
                completeSuccess();
            } else {
                T result = tmpTask();
                completeSuccess(std::move(result));
            }
        } catch (...) {
            completeFailure(std::current_exception());
        }
    }

    inline bool isEmpty() const { return !state; }

    inline bool isReady() const { return state && state->ready.load(); }

    inline bool hasError() const { return state && state->failed.load(); }

    inline void await() const {
        if (isEmpty()) throw std::runtime_error("Cannot await empty future");

        std::unique_lock lock(state->mtx);
        state->cv.wait(lock, [this] { return state->ready.load(); });
    }

    template <typename U = T>
    std::enable_if_t<!std::is_void_v<U>, const U&> value() const {
        if (!isReady()) throw std::runtime_error("Acessing unfinished or empty future");
        if (hasError()) std::rethrow_exception(state->exception);
        return *state->value;
    }

    template <typename U = T>
    std::enable_if_t<!std::is_void_v<U>, const U&> value() {
        if (!isReady()) throw std::runtime_error("Acessing unfinished or empty future");
        if (hasError()) std::rethrow_exception(state->exception);
        return *state->value;
    }

    inline std::exception_ptr getException() const {
        if (!hasError()) return nullptr;
        return state->exception;
    }
};

#endif