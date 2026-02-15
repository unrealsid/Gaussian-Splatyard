#include "platform/input/UIActionManager.h"

namespace ui
{
    // ===== Register actions =====

    void UIActionManager::register_action(UIAction action, const UIActionCallback& callback)
    {
        action_callbacks[action].push_back(callback);
    }

    void UIActionManager::register_bool_action(UIAction action, const UIBoolCallback& callback)
    {
        bool_callbacks[action].push_back(callback);
    }

    void UIActionManager::register_int_action(UIAction action, const UIIntCallback& callback)
    {
        int_callbacks[action].push_back(callback);
    }

    void UIActionManager::register_float_action(UIAction action, const UIFloatCallback& callback)
    {
        float_callbacks[action].push_back(callback);
    }

    void UIActionManager::register_string_action(UIAction action, const UIStringCallback& callback)
    {
        string_callbacks[action].push_back(callback);
    }

    // ===== Unregister actions =====

    void UIActionManager::unregister_action(UIAction action)
    {
        action_callbacks.erase(action);
    }

    void UIActionManager::unregister_bool_action(UIAction action)
    {
        bool_callbacks.erase(action);
    }

    void UIActionManager::unregister_int_action(UIAction action)
    {
        int_callbacks.erase(action);
    }

    void UIActionManager::unregister_float_action(UIAction action)
    {
        float_callbacks.erase(action);
    }

    void UIActionManager::unregister_string_action(UIAction action)
    {
        string_callbacks.erase(action);
    }

    // ===== Queue actions (deferred execution) =====

    void UIActionManager::queue_action(UIAction action)
    {
        action_queue.push(QueuedAction(action));
    }

    void UIActionManager::queue_bool_action(UIAction action, bool value)
    {
        action_queue.push(QueuedBoolAction(action, value));
    }

    void UIActionManager::queue_int_action(UIAction action, int value)
    {
        action_queue.push(QueuedIntAction(action, value));
    }

    void UIActionManager::queue_float_action(UIAction action, float value)
    {
        action_queue.push(QueuedFloatAction(action, value));
    }

    void UIActionManager::queue_string_action(UIAction action, const std::string& value)
    {
        action_queue.push(QueuedStringAction(action, value));
    }

    // ===== Process queued actions =====

    void UIActionManager::process_queued_actions()
    {
        while (!action_queue.empty())
        {
            execute_queued_action(action_queue.front());
            action_queue.pop();
        }
    }

    void UIActionManager::clear_queued_actions()
    {
        while (!action_queue.empty())
        {
            action_queue.pop();
        }
    }

    bool UIActionManager::has_queued_actions() const
    {
        return !action_queue.empty();
    }

    size_t UIActionManager::get_queued_action_count() const
    {
        return action_queue.size();
    }

    // ===== Execute queued action helper =====

    void UIActionManager::execute_queued_action(const QueuedActionVariant& queued_action)
    {
        std::visit([this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, QueuedAction>)
            {
                auto it = action_callbacks.find(arg.action);
                if (it != action_callbacks.end())
                {
                    for (auto& callback : it->second)
                    {
                        callback();
                    }
                }
            }
            else if constexpr (std::is_same_v<T, QueuedBoolAction>)
            {
                auto it = bool_callbacks.find(arg.action);
                if (it != bool_callbacks.end())
                {
                    for (auto& callback : it->second)
                    {
                        callback(arg.value);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, QueuedIntAction>)
            {
                auto it = int_callbacks.find(arg.action);
                if (it != int_callbacks.end())
                {
                    for (auto& callback : it->second)
                    {
                        callback(arg.value);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, QueuedFloatAction>)
            {
                auto it = float_callbacks.find(arg.action);
                if (it != float_callbacks.end())
                {
                    for (auto& callback : it->second)
                    {
                        callback(arg.value);
                    }
                }
            }
            else if constexpr (std::is_same_v<T, QueuedStringAction>)
            {
                auto it = string_callbacks.find(arg.action);
                if (it != string_callbacks.end())
                {
                    for (auto& callback : it->second)
                    {
                        callback(arg.value);
                    }
                }
            }
        }, queued_action);
    }

    // ===== Query if actions are registered =====

    bool UIActionManager::has_action(UIAction action) const
    {
        return action_callbacks.find(action) != action_callbacks.end();
    }

    bool UIActionManager::has_bool_action(UIAction action) const
    {
        return bool_callbacks.contains(action);
    }

    bool UIActionManager::has_int_action(UIAction action) const
    {
        return int_callbacks.contains(action);
    }

    bool UIActionManager::has_float_action(UIAction action) const
    {
        return float_callbacks.contains(action);
    }

    bool UIActionManager::has_string_action(UIAction action) const
    {
        return string_callbacks.contains(action);
    }

    // ===== Clear all actions =====

    void UIActionManager::clear_all_actions()
    {
        action_callbacks.clear();
        bool_callbacks.clear();
        int_callbacks.clear();
        float_callbacks.clear();
        string_callbacks.clear();
        clear_queued_actions();
    }
} // namespace ui