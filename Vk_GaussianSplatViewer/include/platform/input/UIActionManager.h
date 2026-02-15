#pragma once

#include "enums/inputs/UIAction.h"
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <variant>

namespace ui
{
        // Callback types for different UI interactions
    using UIActionCallback = std::function<void()>;
    using UIBoolCallback = std::function<void(bool)>;
    using UIIntCallback = std::function<void(int)>;
    using UIFloatCallback = std::function<void(float)>;
    using UIStringCallback = std::function<void(const std::string&)>;

    class UIActionManager
    {
    public:
        UIActionManager() = default;
        ~UIActionManager() = default;

        // ===== Action callback registration =====

        // Register a simple button/action callback
        void register_action(UIAction action, const UIActionCallback& callback);

        // Register a checkbox/toggle callback (receives bool state)
        void register_bool_action(UIAction action, const UIBoolCallback& callback);

        // Register a slider/spinner callback (receives int value)
        void register_int_action(UIAction action, const UIIntCallback& callback);

        // Register a slider callback (receives float value)
        void register_float_action(UIAction action, const UIFloatCallback& callback);

        // Register a text input callback (receives string value)
        void register_string_action(UIAction action, const UIStringCallback& callback);

        // ===== Unregister actions =====

        void unregister_action(UIAction action);
        void unregister_bool_action(UIAction action);
        void unregister_int_action(UIAction action);
        void unregister_float_action(UIAction action);
        void unregister_string_action(UIAction action);

        // ===== Queue actions (deferred execution) =====

        // Queue a simple action (for buttons, menu items, etc.)
        void queue_action(UIAction action);

        // Queue a bool action (for checkboxes, toggles)
        void queue_bool_action(UIAction action, bool value);

        // Queue an int action (for sliders, spinners)
        void queue_int_action(UIAction action, int value);

        // Queue a float action (for sliders)
        void queue_float_action(UIAction action, float value);

        // Queue a string action (for text inputs)
        void queue_string_action(UIAction action, const std::string& value);

        // ===== Process queued actions =====

        // Process all queued actions (call this at a safe point in your frame)
        void process_queued_actions();

        // Clear all queued actions without executing them
        void clear_queued_actions();

        // Check if there are any queued actions
        [[nodiscard]] bool has_queued_actions() const;

        // Get the number of queued actions
        [[nodiscard]] size_t get_queued_action_count() const;

        // ===== Query if actions are registered =====

        [[nodiscard]] bool has_action(UIAction action) const;
        [[nodiscard]] bool has_bool_action(UIAction action) const;
        [[nodiscard]] bool has_int_action(UIAction action) const;
        [[nodiscard]] bool has_float_action(UIAction action) const;
        [[nodiscard]] bool has_string_action(UIAction action) const;

        // ===== Clear all actions =====

        void clear_all_actions();

    private:
        // Callback storage for different types
        std::unordered_map<UIAction, std::vector<UIActionCallback>> action_callbacks;
        std::unordered_map<UIAction, std::vector<UIBoolCallback>> bool_callbacks;
        std::unordered_map<UIAction, std::vector<UIIntCallback>> int_callbacks;
        std::unordered_map<UIAction, std::vector<UIFloatCallback>> float_callbacks;
        std::unordered_map<UIAction, std::vector<UIStringCallback>> string_callbacks;

        // Queued action types
        struct QueuedAction
        {
            UIAction action;
        };

        struct QueuedBoolAction
        {
            UIAction action;
            bool value;
        };

        struct QueuedIntAction
        {
            UIAction action;
            int value;
        };

        struct QueuedFloatAction
        {
            UIAction action;
            float value;
        };

        struct QueuedStringAction
        {
            UIAction action;
            std::string value;
        };

        // Queue storage using variant
        using QueuedActionVariant = std::variant<
            QueuedAction,
            QueuedBoolAction,
            QueuedIntAction,
            QueuedFloatAction,
            QueuedStringAction
        >;

        std::queue<QueuedActionVariant> action_queue;

        // Helper method to execute a queued action
        void execute_queued_action(const QueuedActionVariant& queued_action);
    };
} // ui
