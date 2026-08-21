#pragma once

#include "events/Event.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <mutex>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace cloud9 {

using SubscriptionId = std::uint64_t;

class EventManager {
public:
    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    template <typename EventType, typename Callback>
    SubscriptionId subscribe(Callback&& callback, int priority = 0) {
        static_assert(std::is_base_of_v<Event, EventType>, "EventType must derive from Event");
        Subscription subscription;
        subscription.priority = priority;
        subscription.callback = [handler = std::forward<Callback>(callback)](Event& event) mutable {
            handler(static_cast<EventType&>(event));
        };
        std::lock_guard lock(mutex_);
        const SubscriptionId id = nextId_++;
        subscription.id = id;
        auto& bucket = subscriptions_[std::type_index(typeid(EventType))];
        bucket.push_back(std::move(subscription));
        std::stable_sort(bucket.begin(), bucket.end(), [](const Subscription& left, const Subscription& right) {
            return left.priority > right.priority;
        });
        return id;
    }

    void unsubscribe(SubscriptionId id);

    template <typename EventType>
    void publish(EventType& event) {
        static_assert(std::is_base_of_v<Event, EventType>, "EventType must derive from Event");
        std::vector<Subscription> callbacks;
        {
            std::lock_guard lock(mutex_);
            const auto it = subscriptions_.find(std::type_index(typeid(EventType)));
            if (it != subscriptions_.end()) callbacks = it->second;
        }
        for (auto& subscription : callbacks) {
            subscription.callback(event);
            if (event.cancelled) break;
        }
    }

    void clear();
    [[nodiscard]] std::size_t subscriptionCount() const;

private:
    struct Subscription {
        SubscriptionId id{0};
        int priority{0};
        std::function<void(Event&)> callback;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::type_index, std::vector<Subscription>> subscriptions_;
    SubscriptionId nextId_{1};
};

} // namespace cloud9
