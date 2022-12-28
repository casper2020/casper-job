/**
 * @file types.h
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
#ifndef CASPER_JOB_DEFERRABLE_TYPES_H_
#define CASPER_JOB_DEFERRABLE_TYPES_H_

#include <inttypes.h>
#include <string>

#include "json/json.h"

#include "cc/non-movable.h"
#include "cc/easy/json.h"
#include "cc/i18n/singleton.h"
#include "cc/exception.h"

namespace casper
{

    namespace job
    {

        namespace deferrable
        {
                    
            typedef struct  {
                const uint64_t    bjid_; //!< BEANSTALKD job ID
                const std::string rjnr_; //!< REDIS job number
                const std::string rjid_; //!< REDIS job key
                const std::string rcid_; //!< REDIS channel ID
                const std::string dpid_; //!< Dispatcher ID
                const std::string ua_;   //!< HTTP User-Agent header value
            } Tracking;
                    
            class Response final : ::cc::NonMovable {
                
            private: // Data
                
                uint16_t                           code_;
                std::map<std::string, std::string> headers_;
                std::string                        body_;
                Json::Value                        json_;
                std::string                        content_type_;
                size_t                             rtt_;
                cc::Exception*                     exception_;
                
            public: // Constructor(s) / Destructor
                
                /**
                 * @brief Default constructor.
                 */
                Response ()
                {
                    code_      = 500;
                    rtt_       = 0;
                    exception_ = nullptr;
                }
            
                /**
                 * @brief Copy constructor.
                 *
                 * @param a_response Response to copy.
                 */
                Response (const Response& a_response)
                {
                    code_         = a_response.code_;
                    headers_      = a_response.headers_;
                    body_         = a_response.body_;
                    json_         = a_response.json_;
                    content_type_ = a_response.content_type_;
                    rtt_          = a_response.rtt_;
                    exception_    = ( nullptr != a_response.exception_ ? new ::cc::Exception(*a_response.exception_) : nullptr );;
                }
                        
                /**
                 * Destructor.
                 */
                virtual ~Response ()
                {
                    if ( nullptr != exception_ ) {
                        delete exception_;
                    }
                }
                
            public: // Method(s) / Function(s)

                /**
                 * @brief Assignment operator.
                 *
                 * @param a_response Response to copy.
                 */
                inline void operator = (Response const& a_response)
                {
                    code_         = a_response.code_;
                    headers_      = a_response.headers_;
                    body_         = a_response.body_;
                    json_.clear();
                    json_         = a_response.json_;
                    content_type_ = a_response.content_type_;
                    rtt_          = a_response.rtt_;
                    if ( nullptr != exception_ ) {
                        delete exception_;
                    }
                    exception_ = ( nullptr != a_response.exception_ ? new ::cc::Exception(*a_response.exception_) : nullptr );
                }
                
                /**
                 * @brief Keep track of an HTTP response, also parse body is it's JSON.
                 *
                 * @param a_code         Status code.
                 * @param a_content_type Content-Type header value.
                 * @param a_body         Body.
                 * @param a_rtt          Round time trip in milliseconds.
                 * @param a_parse        When true body will be parsed as JSON.
                 */
                inline void Set (const uint16_t& a_code, const std::string& a_content_type, const std::string& a_body, const size_t& a_rtt, const bool a_parse = true)
                {
                    code_         = a_code;
                    headers_.clear();
                    body_         = a_body;
                    content_type_ = a_content_type;
                    if ( nullptr != exception_ ) {
                        delete exception_;
                        exception_ = nullptr;
                    }
                    rtt_ = a_rtt;
                    if ( true == a_parse ) {
                        Parse();
                    } else {
                        json_.clear(); json_ = Json::Value::null;
                    }
                }
                
                /**
                 * @brief Keep track of an HTTP response, also parse body is it's JSON.
                 *
                 * @param a_code         Status code.
                 * @param a_content_type Content-Type header value.
                 * @param a_headers      HTTP headers.
                 * @param a_body         Body.
                 * @param a_rtt          Round time trip in milliseconds.
                 * @param a_parse        When true body will be parsed as JSON.
                 */
                inline void Set (const uint16_t& a_code, const std::string& a_content_type, const std::map<std::string, std::string>& a_headers, const std::string& a_body, const size_t& a_rtt,
                                 const bool a_parse = false)
                {
                    code_         = a_code;
                    headers_      = a_headers;
                    const auto it = headers_.find("Content-Length");
                    if ( headers_.end() != it ) {
                        if ( it->second.length() > 0 && ' ' == it->second.c_str()[0] ) {
                            headers_["Content-Length"] = ' ' + std::to_string(a_body.length());
                        } else {
                            headers_["Content-Length"] = std::to_string(a_body.length());
                        }
                    }
                    body_         = a_body;
                    content_type_ = a_content_type;
                    if ( nullptr != exception_ ) {
                        delete exception_;
                        exception_ = nullptr;
                    }
                    rtt_ = a_rtt;
                    if ( true == a_parse ) {
                        Parse();
                    } else {
                        json_.clear(); json_ = Json::Value::null;
                    }
                }
                
                
                /**
                 * @brief Keep track of an HTTP response as error.
                 *
                 * @param a_code            Status code.
                 * @param a_content_type    Content-Type header value.
                 * @param a_error           Error code.
                 * @param error_description Error destription as JSON value.
                 * @param a_rtt             Round time trip in milliseconds.
                 */
                inline void Set (const uint16_t& a_code, const std::string& a_content_type, const std::string& a_error, const Json::Value& a_error_description, const size_t& a_rtt)
                {
                    code_ = a_code;
                    headers_.clear();
                    body_.clear();
                    content_type_ = a_content_type;
                    if ( nullptr != exception_ ) {
                        delete exception_;
                        exception_ = nullptr;
                    }
                    rtt_ = a_rtt;
                    json_.clear();
                    json_                      = Json::Value(Json::ValueType::objectValue);
                    json_["error"]             = a_error;
                    json_["error_description"] = a_error_description;
                }
                
                /**
                 * @brief Validate previously set HTTP Content-Type and parse as JSON.
                 */
                inline void Parse ()
                {
                    json_.clear(); json_ = Json::Value::null;
                    if ( 0 == strncasecmp(content_type_.c_str(), "application/json", sizeof(char) * 16) ) {
                        const ::cc::easy::JSON<::cc::Exception> json; json.Parse(body_, json_);
                    } else {
                        throw ::cc::Exception("Content-Type '%s' as JSON not supported!", content_type_.c_str());
                    }
                }
                
                /**
                 * @brief Keep track of an exception.
                 *
                 * @param a_code      HTTP status code.
                 * @param a_exception Exception copy.
                 */
                inline void Set (const uint16_t a_code, const ::cc::Exception& a_exception)
                {
                    code_ = a_code;
                    headers_.clear();
                    if ( nullptr != exception_ ) {
                        delete exception_;
                    }
                    exception_ = new ::cc::Exception(a_exception);
                }
                
                /**
                 * @brief Reset current context.
                 *
                 * @param a_code HTTP status code.
                 */
                inline void Reset (const uint16_t a_code = 500)
                {
                    code_         = a_code;
                    headers_.clear();
                    body_.clear();
                    json_.clear();
                    json_         = Json::Value::null;
                    content_type_ = "";
                    rtt_          = 0;
                    if ( nullptr != exception_ ) {
                        delete exception_;
                        exception_ = nullptr;
                    }
                }
                
                /**
                 * @return R/O access to HTTP Status Code.
                 */
                inline const uint16_t& code () const
                {
                    return code_;
                }
                
                /**
                 * @return R/O access to HTTP headers.
                 */
                inline const std::map<std::string, std::string>& headers () const
                {
                    return headers_;
                }
                
                /**
                 * @return R/O access to HTTP Status Code.
                 */
                inline std::string const status () const
                {
                    const auto it = cc::i18n::Singleton::k_http_status_codes_map_.find(code_);
                    if ( cc::i18n::Singleton::k_http_status_codes_map_.end() != it ) {
                        return it->second;
                    }
                    return "???";
                }
                
                /**
                 * @return R/O access to body.
                 */
                inline const std::string& body () const
                {
                    return body_;
                }
                
                /**
                 * @return R/O access to body as JSON.
                 */
                inline const Json::Value& json () const
                {
                    return json_;
                }
                
                /**
                 * @return R/O access to HTTP Content-Type header value.
                 */
                inline const std::string& content_type () const
                {
                    return content_type_;
                }
                
                /**
                 * @return R/O access to Round time trip in milliseconds.
                 */
                inline const size_t& rtt () const
                {
                    return rtt_;
                }
                
                /**
                 * @return R/O access a tracked exception.
                 */
                inline const cc::Exception* exception () const
                {
                    return exception_;
                }
                    
            }; // class 'Response'
        
            typedef ::cc::Exception BadRequestException;

        } // end of namespace 'deferrable'

    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_TYPES_H_
