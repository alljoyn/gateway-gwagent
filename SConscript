# # Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
#    Source Project (AJOSP) Contributors and others.
#
#    SPDX-License-Identifier: Apache-2.0
#
#    All rights reserved. This program and the accompanying materials are
#    made available under the terms of the Apache License, Version 2.0
#    which accompanies this distribution, and is available at
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
#    Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for
#    any purpose with or without fee is hereby granted, provided that the
#    above copyright notice and this permission notice appear in all
#    copies.
#
#     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
#     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
#     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
#     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
#     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
#     PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#     PERFORMANCE OF THIS SOFTWARE.

import os

Import('env')

env['_ALLJOYN_GATEWAY'] = True

if not env.has_key('_ALLJOYN_ABOUT_') and os.path.exists('../../core/alljoyn/services/about/SConscript'):
    env.SConscript('../../core/alljoyn/services/about/SConscript')

if not env.has_key('_ALLJOYN_SERVICES_COMMON_') and os.path.exists('../../services/base/services_common/SConscript'):
    env.SConscript('../../services/base/services_common/SConscript')

if not env.has_key('_ALLJOYN_NOTIFICATION_') and os.path.exists('../../services/base/notification/SConscript'):
    env.SConscript('../../services/base/notification/SConscript')

if not env.has_key('_ALLJOYN_CONFIG_') and os.path.exists('../../services/base/config/SConscript'):
    env.SConscript('../../services/base/config/SConscript')
    
if 'cpp' in env['bindings'] and not env.has_key('_ALLJOYNCORE_') and os.path.exists('../../core/alljoyn/alljoyn_core/SConscript'):
    env.SConscript('../../core/alljoyn/alljoyn_core/SConscript')

if 'java' in env['bindings'] and not env.has_key('_ALLJOYN_JAVA_') and os.path.exists('../../core/alljoyn/alljoyn_java/SConscript'):
    env.SConscript('../../core/alljoyn/alljoyn_java/SConscript')

gateway_env = env.Clone()

for b in gateway_env['bindings']:
    if os.path.exists('%s/SConscript' % b):
        gateway_env.VariantDir('$OBJDIR/%s' % b, b, duplicate = 0)

gateway_env.SConscript(['$OBJDIR/%s/SConscript' % b for b in env['bindings'] if os.path.exists('%s/SConscript' % b) ],
                      exports = ['gateway_env'])
