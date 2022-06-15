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
            
        private: // Static Const Data
            
            static const ::cc::easy::job::I18N sk_i18n_in_progress_ ;
            static const ::cc::easy::job::I18N sk_i18n_completed_;
            static const ::cc::easy::job::I18N sk_i18n_error_;
            
        protected: // Const Data

#define __CASPER_JOB(a_level, a_id, a_format, ...) \
if ( a_level <= casper::job::Basic<S>::log_level_ ) \
    ::ev::LoggerV2::GetInstance().Log(casper::job::Basic<S>::logger_client_, casper::job::Basic<S>::tube_.c_str(), "Job #" INT64_FMT ", " a_format, a_id, __VA_ARGS__)

#define CASPER_JOB_LOG(a_level, a_step, a_format, ...) \
__CASPER_JOB(a_level, casper::job::Basic<S>::ID(), \
               CC_JOB_LOG_COLOR(MAGENTA) "%-8.8s" CC_LOGS_LOGGER_RESET_ATTRS ": %-7.7s, " a_format, \
              "JOB", a_step, __VA_ARGS__ \
);

        private: // Data
            
            ::cc::easy::job::I18N* i18n_in_progress_;
            ::cc::easy::job::I18N* i18n_completed_;
            ::cc::easy::job::I18N* i18n_error_;

        public: // Constructor(s) / Destructor
            
            Basic () = delete;
            Basic (const std::string& a_tube,
                 const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config);
            virtual ~Basic ();
            
        public: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Runnable
            
            virtual void Setup ();
        
        protected: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Job
            
            virtual void LogResponse             (const cc::easy::job::Job::Response& a_response, const Json::Value& a_payload);
            
        protected: // Virtual Method(s) / Function(s) 
            
            virtual void LogMessage              (const int a_level, const char* const a_step, const std::string& a_message) const;
            virtual void LogResponseInterception (const std::string& a_message);
            virtual void LogResponseOverride     (const uint16_t a_code, const std::string& a_content_type, const std::string& a_body, bool a_original);

        protected: // Inline Method(s) / Function(s)

            const Json::Value& Payload        (const Json::Value& a_payload, bool* o_broker = nullptr, bool* o_with_job_role = nullptr);
            const bool         SourceIsBroker (const Json::Value& a_payload, bool* o_with_job_role);
                            
        protected: // Method(s) / Function(s)
            
            void Publish (const S& a_step, const Status& a_status,
                          const char* const a_i18n_key,
                          const std::map<std::string, Json::Value>& a_arguments);
            
            void Publish (const double a_progress, const Status& a_status,
                          const char* const a_i18n_key,
                          const std::map<std::string, Json::Value>& a_arguments);
            
        protected: // Method(s) / Function(s)
            
            void                         OverrideI18N   (const Json::Value& a_value);
            const ::cc::easy::job::I18N& I18NInProgress () const;
            const ::cc::easy::job::I18N& I18NCompleted  () const;
            const ::cc::easy::job::I18N& I18NError      () const;
            
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
            : cc::easy::job::Job(a_loggable_data, a_tube, a_config),
             i18n_in_progress_(nullptr), i18n_completed_(nullptr), i18n_error_(nullptr)
        {
            /* empty */
        }

        /**
         * @brief Destructor
         */
        template <typename S>
        casper::job::Basic<S>::~Basic ()
        {
            if ( nullptr != i18n_in_progress_ ) {
                delete i18n_in_progress_;
            }
            if ( nullptr != i18n_completed_ ) {
                delete i18n_completed_;
            }
            if ( nullptr != i18n_error_ ) {
                delete i18n_error_;
            }
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
         * @param a_payload       Payload to inspect.
         * @param o_broker        If not null, check if 'source' was nginx-broker.
         * @param o_with_job_role If not null, check if 'source' is nginx-broker and it has job as 'role'.
         *
         * @return Payload object reference.
         */
        template <typename S>
        inline const Json::Value& casper::job::Basic<S>::Payload (const Json::Value& a_payload, bool* o_broker, bool* o_with_job_role)
        {
            const ::cc::easy::JSON<::cc::Exception> json;
            const Json::Value c_ttr      = Json::Value(static_cast<Json::Int64>(TTR()));
            const Json::Value c_validity = Json::Value(static_cast<Json::Int64>(Validity()));
            // ... NGINX-XXX module awareness ...
            if ( true == a_payload.isMember("body") && true == a_payload.isMember("headers") ) {
                // ... from nginx-broker?
                if ( nullptr != o_broker ) {
                    (*o_broker) = SourceIsBroker(a_payload, o_with_job_role);
                }
                // ... read TTR and validity ...
                const int64_t ttr      = static_cast<int64_t>(json.Get(a_payload["body"], "ttr", Json::ValueType::uintValue, &c_ttr).asUInt64());
                const int64_t validity = static_cast<int64_t>(json.Get(a_payload["body"], "validity", Json::ValueType::uintValue, &c_validity).asUInt64());
                // ... read TTR and validity ...
                SetTTRAndValidity(ttr, validity);
                // ... from nginx-broker 'jobify' module ...
                return a_payload["body"];
            } else {
                // ... reset ...
                if ( nullptr != o_broker ) {
                    (*o_broker) = false;
                }
                if ( nullptr != o_with_job_role ) {
                    (*o_with_job_role) = false;
                }
                // ... read TTR and validity ...
                const int64_t ttr      = static_cast<int64_t>(json.Get(a_payload, "ttr", Json::ValueType::uintValue, &c_ttr).asUInt64());
                const int64_t validity = static_cast<int64_t>(json.Get(a_payload, "validity", Json::ValueType::uintValue, &c_validity).asUInt64());
                // ... read TTR and validity ...
                SetTTRAndValidity(ttr, validity);
                // ... direct from beanstalkd queue ...
                return a_payload;
            }
        }
    
    
        /**
         * @brief Check if this job was injected by nginx-broker.
         *
         * @param a_payload       Payload to inspect.
         * @param o_with_job_role If not null, check if 'source' is nginx-broker and it has job as 'role'.
         *
         * @return True if source is from broker, false otherwise.
         */
        template <typename S>
        inline const bool casper::job::Basic<S>::SourceIsBroker (const Json::Value& a_payload, bool* o_with_job_role)
        {
            // ... reset ...
            if ( nullptr != o_with_job_role ) {
                (*o_with_job_role) = false;
            }
            // ... from nginx-broker?
            if ( not ( true == a_payload.isMember("body") && true == a_payload.isMember("headers") && ( true == a_payload.isMember("__nginx_broker__") ) ) ) {
                // ... no ...
                return false;
            }
            // ... yes, from nginx-broker ...
            if ( nullptr == o_with_job_role ) {
                // ... done ...
                return true;
            }
            // ... test role mask ...
            const auto& headers = a_payload["headers"];
            std::smatch match;
            char* end_ptr = nullptr;
            for ( Json::ArrayIndex idx = 0 ; idx < headers.size(); ++idx ) {
                const std::string value = headers[idx].asString();
                const std::regex hex_expr("X-CASPER-ROLE-MASK:\\s+(0[xX][0-9a-fA-F]+)");
                if ( true == std::regex_match(value, match, hex_expr) && 2 == match.size() ) {
                    const std::string v = match[1].str();
                    (*o_with_job_role) = ( 0 != ( std::strtoull(v.c_str(), &end_ptr, 16) & 0x40000000 ) );
                    break;
                } else {
                    const std::regex dec_expr("X-CASPER-ROLE-MASK:\\s+(\\d+)");
                    if ( true == std::regex_match(value, match, dec_expr) && 2 == match.size() ) {
                        const std::string v = match[1].str();
                        (*o_with_job_role) = ( 0 != ( std::strtoull(v.c_str(), &end_ptr, 10) & 0x40000000 ) );
                        break;
                    }
                }
            }
            // ... done ...
            return true;
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
         * @brief Load this job message.
         *
         * @param a_level
         * @param a_step
         * @param a_message
         */
        template <typename S>
        void casper::job::Basic<S>::LogMessage (const int a_level, const char* const a_step, const std::string& a_message) const
        {
            CASPER_JOB_LOG(a_level, a_step, "%s", a_message.c_str());
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
                if ( true == config_.log_redact() ) {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                   "Response: " CC_JOB_LOG_COLOR(GREEN) SIZET_FMT " byte(s)" CC_LOGS_LOGGER_RESET_ATTRS,
                                   jfw.write(a_payload).length()
                    );
                } else {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                   "Response: " CC_JOB_LOG_COLOR(GREEN) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   jfw.write(a_payload).c_str()
                    );
                }
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
                if ( true == config_.log_redact() ) {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                   "Response: " CC_JOB_LOG_COLOR(RED) SIZET_FMT " byte(s)" CC_LOGS_LOGGER_RESET_ATTRS,
                                   jfw.write(a_payload).length()
                    );
                } else {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_OUT,
                                   "Response: " CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   jfw.write(a_payload).c_str()
                    );
                }
                // ... status ...
                CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_INF, CC_JOB_LOG_STEP_STATUS,
                               CC_JOB_LOG_COLOR(LIGHT_RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                               "Failed"
                );
            }
        }
    
        /**
         * @brief Load this job response.
         *
         * @param a_message Message to log.
         */
        template <typename S>
        void casper::job::Basic<S>::LogResponseInterception (const std::string& a_message)
        {
            // ... log ...
            CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_WRN, CC_JOB_LOG_STEP_INFO, CC_JOB_LOG_COLOR(YELLOW) "%s" CC_LOGS_LOGGER_RESET_ATTRS, a_message.c_str());
        }
    
        /**
         * @brief Loag this job response override notice(s).
         *
         * @param a_code         HTTP status code.
         * @param a_content_type HTTP Content-Type header value.
         * @param a_body         Body.
         * @param a_original     True if it's the original response, false otherwise.
         */
        template <typename S>
        void casper::job::Basic<S>::LogResponseOverride (const uint16_t a_code, const std::string& a_content_type, const std::string& a_body, bool a_original)
        {
            const auto it                 = cc::i18n::Singleton::k_http_status_codes_map_.find(a_code);
            const std::string status_name = ( cc::i18n::Singleton::k_http_status_codes_map_.end() != it ? it->second : "???" );
            const char* const what        = ( a_original ? "Original" : "Overriden" );
            const char* const color       = ( a_original ? CC_JOB_LOG_COLOR(CYAN) : CC_JOB_LOG_COLOR(YELLOW) );
            // ... status ...
            {
                if ( CC_STATUS_CODE_OK == a_code ) {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_WRN, CC_JOB_LOG_STEP_INFO,
                                   "%s%s Status: " CC_JOB_LOG_COLOR(GREEN) UINT16_FMT " - %s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   color, what, a_code, status_name.c_str()
                    );
                } else {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_WRN, CC_JOB_LOG_STEP_INFO,
                                   "%s%s Status: " CC_JOB_LOG_COLOR(RED) UINT16_FMT " - %s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   color, what, a_code, status_name.c_str()
                    );
                }
            }
            // ... response ...
            {
                if ( true == config_.log_redact() ) {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_WRN, CC_JOB_LOG_STEP_INFO,
                                   "%s%s Body: " CC_JOB_LOG_COLOR(DARK_GRAY) SIZET_FMT " byte(s)" CC_LOGS_LOGGER_RESET_ATTRS,
                                   color, what, a_body.length()
                    );
                } else {
                    CASPER_JOB_LOG(CC_JOB_LOG_LEVEL_WRN, CC_JOB_LOG_STEP_INFO,
                                   "%s%s Body: " CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS,
                                   color, what, a_body.c_str()
                    );
                }
            }
        }
    
        /**
         * @brief Override i18n messages.
         *
         * @param a_value i18n messages, JSON representation.
         */
        template <typename S>
        void casper::job::Basic<S>::OverrideI18N (const Json::Value& a_value)
        {
            const std::map<const char* const, ::cc::easy::job::I18N**> supported = {
                { "progress" , &i18n_in_progress_ },
                { "completed", &i18n_completed_   },
                { "error"    , &i18n_error_       }
            };
            const ::cc::easy::JSON<::cc::Exception> json;
            for ( const auto& entry : supported ) {
                const Json::Value& value = json.Get(a_value, entry.first, Json::ValueType::stringValue, &Json::Value::null);
                if ( true == value.isNull() ) {
                    continue;
                }
                if ( nullptr != *entry.second ) {
                    delete *entry.second;
                }
                (*entry.second) = new ::cc::easy::job::I18N({
                    /* key_       */ value.asString(),
                    /* arguments_ */ {}
                });
            }
        }

        /**
         * @return i18 'progress' message key and it's args.
         */
        template <typename S>
        const ::cc::easy::job::I18N& casper::job::Basic<S>::I18NInProgress () const
        {
            return ( nullptr !=  i18n_in_progress_ ? *i18n_in_progress_ : sk_i18n_in_progress_);
        }

        /**
         * @return i18 'completed' message key and it's args.
         */
        template <typename S>
        const ::cc::easy::job::I18N& casper::job::Basic<S>::I18NCompleted () const
        {
            return ( nullptr !=  i18n_completed_ ? *i18n_completed_ : sk_i18n_completed_);
        }

        /**
         * @return i18 'error' message key and it's args.
         */
        template <typename S>
        const ::cc::easy::job::I18N& casper::job::Basic<S>::I18NError () const
        {
            return ( nullptr !=  i18n_error_ ? *i18n_error_ : sk_i18n_error_);
        }
    
    } // end of namespace 'fs'

} // end of namespace 'casper'

#endif // CASPER_JOB_BASIC_H_
