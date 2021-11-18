/**
* @file main.cc
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
* casper-job is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with casper-job if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "version.h"

#include <iostream>

#include "cc/easy/job/handler.h"

#include "casper/job/demo/basic.h"
#include "casper/job/demo/base.h"

int main(int argc, const char * argv[]) {

    const char* short_info = strrchr(CASPER_JOB_INFO, '-');
    if ( nullptr == short_info ) {
        short_info = CASPER_JOB_INFO;
    } else {
        short_info++;
    }

    // ... show banner ...
    fprintf(stdout, "%s\n", CASPER_JOB_BANNER);
    fflush(stdout);
    
    //
    // LOG FILTERING:
    //
    // tail -f /usr/local/var/log/<process-name>/<tube-name>.1.log
    //
    // ... run ...
    return cc::easy::job::Handler::GetInstance().Start(
        /* a_arguments */
        {
            /* abbr_           */ CASPER_JOB_ABBR,
            /* name_           */ CASPER_JOB_NAME,
            /* version_        */ CASPER_JOB_VERSION,
            /* rel_date_       */ CASPER_JOB_REL_DATE,
            /* rel_branch_     */ CASPER_JOB_REL_BRANCH,
            /* rel_hash_       */ CASPER_JOB_REL_HASH,
            /* info_           */ short_info, // short version of CASPER_JOB_INFO
            /* banner_         */ CASPER_JOB_BANNER,
            /* argc_           */ argc,
            /* argv_           */ const_cast<const char** const >(argv),
        },
        /* a_factories */
        {
            {
                ::casper::job::demo::Basic::sk_tube_, [] (const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config) -> cc::easy::job::Job* {
                    return new ::casper::job::demo::Basic(a_loggable_data, a_config);
                }
            },
            {
                ::casper::job::demo::Base::sk_tube_, [] (const ev::Loggable::Data& a_loggable_data, const cc::easy::job::Job::Config& a_config) -> cc::easy::job::Job* {
                    return new ::casper::job::demo::Base(a_loggable_data, a_config);
                }
            }
        },
        /* a_polling_timeout */ 20.0 /* milliseconds */
    );
}
