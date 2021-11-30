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
#ifndef CASPER_JOB_DEMO_BASIC_H_
#define CASPER_JOB_DEMO_BASIC_H_

#include "casper/job/basic.h"

namespace casper
{

    namespace job
    {
    
        namespace demo
        {
        
            enum class BasicDemoStep : uint8_t {
                Fetching = 5,
                DoingIt  = 95,
                Done     = 100
            };
                    
            class Basic final : public ::casper::job::Basic<BasicDemoStep>
            {
                                
            public: // Static Const Data
                
                constexpr static const char* const sk_tube_ = "casper-job-demo-basic";
                
            public: // Constructor(s) / Destructor
                
                Basic () = delete;

                /**
                 * @brief Default constructor.
                 *
                 * param a_loggable_data
                 * param a_config
                 */
                Basic (const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config)
                    : ::casper::job::Basic<BasicDemoStep>(sk_tube_, a_loggable_data, a_config)
                {
                    /* empty */
                }

                /**
                 * @brief Destructor
                 */
                ~Basic ()
                {
                    /* empty */
                }

            protected: // Virtual Method(s) / Function(s)
                
                /**
                 * @brief Process a job to this tube.
                 *
                 * @param a_id      Job ID.
                 * @param a_payload Job payload.
                 *
                 * @param o_response JSON object.
                 */
                virtual void Run (const int64_t& a_id, const Json::Value& a_payload, cc::easy::job::Job::Response& o_response)
                {
                    // ... assuming BAD REQUEST ...
                    o_response.code_ = CC_STATUS_CODE_BAD_REQUEST;

                    //
                    // IN payload:
                    //
                    // {
                    //    "id": <numeric>,
                    //    "tube": <string>,
                    //    "ttr": <numeric>,
                    //    "validity": <validity>,
                    // }

                    //
                    // Payload
                    //
                    const Json::Value& payload = Payload(a_payload);


                    o_response.code_    = CC_STATUS_CODE_OK;
                    o_response.payload_ = Json::Value(Json::ValueType::objectValue);
                    o_response.payload_["__id__"]      = static_cast<Json::UInt64>(a_id);
                    o_response.payload_["__payload__"] = payload;
                }

            }; // end of class 'Basic'
        
        } // end of namespace 'demo'
    
    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEMO_BASIC_H_
