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
#ifndef CASPER_JOB_DEFERRABLE_ARGUMENTS_H_
#define CASPER_JOB_DEFERRABLE_ARGUMENTS_H_

#include "cc/non-movable.h"

#include <inttypes.h>
#include <string>
#include <map>

#include "cc/macros.h"

namespace casper
{

    namespace job
    {

        namespace deferrable
        {
        
            template <class P>
            class Arguments : public cc::NonMovable
            {

            protected: // Data
                
                P parameters_;
                
            public: // Constructor(s) / Destructor
                
                Arguments () = delete;

                /**
                 * @brief Default constructor.
                 */
                Arguments (const P& a_parameters)
                    : parameters_(a_parameters)
                {
                    /* empty */
                }
                
                /**
                 * @brief Copy constructor.
                 */
                Arguments (const Arguments& a_arguments)
                    : parameters_(a_arguments.parameters_)
                {
                    /* empty */
                }
                
                /**
                 * @brief Destructor.
                 */
                virtual ~Arguments()
                {
                    /* empty */
                }
                
            public: // Overloaded Operator(s)
                
                void operator = (Arguments const&)  = delete;  // assignment is not allowed
                
            public: // R/O Method(s) / Function(s)
                
                /**
                 * @brief R/O access to \link P \link.
                 */
                inline const P& parameters () const
                {
                    return parameters_;
                }
            
            public: // R/W Method(s) / Function(s)
                /**
                 * @brief R/W access to \link P \link.
                 */
                inline P& parameters ()
                {
                    return parameters_;
                }
                
            };
        
        } // end of namespace 'deferrable'

    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_DEFERRABLE_ARGUMENTS_H_
