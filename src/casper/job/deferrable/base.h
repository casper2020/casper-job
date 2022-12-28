/**
 * @file base.h
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
#ifndef CASPER_JOB_DEFERRABLE_BASE_H_
#define CASPER_JOB_DEFERRABLE_BASE_H_

#include "casper/job/base.h"

#include "casper/job/deferrable/deferred.h"
#include "casper/job/deferrable/dispatcher.h"

#include "cc/exception.h"
#include "cc/i18n/singleton.h"

namespace casper
{

    namespace job
    {
    
        namespace deferrable
        {
        
            template <class A, typename S, S doneValue>
            class Base : public casper::job::Base<S, doneValue>
            {
                
            private: // Const Data
                
                const std::string abbr_;
                
            private: // Alias
                
                using DeferrableBaseClassAlias = ::casper::job::Base<S, doneValue>;
                
#define CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, a_step, a_format, ...) \
    __CASPER_JOB(a_level, a_tracking.bjid_, \
                CC_JOB_LOG_COLOR(WHITE) "%-8.8s" CC_LOGS_LOGGER_RESET_ATTRS ": %-7.7s, " a_format, \
                "DEFERRED", a_step, __VA_ARGS__ \
    );
                
            protected: // Data Type(s)
                
                typedef struct {
                    deferrable::Dispatcher<A>*                                            dispatcher_;
                    std::function<uint16_t(const deferrable::Deferred<A>*, Json::Value&)> on_deferred_request_completed_;
                    std::function<uint16_t(const deferrable::Deferred<A>*, Json::Value&)> on_deferred_request_failed_;
                } D;
                
            protected: // Helper(s)
                
                D d_;
                
            private: // Data
                
                const bool sequentiable_;

            public: // Constructor(s) / Destructor
                
                Base () = delete;
                Base (const std::string& a_abbr, const std::string& a_tube,
                      const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config,
                      const bool a_sequentiable = true);
                virtual ~Base ();
                
            public: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Runnable

                virtual void Setup ();
                virtual void Run  (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response);

            protected: // Virtual Method(s) / Function(s)
                
                virtual void InnerSetup   () = 0;
                virtual void InnerRun     (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response) = 0;
                virtual void InnerCleanUp () {}

            protected: // Method(s) / Function(s) - Callbacks
                
                void OnMainThread                  (std::function<void()> a_callback);
                void OnMainThreadDelayed           (std::function<void()> a_callback, const size_t a_delay);
                void OnLooperThread                (const std::string& a_id, std::function<void(const std::string&)> a_callback);
                void OnLooperThreadDelayed         (const std::string& a_id, std::function<void(const std::string&)> a_callback, const size_t a_delay);
                void TryCancelOnLooperThread       (const std::string& a_id);

                void OnDeferredRequestCompleted  (const deferrable::Deferred<A>* a_deferred);
                void OnDeferredRequestFailed     (const deferrable::Deferred<A>* a_deferred, Json::Value& o_response);
                void OnDeferredRequestLogStep    (const deferrable::Deferred<A>* a_deferred, const std::string& o_payload);
                void OnDeferredRequestLogDebug   (const deferrable::Deferred<A>* a_deferred, const std::string& o_payload);
                void OnDeferredRequestLogError   (const deferrable::Deferred<A>* a_deferred, const std::string& o_payload);
                void OnDeferredRequestLogVerbose (const deferrable::Deferred<A>* a_deferred, const std::string& o_payload);
                void OnDeferredRequestLog        (const deferrable::Deferred<A>*,const size_t, const char* const, const std::string&);
                
                void OnDeferredRequestLogTracking  (const Tracking& a_tracking, const size_t a_level, const char* const a_step, const std::string& a_message);
                
            protected: // Method(s) / Function(s)

                void Publish (const uint64_t& a_id, const std::string& a_rcid, const std::string& a_rjid,
                              const S& a_step, const cc::easy::job::Job::Status& a_status,
                              const ::cc::easy::job::I18N a_i18n);
                
                void Publish (const uint64_t& a_id, const std::string& a_rcid, const std::string& a_rjid,
                              const float& a_percentage, const cc::easy::job::Job::Status& a_status,
                              const ::cc::easy::job::I18N a_i18n);

                void HandleDeferredRequestCompletion (const deferrable::Deferred<A>* a_deferred, std::function<uint16_t(Json::Value&)> a_callback, const Tracking& a_tracking);

            protected: // Method(s) / Function(s) - Helpers

                void SetDeferredRequestFailed   (const std::string& a_dpid, const deferrable::Response& a_response, const ::cc::Exception* a_exception, Json::Value& o_payload);
                void LogDeferredRequestMessage  (const std::string& a_dpid, const size_t a_level, const deferrable::Tracking& a_tracking, const std::string& a_message);
                void LogDeferredRequestResponse (const std::string& a_dpid, const deferrable::Tracking& a_tracking, const deferrable::Response& a_response);
                
            }; // end of class 'Job'

            /**
             * @brief Default constructor.
             *
             * param a_abbr
             * param a_tube
             * param a_loggable_data
             * param a_config
             * param a_sequentiable
             */
            template <class A, typename S, S doneValue>
            casper::job::deferrable::Base<A, S, doneValue>::Base::Base (const std::string& a_abbr, const std::string& a_tube,
                                                                        const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config,
                                                                        const bool a_sequentiable)
                : DeferrableBaseClassAlias(a_tube, a_loggable_data, a_config),
                  abbr_(a_abbr),
                   d_({
                        /* dispatcher_                    */ nullptr,
                        /* on_deferred_request_completed_ */ nullptr,
                        /* on_deferred_request_failed_    */ nullptr
                   }),
                   sequentiable_(a_sequentiable)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
            }

            /**
             * @brief Destructor
             */
            template <class A, typename S, S doneValue>
            casper::job::deferrable::Base<A, S, doneValue>::Base::~Base ()
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                if ( nullptr != d_.dispatcher_ ) {
                    delete d_.dispatcher_;
                }
            }

            // MARK: -

            /**
             * @brief One-shot initialization.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::Setup ()
            {
                DeferrableBaseClassAlias::Setup();
                
                // ... sanity check ...
                CC_ASSERT(nullptr != d_.dispatcher_);
                CC_ASSERT(nullptr != d_.on_deferred_request_completed_);

                //
                // DISPATCHER setup
                //
                d_.dispatcher_->Setup(DeferrableBaseClassAlias::config_.other());
                d_.dispatcher_->Bind({
                    /* on_changed_                  */ nullptr,
                    /* on_progress_                 */ nullptr,
                    /* on_completed_                */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestCompleted, this, std::placeholders::_1),
                    /* on_main_thread_              */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnMainThread, this, std::placeholders::_1),
                    /* on_main_thread_deferred_     */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnMainThreadDelayed, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_looper_thread_            */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnLooperThread, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_looper_thread_deferred_   */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnLooperThreadDelayed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                    /* try_cancel_on_looper_thread_ */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::TryCancelOnLooperThread, this, std::placeholders::_1),
                    /* on_log_deferred_step_        */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogStep, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_log_deferred_debug_       */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogDebug, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_log_deferred_error_       */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogError, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_log_deferred_verbose_     */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogVerbose, this, std::placeholders::_1, std::placeholders::_2),
                    /* on_log_deferred_             */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
                    /* on_log_tracking_             */ std::bind(&casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogTracking, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
                });
            }
        
            /**
             * @brief Process a job to this tube.
             *
             * @param a_id      Job ID.
             * @param a_payload Job payload.
             *
             * @param o_response JSON object.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::Run (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response)
            {
                // ... sanity check ...
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);

                CC_DEBUG_ASSERT(nullptr != d_.dispatcher_);
                CC_DEBUG_ASSERT(nullptr != d_.on_deferred_request_completed_);

                Json::FastWriter jfw; jfw.omitEndingLineFeed();
                
                // ... log request ...
                if ( ::casper::job::Basic<S>::config_.log_redact() ) {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_IN,
                                   "Payload: " SIZET_FMT " byte(s)", jfw.write(a_payload).size()
                    );
                } else {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_IN,
                                   "Payload: %s", jfw.write(a_payload).c_str()
                    );
                }
                

                // ... one-shot call ensured by dispatcher: load additional configs from dispatcher ...
                d_.dispatcher_->Load();

                try {
                    // ... pre-run clean up ..
                    InnerCleanUp();
                    // ... run ...
                    InnerRun(a_id, a_payload, o_response);
                    // ... post-run clean up ..
                    InnerCleanUp();
                } catch (const ::cc::CodedException& a_coded_exception) {
                    // ... post-failure clean up ..
                    InnerCleanUp();
                    // ... error ...
                    try {
                        o_response.code_ = a_coded_exception.code_;
                        const ::cc::easy::JSON<::cc::Exception> json; json.Parse(a_coded_exception.what(), o_response.payload_);
                    } catch(...) {
                        o_response.code_ = DeferrableBaseClassAlias::SetError(a_coded_exception.code_, /* a_i18n */ nullptr,
                                                    /* a_error */ ::cc::easy::job::InternalError{
                                                        /* code_ */ nullptr,
                                                        /* why_ */ a_coded_exception.what()
                                                    }, o_response.payload_
                        );
                    }
                } catch (const deferrable::BadRequestException& a_br_exception) {
                    // ... post-failure clean up ..
                    InnerCleanUp();
                    // ... parsing error ...
                    o_response.code_ = DeferrableBaseClassAlias::SetBadRequest(/* a_i18n */ &DeferrableBaseClassAlias::I18NError(),
                                                                               /* a_error */ {
                                                                                    /* code_ */ nullptr,
                                                                                    /* why_  */ std::string(a_br_exception.what())
                                                                                },
                                                                                o_response.payload_
                    );
                } catch (const ::cc::Exception& a_cc_exception) {
                    // ... post-failure clean up ..
                    InnerCleanUp();
                    // ... parsing error ...
                    o_response.code_ = DeferrableBaseClassAlias::SetInternalServerError(/* a_i18n */ &DeferrableBaseClassAlias::I18NError(),
                                                                                            /* a_error */ {
                                                                                                /* code_ */ nullptr,
                                                                                                /* why_  */ ( "An error occurred while preparing dispatcher: " + std::string(a_cc_exception.what()))
                                                                                            },
                                                                                            o_response.payload_
                    );
                } catch (...) {
                    // ... post-failure clean up ..
                    InnerCleanUp();
                    try {
                        ::cc::Exception::Rethrow(/* a_unhandled */ true, __FILE__, __LINE__, __FUNCTION__);
                    } catch (::cc::Exception& a_cc_exception) {
                        // ... parsing error ...
                        o_response.code_ = DeferrableBaseClassAlias::SetInternalServerError(/* a_i18n */ &DeferrableBaseClassAlias::I18NError(),
                                                                                                /* a_error */ {
                                                                                                    /* code_ */ nullptr,
                                                                                                    /* why_  */ ( "An error occurred while preparing dispatcher: " + std::string(a_cc_exception.what()))
                                                                                                },
                                                                                                o_response.payload_
                        );
                    }
                }
                    
                // ... log ....
                if ( CC_STATUS_CODE_OK == o_response.code_ ) {
                    // ... insanity checkpoint ...
                    CC_ASSERT(true == DeferrableBaseClassAlias::Deferred());
                    // ... status ...
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATUS,
                                   "%s",
                                   "Deferred"
                    );
                } else {
                    
                    // ... response ...
                    if ( true == DeferrableBaseClassAlias::config_.log_redact() ) {
                        CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                       "Response: " CC_JOB_LOG_COLOR(RED) SIZET_FMT " byte(s)" CC_LOGS_LOGGER_RESET_ATTRS,
                                       jfw.write(o_response.payload_).size()
                        );
                    } else {
                        CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                       "Response: " CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                       jfw.write(o_response.payload_).c_str()
                        );
                    }
                    // ... status ...
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                   CC_JOB_LOG_COLOR(LIGHT_RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   "Failed"
                    );
                }
            }

            // MARK: -  Deferred::Callbacks.

            /**
             * @brief Schedule a callback on 'main' thread.
             *
             * @param a_callback Function to call.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnMainThread (std::function<void()> a_callback)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_); // OPTIONAL CHECK
                DeferrableBaseClassAlias::ExecuteOnMainThread(a_callback, /* a_blocking */ false);
            }
        
            /**
             * @brief Schedule a callback on 'main' thread.
             *
             * @param a_callback Function to call.
             * @param a_delay    Delay in ms.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnMainThreadDelayed (std::function<void()> a_callback, const size_t a_delay)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_); // OPTIONAL CHECK
                DeferrableBaseClassAlias::ScheduleOnMainThread(a_callback, a_delay);
            }

            /**
             * @brief Schedule a callback on 'looper' thread.
             *
             * @param a_id       UNIQUE ID.
             * @param a_callback Function to call.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnLooperThread (const std::string& a_id, std::function<void(const std::string&)> a_callback)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_MAIN_THREAD(); // MANDATORY CHECK
                DeferrableBaseClassAlias::ScheduleCallbackOnLooperThread(a_id, a_callback);
            }
            
            /**
             * @brief Schedule a callback on 'looper' thread deferred.
             *
             * @param a_id       UNIQUE ID.
             * @param a_callback Function to call.
             * @param a_delay    Delay in ms.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnLooperThreadDelayed (const std::string& a_id, std::function<void(const std::string&)> a_callback, const size_t a_delay)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_MAIN_THREAD(); // MANDATORY CHECK
                DeferrableBaseClassAlias::ScheduleCallbackOnLooperThread(a_id, a_callback, a_delay);
            }
        
            /**
             * @brief Try to cancel a previously scheduled callback on 'looper' thread.
             *
             * @param a_id UNIQUE ID.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::TryCancelOnLooperThread (const std::string& a_id)
            {
                // can be called from any thread
                DeferrableBaseClassAlias::TryCancelCallbackOnLooperThread(a_id);
            }

            // MARK: -

            /**
             * @brief Called by a 'deferred' request when it's completed.
             *
             * @param a_deferred Deferred request data.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestCompleted (const deferrable::Deferred<A>* a_deferred)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                
                //
                // ... log response?
                //
                // TODO: review the need for this ( @ log )
                // ... only if it's not sequentiable ...
                if ( true == sequentiable_ ) {
                    LogDeferredRequestResponse(abbr_.c_str(), a_deferred->tracking_, a_deferred->response());
                }
                
                //
                // ... process response ...
                //
                HandleDeferredRequestCompletion(a_deferred,
                                                [this, a_deferred](Json::Value& o_payload) -> uint16_t {
                                                    // ... success?
                                                    if ( CC_STATUS_CODE_OK == a_deferred->response().code() && nullptr == a_deferred->response().exception() ) {
                                                        // ... process ...
                                                        return d_.on_deferred_request_completed_(a_deferred, o_payload);
                                                    } else {
                                                        // ... performed, but an error ocurred ...
                                                        if ( nullptr != d_.on_deferred_request_failed_ ) {
                                                            // ... use job specific failure handler ...
                                                            d_.on_deferred_request_failed_(a_deferred, o_payload);
                                                        } else {
                                                            // ... use default failure handler ...
                                                            OnDeferredRequestFailed(a_deferred, o_payload);
                                                        }
                                                    }
                                                    // ... finalize ...
                                                    return a_deferred->response().code();
                                                }, a_deferred->tracking_
                );
            }

            /**
             * @brief Called whan a 'deferred' request failed.
             *
             * @param a_deferred Deferred request data.
             * @param o_payload  Response payload to fill.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestFailed (const deferrable::Deferred<A>* a_deferred, Json::Value& o_payload)
            {
                const auto exception = a_deferred->response().exception();
                if ( nullptr == exception ) {
                    if ( false == a_deferred->response().json().isNull() ) {
                        if ( true == a_deferred->response().json().isObject() ) {
                            o_payload = a_deferred->response().json();
                        } else {
                            o_payload["error"] = a_deferred->response().json();
                        }
                    } else {
                        o_payload["error"] = a_deferred->response().body();
                    }
                } else {
                    o_payload["error"] = exception->what();
                }
            }

            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogStep (const deferrable::Deferred<A>* a_deferred, const std::string& a_message)
            {
                // ... log message ...
                CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_INF, a_deferred->tracking_, CC_JOB_LOG_STEP_STEP,
                                        "{%s} - %s",
                                        abbr_.c_str(), a_message.c_str()
                );
            }

            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogDebug (const deferrable::Deferred<A>* a_deferred, const std::string& a_message)
            {
                // ... log message ...
                CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_DBG, a_deferred->tracking_, CC_JOB_LOG_STEP_STEP,
                                        "{%s} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                        abbr_.c_str(), a_message.c_str()
                );
            }
        
            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogError (const deferrable::Deferred<A>* a_deferred, const std::string& a_message)
            {
                // ... log message ...
                CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_ERR, a_deferred->tracking_, CC_JOB_LOG_STEP_STEP,
                                        "{%s} - %s",
                                        abbr_.c_str(), a_message.c_str()
                );
            }
            
            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogVerbose (const deferrable::Deferred<A>* a_deferred, const std::string& a_message)
            {
                // ... log message ...
                CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_VBS, a_deferred->tracking_, CC_JOB_LOG_STEP_HTTP,
                                        "{%s} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                        abbr_.c_str(), a_message.c_str()
                );
            }
        
            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_deferred Deferred request data.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLog (const deferrable::Deferred<A>* a_deferred, const size_t a_level, const char* const a_step, const std::string& a_message)
            {
                if ( 0 == strcasecmp(a_step, CC_JOB_LOG_STEP_DUMP) || 0 == strcasecmp(a_step, CC_JOB_LOG_STEP_HTTP) ) {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_deferred->tracking_, a_step,
                                            "{%s} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            abbr_.c_str(), a_message.c_str()
                    );
                } else if ( 0 == strcasecmp(a_step, CC_JOB_LOG_STEP_ERROR) ) {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_deferred->tracking_, a_step,
                                            "{%s} - " CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            abbr_.c_str(), a_message.c_str()
                    );
                } else {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_deferred->tracking_, a_step,
                                            "{%s} - %s", abbr_.c_str(), a_message.c_str()
                    );
                }
            }

            // MARK: -

            /**
             * @brief Called by a 'deferred' request when there's a messages that needs to be logged.
             *
             * @param a_tracking Deferred request tracking data.
             * @param a_level    JOB Log level.
             * @param a_step     JOB step.
             * @param a_message  Message to log
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::OnDeferredRequestLogTracking (const Tracking& a_tracking, const size_t a_level, const char* const a_step, const std::string& a_message)
            {
                if ( CC_JOB_LOG_LEVEL_DBG == a_level && 0 == strcasecmp(a_step, CC_JOB_LOG_STEP_DUMP) ) {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, CC_JOB_LOG_STEP_DUMP,
                                            "{%s} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_tracking.dpid_.c_str(), a_message.c_str()
                    );
                } else {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, a_step,
                                            "{%s} - %s", a_tracking.dpid_.c_str(), a_message.c_str()
                    );
                }
            }
        
            /**
             * @brief Call this method to publish a progress message.
             *
             * @param a_step \link Signer::Step \link
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::Publish (const uint64_t& a_id, const std::string& a_rcid, const std::string& a_rjid,
                                                                          const S& a_step, const cc::easy::job::Job::Status& a_status,
                                                                          const ::cc::easy::job::I18N a_i18n)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                ev::loop::beanstalkd::Job::Publish(
                a_id, a_rcid, a_rjid,
                {
                    /* key_    */ a_i18n.key_.c_str(),
                    /* args_   */ a_i18n.arguments_,
                    /* status_ */ a_status,
                    /* value_  */ static_cast<double>(a_step),
                    /* now_    */ true
                });
            }
            
            /**
             * @brief Call this method to publish a progress message.
             *
             * @param a_step \link Signer::Step \link
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::Publish (const uint64_t& a_id, const std::string& a_rcid, const std::string& a_rjid,
                                                                          const float& a_percentage, const cc::easy::job::Job::Status& a_status,
                                                                          const ::cc::easy::job::I18N a_i18n)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                ev::loop::beanstalkd::Job::Publish(
                a_id, a_rcid, a_rjid,
                {
                    /* key_    */ a_i18n.key_.c_str(),
                    /* args_   */ a_i18n.arguments_,
                    /* status_ */ a_status,
                    /* value_  */ static_cast<double>(a_percentage),
                    /* now_    */ true
                });
            }

            /**
             * @brief Helper function to be called when a deferred request returned.
             *
             * @param a_deferred Deferred request data.
             * @param a_callback Function to call:
             *                  - if returns 0 don't finalize job now ( still work to do );
             *                  - if 0, or if an exception is catched finalize job immediatley.
             * @param o_payload JSON response to fill.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::HandleDeferredRequestCompletion (const deferrable::Deferred<A>* a_deferred, std::function<uint16_t(Json::Value& o_payload)> a_callback, const Tracking& a_tracking)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                
                Json::Value payload  = Json::Value(Json::ValueType::objectValue);
                Json::Value response = Json::Value::null;
                
                uint16_t code = CC_STATUS_CODE_INTERNAL_SERVER_ERROR;
                
                try {
                    // ... perform callback ...
                    code = a_callback(payload);
                    // ... success?
                    if ( CC_STATUS_CODE_OK == code ) {
                        // ... set 'completed' response ...
                        code = DeferrableBaseClassAlias::SetCompletedResponse(payload, response);
                    } else if ( 0 != code ) {
                        // ... set 'failed' response ...
                        code = DeferrableBaseClassAlias::SetFailedResponse(code, payload, response);
                    }                
                } catch (const cc::Exception& a_cc_exception) {
                    // ... set internal server error ....
                    code = DeferrableBaseClassAlias::SetFailedResponse(
                            DeferrableBaseClassAlias::SetInternalServerError(&DeferrableBaseClassAlias::I18NError(), /* a_exception */ { /* code_ */ nullptr, /* exception_ */ a_cc_exception  }, payload),
                            payload, response
                    );
                } catch (...) {
                    try {
                        ::cc::Exception::Rethrow(/* a_unhandled */ true, __FILE__, __LINE__, __FUNCTION__);
                    } catch (::cc::Exception& a_cc_exception) {
                        // ... set internal server error ....
                        code = DeferrableBaseClassAlias::SetFailedResponse(
                                DeferrableBaseClassAlias::SetInternalServerError(&DeferrableBaseClassAlias::I18NError(), /* a_exception */ { /* code_ */ nullptr, /* exception_ */ a_cc_exception  },payload),
                                payload, response
                        );
                    }
                }
                
                // ... still work to do?
                if ( 0 == code ) {
                    // ... yes, we're done here ...
                    return;
                }
                        
                // ... insanity checkpoint ...
                CC_ASSERT(false == response.isNull());

                // ... publish progress ( 100% ) ...
                Publish(a_tracking.bjid_, a_tracking.rcid_, a_tracking.rjid_, doneValue, DeferrableBaseClassAlias::Status::InProgress,
                        DeferrableBaseClassAlias::I18NCompleted()
                );

                //
                // ... log final response ...
                //
                DeferrableBaseClassAlias::LogResponse({ code, Json::Value::null }, response);

                // ... publish result ...
                DeferrableBaseClassAlias::Finished(/* a_id               */ a_tracking.bjid_,
                                                   /* a_channel          */ a_tracking.rcid_,
                                                   /* a_key              */ a_tracking.rjid_,
                                                   /* a_response         */ response,
                                                   /* a_success_callback */ nullptr,
                                                   /* a_failure_callback */
                                                   [this](const ev::Exception& a_ev_exception) {
                                                       // ... log error ...
                                                       CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_ERR, CC_JOB_LOG_STEP_ERROR,
                                                                      CC_JOB_LOG_COLOR(LIGHT_RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS " - %s: %s",
                                                                      "FAILED", "while publishing finished notification", a_ev_exception.what()
                                                        );
                                                   },
                                                   /* a_mode */ ( true == a_deferred->arguments().Primitive() ? DeferrableBaseClassAlias::Mode::Gateway : DeferrableBaseClassAlias::Mode::Default )
                );
            }

            /**
             * @brief Helper function to be called when a deferred request returned and response must be logged.
             *
             * @param a_dpid     Dispatcher ID.
             * @param a_tracking Request tracking info.
             * @param a_response Deferred request response data.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::LogDeferredRequestResponse (const std::string& a_dpid, const deferrable::Tracking& a_tracking, const deferrable::Response& a_response)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                //
                // ... log response ...
                //
                // ... success?
                if ( CC_STATUS_CODE_OK == a_response.code() ) {
                    // ... log response status code  ...
                    CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_INF, a_tracking, CC_JOB_LOG_STEP_STATUS,
                                            "{%s} - " UINT16_FMT ", %s",
                                            a_dpid.c_str(), a_response.code(), "OK"
                    );
                    // ... log success response  ...
                    CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_DBG, a_tracking, CC_JOB_LOG_STEP_DUMP,
                                            "{%s} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_dpid.c_str(), a_response.body().c_str()
                    );
                } else {
                    // ... log response status code  ...
                    CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_INF, a_tracking, CC_JOB_LOG_STEP_STATUS,
                                            "{%s} - " CC_JOB_LOG_COLOR(RED) UINT16_FMT ", %s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_dpid.c_str(), a_response.code(), a_response.status().c_str()
                    );
                    // ... log error response  ...
                    CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_DBG, a_tracking, CC_JOB_LOG_STEP_DUMP,
                                            "{%s} - " CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_dpid.c_str(), a_response.body().c_str()
                    );
                }
                // ... log response RTT ...
                CASPER_JOB_LOG_DEFERRED(CC_JOB_LOG_LEVEL_INF, a_tracking, CC_JOB_LOG_STEP_RTT,
                                        "{%s} - took " SIZET_FMT "ms",
                                        a_dpid.c_str(), a_response.rtt()
                );
            }
        
        
            /**
             * @brief Helper function to be called when a deferred request failed and response payload must be set.
             *
             * @param a_dpid      Dispatcher ID.
             * @param a_response  Deferred request response data.
             * @param a_exception Exception to throw ( if any and instead of filling payload ).
             * @param o_payload   Response payload to fill.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::SetDeferredRequestFailed (const std::string& /* a_dpid */, const deferrable::Response& a_response, const ::cc::Exception* a_exception, Json::Value& o_payload)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                // ... an exception occurred ...
                if ( nullptr != a_exception ) {
                    throw cc::Exception(*a_exception);
                }
                // ... set failed response ...
                if ( false == a_response.json().isNull() ) {
                    if ( true == a_response.json().isObject() ) {
                        o_payload = a_response.json();
                    } else {
                        o_payload["error"] = a_response.json();
                    }
                } else {
                    o_payload["error"] = a_response.body();
                }
            }

            /**
             * @brief  Helper function to be called when a deferred message must be logged.
             *
             * @param a_dpid     Dispatcher ID.
             * @param a_level    Log level.
             * @param a_tracking Request tracking info.
             * @param a_message  Message to log.
             */
            template <class A, typename S, S doneValue>
            void casper::job::deferrable::Base<A, S, doneValue>::LogDeferredRequestMessage (const std::string& a_dpid, const size_t a_level, const deferrable::Tracking& a_tracking, const std::string& a_message)
            {
                CC_DEBUG_FAIL_IF_NOT_AT_THREAD(DeferrableBaseClassAlias::thread_id_);
                // ... log message ...
                if ( CC_JOB_LOG_LEVEL_ERR == a_level ) {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, CC_JOB_LOG_STEP_ERROR,
                                            "{" CC_JOB_LOG_COLOR(LIGHT_RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS  "} - " CC_JOB_LOG_COLOR(LIGHT_RED) "%s"
                                            CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_dpid.c_str(), a_message.c_str()
                   );
                } else if ( CC_JOB_LOG_LEVEL_DBG == a_level ) {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, CC_JOB_LOG_STEP_DUMP,
                                            "{" CC_JOB_LOG_COLOR(YELLOW) "%s" CC_LOGS_LOGGER_RESET_ATTRS  "} - " CC_JOB_LOG_COLOR(DARK_GRAY) "%s"
                                            CC_LOGS_LOGGER_RESET_ATTRS,
                                            a_dpid.c_str(), a_message.c_str()
                   );
                } else {
                    CASPER_JOB_LOG_DEFERRED(a_level, a_tracking, CC_JOB_LOG_STEP_STEP,
                                            "{%s} - %s",
                                            a_dpid.c_str(), a_message.c_str()
                   );
                }
            }
                
        } // end of namespace 'deferrable'
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_BASE_H_
