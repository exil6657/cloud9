#include "events/EventManager.h"

namespace cloud9 {

void EventManager::unsubscribe(SubscriptionId id) {
    std::lock_guard lock(mutex_);
    for (auto it = subscriptions_.begin(); it != subscriptions_.end();) {
        auto& bucket = it->second;
        bucket.erase(std::remove_if(bucket.begin(), bucket.end(), [id](const Subscription& subscription) {
            return subscription.id == id;
        }), bucket.end());
        if (bucket.empty()) it = subscriptions_.erase(it);
        else ++it;
    }
}

void EventManager::clear() {
    std::lock_guard lock(mutex_);
    subscriptions_.clear();
}

std::size_t EventManager::subscriptionCount() const {
    std::lock_guard lock(mutex_);
    std::size_t count = 0;
    for (const auto& [_, bucket] : subscriptions_) count += bucket.size();
    return count;
}

} // namespace cloud9
