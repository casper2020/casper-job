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
* casper-job is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with casper-job. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#ifndef CASPER_JOB_BASE_H_
#define CASPER_JOB_BASE_H_

#include "casper/job/basic.h"

namespace casper
{

    namespace job
    {
    
        template <typename S, S doneValue>
        class Base : public ::casper::job::Basic<S>
        {

        public: // Constructor(s) / Destructor
            
            Base () = delete;
            Base (const std::string& a_tube,
                 const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config);
            virtual ~Base ();
            
        public: // Inherited Virtual Method(s) / Function(s) - from cc::easy::job::Runnable

            virtual void Setup ();
            virtual void Run   (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response);
            
        protected: // Virtual Method(s) / Function(s)
            
            virtual void InnerSetup () {}
            virtual void InnerRun   (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response) = 0;
            
        protected: // Method(s) / Function(s)
            
            void Log (const size_t a_level, const char* const a_step, const std::string& a_message);
        
        }; // end of class 'Job'
            
        /**
         * @brief Default constructor.
         *
         * param a_tube
         * param a_loggable_data
         * param a_config
         */
        template <typename S, S doneValue>
        ::casper::job::Base<S, doneValue>::Base (const std::string& a_tube,
                                                 const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config)
            : ::casper::job::Basic<S>::Basic(a_tube, a_loggable_data, a_config)
        {
            /* empty */
        }

        /**
         * @brief Destructor
         */
        template <typename S, S doneValue>
        ::casper::job::Base<S, doneValue>::~Base ()
        {
            /* empty */
        }
    
        /**
         * @brief One-shot initialization.
         */
        template <typename S, S doneValue>
        void ::casper::job::Base<S, doneValue>::Setup ()
        {
            ::casper::job::Basic<S>::Setup();
            InnerSetup();
        }
    
        /**
         * @brief Process a job to this tube.
         *
         * @param a_id      Job ID.
         * @param a_payload Job payload.
         *
         * @param o_response JSON object.
         */
        template <typename S, S doneValue>
        void ::casper::job::Base<S, doneValue>::Run (const uint64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response)
        {
            // ... sanity check ...
            CC_DEBUG_FAIL_IF_NOT_AT_THREAD(::casper::job::Basic<S>::thread_id_);

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

            // ... assuming BAD REQUEST ...
            o_response.code_ = CC_STATUS_CODE_BAD_REQUEST;
            
            // ... run ...
            try {

                InnerRun(a_id, a_payload, o_response);

            } catch (const ::cc::BadRequest& a_br_exception) {
                // ... parsing error ...
                o_response.code_ = ::casper::job::Basic<S>::SetBadRequest(/* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                             /* a_error */ {
                                                                                /* code_ */ nullptr,
                                                                                /* why_  */ std::string(a_br_exception.what())
                                                                             },
                                                                             o_response.payload_
                );
            } catch (const ::cc::NotImplemented& a_ni_exception) {
                // ... ISE ...
                o_response.code_ = ::casper::job::Basic<S>::SetNotImplemented(/* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                              /* a_error */ {
                                                                                  /* code_ */ nullptr,
                                                                                  /* why_  */ std::string(a_ni_exception.what())
                                                                               },
                                                                               o_response.payload_
                );
            } catch (const ::cc::InternalServerError& a_ise_exception) {
                // ... ISE ...
                o_response.code_ = ::casper::job::Basic<S>::SetInternalServerError(/* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                                   /* a_error */ {
                                                                                     /* code_ */ nullptr,
                                                                                     /* why_  */ std::string(a_ise_exception.what())
                                                                                   },
                                                                                   o_response.payload_
                );
            } catch (const ::cc::CodedException& a_ce_exception) {
                // ... ISE ...
                o_response.code_ = ::casper::job::Basic<S>::SetError(a_ce_exception.code_,
                                                                    /* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                    /* a_error */ {
                                                                        /* code_ */ nullptr,
                                                                        /* why_  */ std::string(a_ce_exception.what())
                                                                    },
                                                                    o_response.payload_
                );
            } catch (const ::cc::Exception& a_cc_exception) {
                // ... parsing error ...
                o_response.code_ = ::casper::job::Basic<S>::SetInternalServerError(/* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                                          /* a_error */ {
                                                                                                /* code_ */ nullptr,
                                                                                                /* why_  */ ( "An error occurred while preparing job: " + std::string(a_cc_exception.what()))
                                                                                          },
                                                                                          o_response.payload_
                );
            } catch (...) {
                try {
                    ::cc::Exception::Rethrow(/* a_unhandled */ true, __FILE__, __LINE__, __FUNCTION__);
                } catch (::cc::Exception& a_cc_exception) {
                    // ... parsing error ...
                    o_response.code_ = ::casper::job::Basic<S>::SetInternalServerError(/* a_i18n */ &::casper::job::Base<S, doneValue>::I18NError(),
                                                                                              /* a_error */ {
                                                                                                    /* code_ */ nullptr,
                                                                                                    /* why_  */ ( "An error occurred while preparing job: " + std::string(a_cc_exception.what()))
                                                                                              },
                                                                                              o_response.payload_
                    );
                }
            }
        }
        
        /**
         * @brief Log a message.
         *
         * @param a_level   Log level.
         * @param a_step    Job step.
         * @param a_message Message to log.
         */
        template <typename S, S doneValue>
        void ::casper::job::Base<S, doneValue>::Log (const size_t a_level, const char* const a_step, const std::string& a_message)
        {
            if ( a_level == CC_JOB_LOG_LEVEL_ERR ) {
                CASPER_JOB_LOG(a_level, a_step, CC_JOB_LOG_COLOR(RED) "%s" CC_LOGS_LOGGER_RESET_ATTRS, a_message.c_str());
            } else if ( a_level == CC_JOB_LOG_LEVEL_DBG ) {
                CASPER_JOB_LOG(a_level, a_step, CC_JOB_LOG_COLOR(DARK_GRAY) "%s" CC_LOGS_LOGGER_RESET_ATTRS, a_message.c_str());
            } else {
                CASPER_JOB_LOG(a_level, a_step, "%s", a_message.c_str());
            }
        }
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_BASE_H_
