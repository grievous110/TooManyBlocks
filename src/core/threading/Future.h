#ifndef TOOMANYBLOCKS_FUTURE_H
#define TOOMANYBLOCKS_FUTURE_H

#include <stddef.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

constexpr uint64_t DEFAULT_TASKCONTEXT = 0;

enum class Executor {
    Worker,
    Main
};

enum class FutureStatus {
    // State allows increasing the dependency count
    Building,
    Finalized,
    Pending,
    Running,
    Completed,
    Failed
};

class FutureBase {
    template <typename T>
    friend class Future;

private:
    virtual void addDependent(const std::shared_ptr<FutureBase>& other) = 0;

    virtual void dependecyFinished() = 0;

public:
    inline static void (*scheduleCallback)(std::unique_ptr<FutureBase>, Executor) = nullptr;

    virtual ~FutureBase() = default;

    virtual uint64_t getContext() const = 0;

    virtual bool isEmpty() const = 0;

    virtual bool isReady() const = 0;

    virtual void execute() = 0;

    virtual void cancel() = 0;
};

/**
 * Advanced future class for async tasks. Once the task finished, the future is no longer responsible
 * for the result. It will just cleanly provide the value to all consumers that hold an future instance
 * pointing to the result value. All futures can only be executed once.
 * 
 * Futures can also be chained. This way it can be guranteed that once one future runs,all dependency futures
 * are ready so await never needs to be called.
 */
template <typename T>
class Future : public FutureBase {
private:
    template <typename U>
    friend class Future;

    struct TaskState {
        std::atomic<FutureStatus> status{FutureStatus::Building};
        Executor executor;
        uint64_t taskContext;

        std::mutex mtx;
        std::condition_variable cv;

        std::atomic<int> unresolvedDeps{0};
        std::vector<std::shared_ptr<FutureBase>> dependents;

        std::function<T()> task;
        std::conditional_t<!std::is_void_v<T>, std::optional<T>, char> value;
        std::exception_ptr exception;
    };

    std::shared_ptr<TaskState> state;

    void completeSuccess(std::conditional_t<std::is_void_v<T>, char, T>&& v = 0) {
        FutureStatus expected = FutureStatus::Running;
        if (!state->status.compare_exchange_strong(expected, FutureStatus::Completed)) {
            return;  // already completed by other means
        }

        std::lock_guard<std::mutex> lock(state->mtx);
        if constexpr (!std::is_void_v<T>) {
            state->value.emplace(std::move(v));
        }

        onCompleted();
    }

    void completeFailure(const std::exception_ptr& e) {
        FutureStatus expected = FutureStatus::Running;
        if (!state->status.compare_exchange_strong(expected, FutureStatus::Failed)) {
            return;  // already completed by other means
        }

        std::lock_guard<std::mutex> lock(state->mtx);
        state->exception = e;

        onCompleted();
    }

    void onCompleted() {
        for (const std::shared_ptr<FutureBase>& dep : state->dependents) {
            // Decrement remaining dependency count
            dep->dependecyFinished();
        }

        state->dependents.clear();
        state->cv.notify_all();
    }

    virtual void addDependent(const std::shared_ptr<FutureBase>& other) override {
        state->dependents.emplace_back(other);
    }

    virtual void dependecyFinished() override {
        if (state->unresolvedDeps.fetch_sub(1) == 1) {
            trySchedule();
        }
    }

    virtual uint64_t getContext() const override {
        return state->taskContext;
    }

    void trySchedule() {
        if (isEmpty()) throw std::runtime_error("Cannot schedule empty future");

        std::lock_guard<std::mutex> lock(state->mtx);
        if (state->unresolvedDeps.load() > 0) {
            return;
        }

        FutureStatus expected = FutureStatus::Finalized;
        if (!state->status.compare_exchange_strong(expected, FutureStatus::Pending)) {
            return;  // already scheduled by someone else or not finalized
        }

        std::unique_ptr<Future<T>> selfRef = std::make_unique<Future<T>>();
        selfRef->state = state;
        FutureBase::scheduleCallback(std::move(selfRef), state->executor);
    }

public:
    Future() noexcept = default;

    Future(std::function<T()> fn, uint64_t taskContext = DEFAULT_TASKCONTEXT, Executor executor = Executor::Worker)
        : state(std::make_shared<TaskState>()) {
        state->task = std::move(fn);
        state->taskContext = taskContext;
        state->executor = executor;
    }

    virtual ~Future() = default;

    void debug() {
        if (state) state->debug.store(true);
    }

    template <typename U>
    inline Future<T>& dependsOn(Future<U> other) {
        if (isEmpty() || other.isEmpty()) throw std::runtime_error("Cannot depend on empty future");

        if (state->status.load() != FutureStatus::Building)
            throw std::runtime_error("Trying to add dependency to a finalized future");

        // Hold both locks to avoid concurrent state changes
        std::lock_guard<std::mutex> lock(state->mtx);
        std::lock_guard<std::mutex> otherLock(other.state->mtx);

        if (other.isReady()) return *this;

        state->unresolvedDeps.fetch_add(1);

        std::shared_ptr<Future<T>> selfRef = std::make_shared<Future<T>>();
        selfRef->state = state;
        other.addDependent(selfRef);

        return *this;
    }

    inline Future<T>& start() {
        if (isEmpty()) throw std::runtime_error("Cannot start empty future");

        // Advance status to Finalized if neeeded
        FutureStatus expected = FutureStatus::Building;
        state->status.compare_exchange_strong(expected, FutureStatus::Finalized);

        trySchedule();
        return *this;
    }

    virtual inline void execute() override {
        if (isEmpty()) throw std::runtime_error("Cannot execute empty future");

        FutureStatus expected = FutureStatus::Pending;
        if (!state->status.compare_exchange_strong(expected, FutureStatus::Running)) {
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

    virtual inline void cancel() override {
        if (isEmpty() || isReady()) return;

        // Force advance status to Running so that completeFailure can run
        FutureStatus expected = FutureStatus::Building;
        state->status.compare_exchange_strong(expected, FutureStatus::Running);
        expected = FutureStatus::Finalized;
        state->status.compare_exchange_strong(expected, FutureStatus::Running);
        expected = FutureStatus::Pending;
        state->status.compare_exchange_strong(expected, FutureStatus::Running);

        completeFailure(std::make_exception_ptr(std::runtime_error("Task canceled")));
    }

    // Caller suspends until future has been executed / finished with error
    void await() {
        if (isEmpty()) return;

        std::unique_lock<std::mutex> lock(state->mtx);
        state->cv.wait(lock, [this] { return isReady(); });
    }

    inline void reset() {
        if (state) state.reset();
    }

    virtual inline bool isEmpty() const override { return !state; }

    virtual inline bool isReady() const {
        if (!state) return false;
        FutureStatus status = state->status.load();
        return status == FutureStatus::Completed || status == FutureStatus::Failed;
    }

    inline bool hasError() const { return state && state->status.load() == FutureStatus::Failed; }

    inline size_t useCount() const { return state ? state.use_count() : 0; }

    template <typename U = T>
    std::enable_if_t<!std::is_void_v<U>, const U&> inline value() const {
        if (!isReady()) throw std::runtime_error("Acessing unfinished or empty future");
        if (hasError()) std::rethrow_exception(state->exception);
        return *state->value;
    }

    template <typename U = T>
    std::enable_if_t<!std::is_void_v<U>, U&> inline value() {
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
