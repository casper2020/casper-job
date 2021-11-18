/**
 * @file exceptions.h
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
#ifndef CASPER_JOB_HANDLER_H_
#define CASPER_JOB_HANDLER_H_

#include "cc/easy/job/handler.h"

namespace casper
{

    namespace job
    {

        typedef ::cc::easy::job::Handler Handler;

    } // end of namespace 'job'

} // end of namespace 'casper'

#endif // CASPER_JOB_HANDLER_H_
