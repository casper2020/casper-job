/**
 * @file dispatcher.h
 *
 * Copyright (c) 2011-2020 Cloudware S.A. All rights reserved.
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
#ifndef CASPER_JOB_DEFERRABLE_DISPATCHER_H_
#define CASPER_JOB_DEFERRABLE_DISPATCHER_H_

#include "cc/non-copyable.h"
#include "cc/non-movable.h"

#include "ev/loggable.h"

#include "cc/debug/types.h"

#include "json/json.h"

#include "casper/job/deferrable/deferred.h"

#include <string>
#include <map>

#include "cc/easy/job/types.h"

namespace casper
{

    namespace job
    {
    
        namespace deferrable
        {
        
            template <class A> class Dispatcher : public ::cc::NonCopyable, public ::cc::NonMovable
            {
                
            public: // Data Type(s)
                
                typedef typename Deferred<A>::Callbacks Callbacks;
                
            protected: // Data Type(s)
                
                typedef std::map<std::string, Deferred<A>*> RunningMap; //!< RCID ( REDIS Channel ID ) -> Deferred<A>

        protected: // Const Data - DEBUG
                
                CC_IF_DEBUG_DECLARE_VAR(const cc::debug::Threading::ThreadID, thread_id_);

            protected: // Function Ptrs
                
                Callbacks callbacks_;
                
            private: // Data
                
                RunningMap running_; //!< Deferred running requests.

            public: // Constructor(s) / Destructor
                
                CC_IF_DEBUG(Dispatcher () = delete;)
                Dispatcher (CC_IF_DEBUG_CONSTRUCT_DECLARE_VAR(const cc::debug::Threading::ThreadID, a_thread_id));
                virtual ~Dispatcher ();
                
            public: // API Method(s) / Function(s)
                
                virtual void Setup (const Json::Value& a_config) = 0;
                virtual void Load  (const bool a_reload = false) { (void)a_reload;};
                
            public: // API - One-shot Call Method(s) / Function(s)
                
                void         Bind    (Callbacks a_callbacks);
                
            protected: // API - One-shot Call Method(s) / Function(s)
                
                void         Bind     (Deferred<A>* a_deferred);
                
            protected: // API - Method(s) / Function(s)
                
                void Dispatch (const A& a_args, Deferred<A>* a_deferred);
                
            }; // end of class 'Dispatcher'
        
            /**
             * @brief Default constructor.
             *
             * param a_thread_id For debug proposes only
             */
            template <class A>
            Dispatcher<A>::Dispatcher (CC_IF_DEBUG_CONSTRUCT_DECLARE_VAR(const cc::debug::Threading::ThreadID, a_thread_id))
                CC_IF_DEBUG(: CC_IF_DEBUG_CONSTRUCT_SET_VAR(thread_id_, a_thread_id))
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
            }

            /**
             * @brief Destructor.
             */
            template <class A>
            Dispatcher<A>::~Dispatcher ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                callbacks_.on_completed_          = nullptr;
                callbacks_.on_main_thread_        = nullptr;
                callbacks_.on_looper_thread_      = nullptr;
                callbacks_.on_log_deferred_step_  = nullptr;
                callbacks_.on_log_tracking_       = nullptr;
            }

            /**
             * @brief Bind callbacks.
             *
             * @param a_callbacks A set of functions to be called by this object.
             */
            template <class A>
            inline void Dispatcher<A>::Bind (Callbacks a_callbacks)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                callbacks_ = a_callbacks;
                // ... forget running activities ...
                for ( auto it : running_ ) {
                    delete it.second;
                }
                running_.clear();
            }
            
            /**
             * @brief Track a deferred request.
             *
             * @param a_deferred Deferred request.
             */
            template <class A>
            inline void Dispatcher<A>::Bind (Deferred<A>* a_deferred)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                a_deferred->Bind({
                    /* on_track_ */ [this] (Deferred<A>* a_deferred) {
                        // ... log ...
                        callbacks_.on_log_tracking_(a_deferred->tracking_, CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATS, "Track  : " + a_deferred->id_);
                        // ... track ...
                        const auto it = running_.find(a_deferred->id_);
                        if ( running_.end() != it ) {
                            throw cc::Exception("Logic error, '%s' already tracked!", a_deferred->id_.c_str());
                        }
                        running_[a_deferred->id_] = a_deferred;
                    },
                    /* is_tracked_ */ [this] (Deferred<A>* a_deferred) -> bool {
                        return ( running_.end() != running_.find(a_deferred->id_) );
                    },
                    /* on_untrack_ */ [this] (Deferred<A>* a_deferred) {
                        // ... log ...
                        callbacks_.on_log_tracking_(a_deferred->tracking_, CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATS, "Untrack: " + a_deferred->id_);
                        // ... untrack ...
                        const auto it = running_.find(a_deferred->id_);
                        if ( running_.end() == it ) {
                            // TODO: review old behaviour was:  throw cc::Exception("Logic error, '%s' not found!", a_deferred->id_.c_str());
                            delete a_deferred;
                        } else {
                            delete it->second;
                            running_.erase(it);
                        }
                    }
                });
            }
        
            /**
             * @brief Track and launch deferred request.
             *
             * @param a_args     Request specific arguments.
             * @param a_deferred Request to run.
             */
            template <class A>
            inline void Dispatcher<A>::Dispatch (const A& a_args, Deferred<A>* a_deferred)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
                try {
                    Bind(a_deferred);
                    a_deferred->Run(a_args, callbacks_);
                } catch (...) {
                    if ( true == a_deferred->Tracked() ) {
                        a_deferred->Untrack();
                    } else {
                        delete a_deferred;
                    }
                    cc::Exception::Rethrow(/* a_unhandled */ false, __FILE__, __LINE__, __FUNCTION__);
                }
            }
        
        } // end of namespace 'deferrable'
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_DISPATCHER_H_
