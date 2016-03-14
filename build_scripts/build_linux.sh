#!/bin/bash

# Copyright AllSeen Alliance. All rights reserved.
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
#


#
# Builds an archive of the Linux build for gwagent
#
#   BUILD_VARIANT - release or debug (default to release if not given)
#   GWAGENT_SDK_VERSION - version name to use in building the archive file (version number left out if not given)
#   GWAGENT_SRC_DIR - root directory of the gwagent git repo (defaults to relative location if not given)
#   ARTIFACTS_DIR - directory to copy build products (defaults to build/jobs/artifacts)
#   WORKING_DIR - directory for working with files (defaults to build/jobs/tmp)
#   CPU - CPU parameter to give to scons, x86_64 or x86 (defaults to x86_64)


set -o nounset
set -o errexit
set -o verbose
set -o xtrace

#========================================
# Set default values for any unset environment variables

export BUILD_VARIANT=${BUILD_VARIANT:-release}

if [ -z "${GWAGENT_SRC_DIR:-}" ]
then
    # set it to the top level directory for the git repo
    # (based on relative position of the build_scripts)
    export GWAGENT_SRC_DIR=$(dirname $(dirname $(readlink -f $0)))
fi

export ARTIFACTS_DIR=${ARTIFACTS_DIR:-$GWAGENT_SRC_DIR/build/jobs/artifacts}
export WORKING_DIR=${WORKING_DIR:-$GWAGENT_SRC_DIR/build/jobs/tmp}
export CPU=${CPU:-x86_64}

#========================================
# set variables for the different directories needed
sdkStaging=${WORKING_DIR}/sdk_stage
sdksDir=${ARTIFACTS_DIR}/sdks


# remove any existing directory and contents
rm -fr $sdkStaging

# create the directories needed
mkdir -p $sdkStaging
mkdir -p $sdksDir


# determine the variant string
case ${BUILD_VARIANT} in
    debug)
        variantString=dbg
        ;;
    release)
        variantString=rel
        ;;
esac


#========================================
# build the code

pushd ${GWAGENT_SRC_DIR}
scons V=1 OS=linux CPU=${CPU} BINDINGS=cpp BR=off VARIANT=${BUILD_VARIANT} WS=fix  POLICYDB=on
git diff
popd
