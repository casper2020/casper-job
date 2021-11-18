/**
 * @file deferred.h
 *
 * Copyright (c) 2011-2021 Cloudware S.A. All rights reserved.
 *
 * This file is part of casper-job.
 *
 * casper-job is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * casper-job  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with casper-job. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#ifndef CASPER_JOB_DEFERRABLE_DEFERRED_H_
#define CASPER_JOB_DEFERRABLE_DEFERRED_H_

#include "cc/non-copyable.h"
#include "cc/non-movable.h"

#include "cc/debug/types.h"

#include "cc/exception.h"

#include "casper/job/deferrable/arguments.h"
#include "casper/job/deferrable/types.h"

#include "json/json.h"

#include <functional> // std::function

namespace casper
{

    namespace job
    {
    
        namespace deferrable
        {
                
            template <class A> // , class = std::enable_if<std::is_base_of<A, Arguments<A>>::value>>
            class Deferred : public ::cc::NonCopyable, public ::cc::NonMovable
            {

            public: // Data Type(s)
                
                typedef struct
                {
                    std::function<void(const Deferred<A>*)>                                                        on_progress_;
                    std::function<void(const Deferred<A>*)>                                                        on_changed_;
                    std::function<void(const Deferred<A>*)>                                                        on_completed_;
                    std::function<void(std::function<void()>)>                                                     on_main_thread_;
                    std::function<void(std::function<void()>, const size_t)>                                       on_main_thread_deferred_;
                    std::function<void(const std::string&, std::function<void(const std::string&)>)>               on_looper_thread_;
                    std::function<void(const std::string&, std::function<void(const std::string&)>, const size_t)> on_looper_thread_deferred_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_step_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_debug_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_error_;
                    std::function<void(const Tracking&, const int, const char* const, const std::string&)>         on_log_tracking_;
                } Callbacks;
                
                typedef struct {
                    std::function<void(Deferred<A>*)> on_track_;   //!< Owner should track this object.
                    std::function<void(Deferred<A>*)> on_untrack_; //!< Owner should untrack and dispose this object.
                } LifeCycleHandler;
                
            public: // Const Data
                
                const std::string id_;
                const Tracking    tracking_;

            protected: // Const Data - DEBUG
                
                CC_IF_DEBUG_DECLARE_VAR(const cc::debug::Threading::ThreadID, thread_id_);

            protected: // Data

                A*             arguments_;
                Response       response_;

            protected: // Function Ptrs
                
                Callbacks        callbacks_;
                LifeCycleHandler handler_;

            public: // Constructor(s) / Destructor
                
                Deferred () = delete;
                Deferred(const std::string& a_id, const Tracking&
                         CC_IF_DEBUG_CONSTRUCT_APPEND_VAR(const cc::debug::Threading::ThreadID, a_thread_id));
                virtual ~Deferred ();
                
            public: // Method(s) / Function(s)
                
                virtual void Run (const A& a_args, Callbacks a_callbacks) = 0;
                
            public: // API Method(s) / Function(s)
                
                void Bind       (LifeCycleHandler a_handler);

                void Track     ();
                void Untrack   ();
                bool Tracked   ();

            public: // Inline Method(s) / Function(s)

                /**
                 * @return R/O access to \link A \link.
                 */
                inline const A& arguments () const
                {
                    return *arguments_;
                }
                
                /**
                 * @return R/O access to \link A \link.
                 */
                inline A& arguments ()
                {
                    return *arguments_;
                }

                /**
                 * @return R/O access to \link Response \link.
                 */
                inline const Response& response () const
                {
                    return response_;
                }
                
            }; // end of class 'Deferred'
        
            /**
             * @brief Default constructor.
             *
             * @param a_tracking Request tracking info.
             */
            template <class A>
            Deferred<A>::Deferred (const std::string& a_id, const Tracking& a_tracking
                                   CC_IF_DEBUG_CONSTRUCT_APPEND_VAR(const cc::debug::Threading::ThreadID, a_thread_id))
                : id_(a_id.length() > 0 ? a_id : a_tracking.rcid_),
                  tracking_(a_tracking)
                  CC_IF_DEBUG_CONSTRUCT_APPEND_SET_VAR(thread_id_, a_thread_id,)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                arguments_                   = nullptr;

                handler_.on_track_           = nullptr;
                handler_.on_untrack_         = nullptr;
            }

            /**
             * @brief Destructor.
             */
            template <class A>
            Deferred<A>::~Deferred ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                if ( nullptr != arguments_ ) {
                    delete arguments_;
                }
            }
                        
            /**
             * @brief Set the lifecycle handler.
             *
             * @param a_handle See \link LifeCycleHandler \link.
             */
            template <class A>
            inline void Deferred<A>::Bind (Deferred<A>::LifeCycleHandler a_handler)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                handler_ = a_handler;
            }
        
            /**
             * @brief Request to be tracked;
             */
            template <class A>
            inline void Deferred<A>::Track ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                handler_.on_track_(this);
            }
        
            /**
             * @brief Request to be untracked ( and disposed );
             */
            template <class A>
            inline void Deferred<A>::Untrack ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                handler_.on_untrack_(this);
            }
        
            /**
             * @return True if is being tracked, false otherwise.
             */
            template <class A>
            inline bool Deferred<A>::Tracked ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                return ( nullptr != handler_.on_untrack_);
            }
                                                    
        } // end of namespace 'deferrable'
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_DEFERRED_H_
