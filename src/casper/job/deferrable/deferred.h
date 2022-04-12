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
                
            template <class A> //, class = std::enable_if<std::is_base_of<A, Arguments<A>>::value>>
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
                    std::function<void(const std::string&)>                                                        try_cancel_on_looper_thread_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_step_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_debug_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_error_;
                    std::function<void(const Deferred<A>*, const std::string&)>                                    on_log_deferred_verbose_;
                    std::function<void(const Deferred<A>*, const uint8_t, const char* const, const std::string&)>  on_log_deferred_;
                    std::function<void(const Tracking&, const int, const char* const, const std::string&)>         on_log_tracking_;
                } Callbacks;
                
                typedef struct {
                    std::function<void(Deferred<A>*)> on_track_;   //!< Owner should track this object.
                    std::function<bool(Deferred<A>*)> is_tracked_; //!< Check if this object is being tracked.
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
                
            private: // TODO:
                
                typedef struct {
                    std::mutex            mutex_;
                    std::set<std::string> callbacks_;
                } Pending;
                
                Pending   pending_;
                
                Callbacks callbacks_;

            protected: // Function Ptrs

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

            protected: // Inline Method(s) / Function(s(
                
                inline void Bind (Callbacks a_callbacks);

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
                
                /**
                 * @brief Override some \link Response \link values.
                 *
                 * @param a_code         HTTP Status Code
                 * @param a_content_type HTTP Content-Type header value.
                 * @param a_body         HTTP response body.
                 * @param a_parse        When true body will be parsed as JSON.
                 */
                inline void OverrideResponse (const uint16_t a_code, const std::string& a_content_type, const std::string& a_body, const bool a_parse = true)
                {
                    response_.Set(a_code, a_content_type, a_body, response_.rtt(), a_parse);
                }
                
                /**
                 * @brief Override some \link Response \link values.
                 *
                 * @param a_code      HTTP Status Code
                 * @param a_exception Exception.
                 */
                inline void OverrideResponse (const uint16_t a_code, const cc::Exception& a_exception)
                {
                    response_.Set(a_code, a_exception);
                }
                
            protected: // API - Method(s) / Function(s)
                
                void OnProgress                 (const Deferred<A>* a_deferred);
                void OnChanged                  (const Deferred<A>* a_deferred);
                void OnCompleted                (const Deferred<A>* a_deferred);
                
                void CallOnMainThread           (std::function<void()> a_function);
                void CallOnMainThreadDeferred   (std::function<void()> a_function, const size_t a_delay);

                void CallOnLooperThread         (const std::string& a_id, std::function<void(const std::string&)> a_function, const bool a_daredevil = false);
                void CallOnLooperThreadDeferred (const std::string& a_id, std::function<void(const std::string&)> a_function, const size_t a_delay, const bool a_daredevil = false);
                void TryCancelOnLooperThread    (const std::string& a_id);

                void OnLogDeferredStep          (const Deferred<A>* a_deferred, const std::string& a_message);
                void OnLogDeferredDebug         (const Deferred<A>* a_deferred, const std::string& a_message);
                void OnLogDeferredError         (const Deferred<A>* a_deferred, const std::string& a_message);
                void OnLogDeferredVerbose       (const Deferred<A>* a_deferred, const std::string& a_message);
                
                void OnLogDeferred              (const Deferred<A>* a_deferred, const uint8_t a_level, const char* const a_step, const std::string& a_message);
                void OnLogTracking              (const Tracking& a_tracking   , const int a_level, const char* const a_step, const std::string& a_message);                
                
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
                std::lock_guard<std::mutex> lock_out(pending_.mutex_);
                for ( const auto& it : pending_.callbacks_ ) {
                    callbacks_.try_cancel_on_looper_thread_(it);
                }
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
             * @brief Set the callbacks.
             *
             * @param a_callbacks See \link Callbacks \link.
             */
            template <class A>
            inline void Deferred<A>::Bind (Deferred<A>::Callbacks a_callbacks)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                CC_DEBUG_ASSERT(nullptr == callbacks_.on_main_thread_);
                callbacks_ = a_callbacks;
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
                return ( nullptr != handler_.is_tracked_ ? handler_.is_tracked_(this) : false );
            }

            // MARK: control

            /**
             * @brief Call this method to report progress for a deferred request.
             *
             * @param a_deferred Deferred request data.
             */
            template <class A>
            inline void Deferred<A>::OnProgress (const Deferred<A>* a_deferred)
            {
                callbacks_.on_progress_(a_deferred);
            }

            /**
             * @brief Call this method to report that a deferred request state changed.
             *
             * @param a_deferred Deferred request data.
             */
            template <class A>
            inline void Deferred<A>::OnChanged (const Deferred<A>* a_deferred)
            {
                callbacks_.on_changed_(a_deferred);
            }

            /**
             * @brief Call this method to report that a deferred request is now completed.
             *
             * @param a_deferred Deferred request data.
             */
            template <class A>
            inline void Deferred<A>::OnCompleted (const Deferred<A>* a_deferred)
            {
                callbacks_.on_completed_(a_deferred);
            }

            // MARK: main

            /**
             * @brief Schedule a callback on 'main' thread.
             *
             * @param a_function Function to call.
             */
            template <class A>
            inline void Deferred<A>::CallOnMainThread (std::function<void()> a_function)
            {
                callbacks_.on_main_thread_(a_function);
            }

            /**
             * @brief Schedule a callback on 'main' thread.
             *
             * @param a_function Function to call.
             * @param a_delay    Amount of time ( in ms ) to delay call, 0 none.
             */
            template <class A>
            inline void Deferred<A>::CallOnMainThreadDeferred (std::function<void()> a_function, const size_t a_delay)
            {
                callbacks_.on_main_thread_deferred_(a_function, a_delay);
            }                

            // MARK: looper

            /**
             * @brief Schedule a callback on 'looper' thread.
             *
             * @param a_id       Callback ID.
             * @param a_function Function to call.
             * @param a_daredevil When true, won't attempt cleanup.
             */
            template <class A>
            inline void Deferred<A>::CallOnLooperThread (const std::string& a_id, std::function<void(const std::string&)> a_function, const bool a_daredevil)
            {
                CallOnLooperThreadDeferred(a_id, a_function, /* a_delay */ 0, a_daredevil);
            }

            /**
             * @brief Schedule a callback on 'looper' thread.
             *
             * @param a_id       Callback ID.
             * @param a_function Function to call.
             * @param a_delay    Amount of time ( in ms ) to delay call, 0 none.
             * @param a_daredevil When true, won't attempt cleanup.
             */
            template <class A>
            inline void Deferred<A>::CallOnLooperThreadDeferred (const std::string& a_id, std::function<void(const std::string&)> a_function, const size_t a_delay, const bool a_daredevil)
            {
                // ... (in)sanity checkpoint ...
                CC_DEBUG_FAIL_IF_NOT_AT_MAIN_THREAD();
                // ... track callback id ...
                bool duplicated = false;
                //..  ⚠️ do not use std::lock_guard here ⚠️ ...
                pending_.mutex_.lock();
                if ( pending_.callbacks_.end() != pending_.callbacks_.find(a_id) ) {
                    duplicated = true;
                } else {
                    pending_.callbacks_.insert(a_id);
                }
                pending_.mutex_.unlock();
                // ... for debug catch ...
                CC_DEBUG_ASSERT(false == duplicated);
                // ... but if in release, do not crash this process just cancel this job deferred action ...
                if ( true == duplicated ) {
                    throw ::cc::InternalServerError("Found duplicated id call looper id %s!", a_id.c_str());
                }
                // ... schedule callback ...
                if ( 0 != a_delay ) {
                    callbacks_.on_looper_thread_deferred_(a_id, [this, a_function, a_daredevil](const std::string& a_id2) {
                        // ... untrack callback id ...
                        if ( false == a_daredevil ) {
                            //..  ⚠️ do not use std::lock_guard here ⚠️ ...
                            pending_.mutex_.lock();
                            const auto it = pending_.callbacks_.find(a_id2);
                            if ( pending_.callbacks_.end() != it ) {
                                pending_.callbacks_.erase(it);
                            }
                            pending_.mutex_.unlock();
                        }
                        // ... perform ...
                        a_function(a_id2);
                    }, a_delay);
                } else {
                    callbacks_.on_looper_thread_(a_id, [this, a_function, a_daredevil](const std::string& a_id2) {
                        // ... untrack callback id ...
                        if ( false == a_daredevil ) {
                            //..  ⚠️ do not use std::lock_guard here ⚠️ ...
                            pending_.mutex_.lock();
                            const auto it = pending_.callbacks_.find(a_id2);
                            if ( pending_.callbacks_.end() != it ) {
                                pending_.callbacks_.erase(it);
                            }
                            pending_.mutex_.unlock();
                        }
                        // ... perform ...
                        a_function(a_id2);
                    });
                }
            }

            /**
             * @brief Try to cancel a previously schedule callback on 'looper' thread.
             *
             * @param a_id Callback ID.
             */
            template <class A>
            inline void Deferred<A>::TryCancelOnLooperThread (const std::string& a_id)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                std::lock_guard<std::mutex> lock(pending_.mutex_);
                // ... untrack of callback id ...
                const auto it = pending_.callbacks_.find(a_id);
                if ( pending_.callbacks_.end() != it ) {
                    pending_.callbacks_.erase(it);
                }
                // ... try ...
                callbacks_.try_cancel_on_looper_thread_(a_id);
            }
        
            // MARK: logging
        
            /**
             * @brief Call this method to log a deferred request step message.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogDeferredStep (const Deferred<A>* a_deferred, const std::string& a_message)
            {
                callbacks_.on_log_deferred_step_(a_deferred, a_message);
            }
            
            /**
             * @brief Call this method to log a deferred request debug message.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogDeferredDebug (const Deferred<A>* a_deferred, const std::string& a_message)
            {
                callbacks_.on_log_deferred_debug_(a_deferred, a_message);
            }
            
            /**
             * @brief Call this method to log a deferred request error message.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogDeferredError (const Deferred<A>* a_deferred, const std::string& a_message)
            {
                callbacks_.on_log_deferred_error_(a_deferred, a_message);
            }
            
            /**
             * @brief Call this method to log a deferred request verbose message.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogDeferredVerbose (const Deferred<A>* a_deferred, const std::string& a_message)
            {
                callbacks_.on_log_deferred_verbose_(a_deferred, a_message);
            }
            
            /**
             * @brief Call this method to log a deferred request message.
             *
             * @param a_deferred Deferred request data.
             * @param a_level    Log level.
             * @param a_step     Log step.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogDeferred (const Deferred<A>* a_deferred, const uint8_t a_level, const char* const a_step, const std::string& a_message)
            {
                callbacks_.on_log_deferred_(a_deferred, a_level, a_step, a_message);
            }
            
            /**
             * @brief Call this method to log a deferred request tracking message..
             *
             * @param a_tracking Request tracking info.
             * @param a_level    Log level.
             * @param a_step     Log step.
             * @param a_message  Message to log.
             */
            template <class A>
            inline void Deferred<A>::OnLogTracking (const Tracking& a_tracking, const int a_level, const char* const a_step, const std::string& a_message)
            {
                callbacks_.on_log_tracking_(a_tracking, a_level, a_step, a_message);
            }
        
        } // end of namespace 'deferrable'
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_DEFERRED_H_
