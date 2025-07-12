/* blade/core/event.h
 * 
 * This file contains the implementation of handling events being 
 * dispatched and handled. Events are signals sent from Blade when
 * certain things occur. Multiple places in the engine will send
 * events. The most common being the `blade::window` class.
 *
 * This interface is designed to be extensible allowing for user-defined
 * events. 
 */

#ifndef BLADE_CORE_EVENT_H
#define BLADE_CORE_EVENT_H

#include "core/types.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace blade
{
    namespace events
    {
        template <typename T>
        struct event
        {
            using event_callback = std::function<bool(const T&)>;
        };

        template <typename T>
        concept IsEvent = std::is_base_of_v<event<T>, T>;

        /// @brief The event fired once the signal to shutdown the application
        struct application_quit : public event<application_quit> {}; 
      
        /// @brief Event fired when a window is resized 
        /// @tparam W is the type of the window being sent
        template <typename W>
        struct window_resize : public event<window_resize<W>>
        {
            u32 width {0};
            u32 height {0};
            W& window;

            window_resize(struct width w, struct height h, W& window) 
                : width{w.w}
                , height{h.h}
                , window{window} 
            {}
        };

        struct window_close : public event<window_close> {};

        namespace detail
        {
            template <IsEvent EventType>
            class event_bus
            {
                public:
                    /// @brief Subscribe a handler to events of type T
                    static void subscribe(typename EventType::event_callback handler)
                    {
                        _handlers.push_back(handler);
                    }

                    /// @brief Dispatch events for all handlers of T event type
                    static void dispatch(const EventType& event)
                    {
                        for (const auto& handler : _handlers)
                        {
                            handler(event);
                        }
                    }
                private:
                    static std::vector<typename event<EventType>::event_callback> _handlers;
            };

            template <IsEvent T>
            std::vector<typename event<T>::event_callback> event_bus<T>::_handlers; 
        }

        /// @brief Register a handler to process events of this type
        /// @tparam T Type of event to register callback to handle
        /// @param handler The handler to register
        template <IsEvent T>
        void subscribe(const typename event<T>::event_callback& handler)
        {
            detail::event_bus<T>::subscribe(handler);
        }

        /// @brief Dispatch this event to be processed by registered handlers
        /// @tparam T Type of the Event
        /// @param event Event data to process
        template <IsEvent T>
        void dispatch(const T& event)
        {
            detail::event_bus<T>::dispatch(event);
        }
    }
}

#endif // BLADE_CORE_EVENT_H
