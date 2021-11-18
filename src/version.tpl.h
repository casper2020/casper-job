/**
 * @file version.h
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
#ifndef CASPER_JOB_VERSION_H_
#define CASPER_JOB_VERSION_H_

#ifndef CASPER_JOB_ABBR
#define CASPER_JOB_ABBR "cj"
#endif

#ifndef CASPER_JOB_NAME
#define CASPER_JOB_NAME "casper-job@b.n.s@"
#endif

#ifndef CASPER_JOB_VERSION
#define CASPER_JOB_VERSION "x.x.x"
#endif

#ifndef CASPER_JOB_REL_DATE
#define CASPER_JOB_REL_DATE "r.r.d"
#endif

#ifndef CASPER_JOB_REL_BRANCH
#define CASPER_JOB_REL_BRANCH "r.r.b"
#endif

#ifndef CASPER_JOB_REL_HASH
#define CASPER_JOB_REL_HASH "r.r.h"
#endif

#ifndef CASPER_JOB_INFO
#define CASPER_JOB_INFO CASPER_JOB_NAME " v" CASPER_JOB_VERSION
#endif

#define CASPER_JOB_BANNER \
" ____    _    ____  ____  _____ ____           _  ___  ____\n" \
"/ ___|  / \\  / ___||  _ \\| ____|  _ \\         | |/ _ \\| __ )\n" \
"| |     / _ \\ \\___ \\| |_) |  _| | |_) |____ _  | | | | |  _ \\\n" \
"| |___ / ___ \\ ___) |  __/| |___|  _ <_____| |_| | |_| | |_) |\n" \
"\\____/_/   \\_\\____/|_|   |_____|_| \\_\\     \\___/ \\___/|____/"

#endif // CASPER_JOB_VERSION_H_
