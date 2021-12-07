/**
 * @file basic.h
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
 * along with casper-job.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#ifndef CASPER_JOB_BASIC_H_
#define CASPER_JOB_BASIC_H_

#include "cc/easy/job/job.h"

#if defined(__APPLE__) && defined(CC_DEBUG_ON)
    #include "sys/bsd/process.h"
#endif

#include "cc/codes.h"

#include "cc/exception.h"
#include "cc/easy/json.h"
#include "cc/i18n/singleton.h"

namespace casper
{

    namespace job
    {
    
        template <typename S>
        class Basic : public cc::easy::job::Job
        {
            
        protected: // Static Const Data
            
            static const ::cc::easy::job::I18N sk_i18n_in_progress_ ;
            static const ::cc::easy::job::I18N sk_i18n_completed_;
            static const ::cc::easy::job::I18N sk_i18n_error_;

        protected: // Const Data
            
            CC_IF_DEBUG_DECLARE_VAR(const cc::debug::Threading::ThreadID, thread_id_);

#define __CASPER_JOB(a_level, a_id, a_format, ...) \
if ( a_level <= casper::job::Basic<S>::log_level_ ) \
    ::ev::LoggerV2::GetInstance().Log(casper::job::Basic<S>::logger_client_, casper::job::Basic<S>::tube_.c_str(), "Job #" INT64_FMT ", " a_format, a_id, __VA_ARGS__)

#define CASPER_JOB_LOG(a_level, a_step, a_format, ...) \
__CASPER_JOB(a_level, casper::job::Basic<S>::ID(), \
               CC_JOB_LOG_COLOR(MAGENTA) "%-8.8s" CC_LOGS_LOGGER_RESET_ATTRS ": %-7.7s, " a_format, \
              "JOB", a_step, __VA_ARGS__ \
);

        public: // Constructor(s) / Destructor
            
            Basic () = delete;
            Basic (const std::string& a_tube,
                 const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config);
            virtual ~Basic ();
            
        public: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Runnable
            
            virtual void Setup ();
        
        protected: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Job
            
            virtual void LogResponse (const cc::easy::job::Job::Response& a_response, const Json::Value& a_payload);

        protected: // Inline Method(s) / Function(s)

            const Json::Value& Payload (const Json::Value& a_payload, bool* o_broker = nullptr);
                            
        protected: // Method(s) / Function(s)
            
            void Publish (const S& a_step, const Status& a_status,
                          const char* const a_i18n_key,
                          const std::map<std::string, Json::Value>& a_arguments);
            
            void Publish (const double a_progress, const Status& a_status,
                          const char* const a_i18n_key,
                          const std::map<std::string, Json::Value>& a_arguments);
            
        }; // end of class 'Basic'
    
        template <typename S>
        const ::cc::easy::job::I18N casper::job::Basic<S>::sk_i18n_in_progress_ = { /* key_ */ "i18n_in_progress" , /* args_ */ {} };
        template <typename S>
        const ::cc::easy::job::I18N casper::job::Basic<S>::sk_i18n_completed_ = { /* key_ */ "i18n_completed", /* args_ */ {} };
        template <typename S>
        const ::cc::easy::job::I18N casper::job::Basic<S>::sk_i18n_error_     = { /* key_ */ "i18n_error"    , /* args_ */ {} };
        
        /**
         * @brief Default constructor.
         *
         * param a_tube
         * param a_loggable_data
         * param a_config
         */
        template <typename S>
        casper::job::Basic<S>::Basic (const std::string& a_tube,
                                          const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config)
            : cc::easy::job::Job(a_loggable_data, a_tube, a_config)
             CC_IF_DEBUG_CONSTRUCT_APPEND_SET_VAR(thread_id_, cc::debug::Threading::GetInstance().CurrentThreadID(),)
        {
            /* empty */
        }

        /**
         * @brief Destructor
         */
        template <typename S>
        casper::job::Basic<S>::~Basic ()
        {
            /* empty */
        }

        /**
         * @brief One-shot initialization.
         */
        template <typename S>
        void casper::job::Basic<S>::Setup ()
        {
            CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
            const Json::Value& directories = GetJSONObject(config_.other(), "directories", Json::ValueType::objectValue, &Json::Value::null);
            if ( false == directories.isNull() ) {
                const Json::Value& tmp = GetJSONObject(directories, "tmp", Json::ValueType::stringValue, &Json::Value::null);
                if ( false == tmp.isNull() ) {
                    SetOutputDirectoryPrefix(OSAL_NORMALIZE_PATH(tmp.asString()));
                }
            }
        }
    
        /**
         * @brief Extract 'true' payload from provided data.
         *
         * @param a_payload  Payload to inspect.
         * @param o_broker   If not null, check if 'source' was nginx-broker.
         *
         * @return Payload object reference.
         */
        template <typename S>
        inline const Json::Value& casper::job::Basic<S>::Payload (const Json::Value& a_payload, bool* o_broker)
        {
            const ::cc::easy::JSON<::cc::Exception> json;
            const Json::Value c_ttr      = Json::Value(static_cast<Json::Int64>(TTR()));
            const Json::Value c_validity = Json::Value(static_cast<Json::Int64>(Validity()));
            // ... NGINX-BROKER 'jobify' module awareness ...
            if ( true == a_payload.isMember("body") && true == a_payload.isMember("headers") ) {
                // ... check source?
                if ( nullptr != o_broker ) { // TODO: review this code
                    (*o_broker) = ( true == a_payload.isMember("__nginx_broker__") );
                }
                // ... read TTR and validity ...
                const int64_t ttr      = static_cast<int64_t>(json.Get(a_payload["body"], "ttr", Json::ValueType::uintValue, &c_ttr).asUInt64());
                const int64_t validity = static_cast<int64_t>(json.Get(a_payload["body"], "validity", Json::ValueType::uintValue, &c_validity).asUInt64());
                // ... read TTR and validity ...
                SetTTRAndValidity(ttr, validity);
                // ... from nginx-broker 'jobify' module ...
                return a_payload["body"];
            } else {
                // ... read TTR and validity ...
                const int64_t ttr      = static_cast<int64_t>(json.Get(a_payload, "ttr", Json::ValueType::uintValue, &c_ttr).asUInt64());
                const int64_t validity = static_cast<int64_t>(json.Get(a_payload, "validity", Json::ValueType::uintValue, &c_validity).asUInt64());
                // ... read TTR and validity ...
                SetTTRAndValidity(ttr, validity);
                // ... direct from beanstalkd queue ...
                return a_payload;
            }
        }

        // MARK: - PROGRESS REPORT HELPER(S)

        /**
         * @brief Call this method to publish a progress message.
         *
         * @param a_step      Current step value, onle of \link S \link.
         * @param a_status    Current status value, onle of \link Status \link.
         * @param a_i18n_key  I18N key.
         * @param a_arguments I18N arguments map.
         */
        template <typename S>
        void casper::job::Basic<S>::Publish (const S& a_step, const Status& a_status,
                                             const char* const a_i18n_key, const std::map<std::string, Json::Value>& a_arguments)
        {
            CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
            ev::loop::beanstalkd::Job::Publish({
                /* key_    */ a_i18n_key,
                /* args_   */ a_arguments,
                /* status_ */ a_status,
                /* value_  */ static_cast<double>(a_step),
                /* now_    */ true
            });
        }
    
        /**
         * @brief Call this method to publish a progress message.
         *
         * @param a_progress  Percentage value ( 0..100 ).
         * @param a_status    Current status value, onle of \link Status \link.
         * @param a_i18n_key  I18N key.
         * @param a_arguments I18N arguments map.
         */
        template <typename S>
        void casper::job::Basic<S>::Publish (const double a_progress, const Status& a_status,
                                             const char* const a_i18n_key, const std::map<std::string, Json::Value>& a_arguments)
        {
            CC_DEBUG_FAIL_IF_NOT_AT_THREAD(thread_id_);
            ev::loop::beanstalkd::Job::Publish({
                /* key_    */ a_i18n_key,
                /* args_   */ a_arguments,
                /* status_ */ a_status,
                /* value_  */ a_progress,
                /* now_    */ true
            });
        }
            
        /**
         * @brief Load this job response.
         *
         * @param a_response Response object.
         * @param a_payload  Actual JSON response object.
         */
        template <typename S>
        void casper::job::Basic<S>::LogResponse (const cc::easy::job::Job::Response& a_response, const Json::Value& a_payload)
        {
            //
            // ... log final response ...
            //

            const auto it                 = cc::i18n::Singleton::k_http_status_codes_map_.find(a_response.code_);
            const std::string status_name = ( cc::i18n::Singleton::k_http_status_codes_map_.end() != it ? it->second : "???" );

            Json::FastWriter jfw; jfw.omitEndingLineFeed();
            
            if ( CC_STATUS_CODE_OK == a_response.code_ ) {
                // ... status ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                               "Status: " CC_JOB_LOG_COLOR(GREEN) UINT16_FMT " - %s" CC_LOGS_LOGGER_RESET_ATTRS,
                               a_response.code_, status_name.c_str()
                );
                // ... response ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                               "Response: " CC_JOB_LOG_COLOR(GREEN) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                               jfw.write(a_payload).c_str()
                );
                // ... status ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATUS,
                               CC_JOB_LOG_COLOR(LIGHT_GREEN) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                               "Succeeded"
                );
            } else {
                // ... status ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                               "Status: " CC_JOB_LOG_COLOR(RED) UINT16_FMT " - %s" CC_LOGS_LOGGER_RESET_ATTRS,
                               a_response.code_, status_name.c_str()
                );
                // ... response ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                               "Response: " CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                               jfw.write(a_payload).c_str()
                );
                // ... status ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATUS,
                               CC_JOB_LOG_COLOR(LIGHT_RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                               "Failed"
                );
            }
        }
    
    } // end of namespace 'fs'

} // end of namespace 'casper'

#endif // CASPER_JOB_BASIC_H_
