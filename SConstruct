# Copyright (c) 2013-2014, AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import os

gatewayDir = os.getcwd()
env = SConscript('../../core/alljoyn/build_core/SConscript')

vars = Variables()
vars.Add(EnumVariable('BR', 'Have bundled router built-in for C++ test samples', 'on', allowed_values=('on', 'off')))

if env['BR'] == 'on':
    os.chdir("../../core/alljoyn/alljoyn_core/router/posix")
    os.system("patch daemon-main.cc < " + gatewayDir + "/patches/daemon-main.patch")
    os.system("patch SConscript < " + gatewayDir + "/patches/daemon-sconscript.patch")
    os.chdir(gatewayDir)

vars.Add('BINDINGS', 'Bindings to build (comma separated list): cpp, java', 'cpp,java')
vars.Add(PathVariable('ALLJOYN_DISTDIR',
                      'Directory containing a built AllJoyn Core dist directory.',
                      os.environ.get('ALLJOYN_DISTDIR')))
                      
vars.Add(EnumVariable('BUILD_SERVICES_SAMPLES',
                      'Build the services samples.',
                      'off',
                      allowed_values = ['off', 'on']))

vars.Add(PathVariable('APP_COMMON_DIR',
                      'Directory containing common sample application sources.',
                      os.environ.get('APP_COMMON_DIR','../../services/base/sample_apps')))
					  
vars.Add(PathVariable('LIBXML2_BASE',
                      'Directory containing libxml2 include files.',
                      os.environ.get('LIBXML2_BASE','/usr/include/libxml2')))					  
                      
vars.Update(env)
Help(vars.GenerateHelpText(env))

if env.get('ALLJOYN_DISTDIR'):
    # normalize ALLJOYN_DISTDIR first
    env['ALLJOYN_DISTDIR'] = env.Dir('$ALLJOYN_DISTDIR')
    env.Append(CPPPATH = [ env.Dir('$ALLJOYN_DISTDIR/cpp/inc'),
                           env.Dir('$ALLJOYN_DISTDIR/about/inc'),
                           env.Dir('$ALLJOYN_DISTDIR/services_common/inc') ])
    env.Append(LIBPATH = [ env.Dir('$ALLJOYN_DISTDIR/cpp/lib'),
                           env.Dir('$ALLJOYN_DISTDIR/about/lib'),
                           env.Dir('$ALLJOYN_DISTDIR/services_common/lib') ])

if env.get('APP_COMMON_DIR'):
    # normalize APP_COMMON_DIR
    env['APP_COMMON_DIR'] = env.Dir('$APP_COMMON_DIR')

env['bindings'] = set([ b.strip() for b in env['BINDINGS'].split(',') ])

env.SConscript('SConscript')

def unpatch(target, source, env):
    if env['BR'] == 'on':
        os.chdir("../../core/alljoyn/alljoyn_core/router/posix")
        os.system("patch -R daemon-main.cc < " + gatewayDir + "/patches/daemon-main.patch")
        os.system("patch -R SConscript < " + gatewayDir + "/patches/daemon-sconscript.patch")
        os.chdir(gatewayDir)
    return None


unpatch_command = env.Command('unpatch', [], unpatch)

env.Depends(unpatch_command, 'build')
env.Default(unpatch_command)
