#!/bin/bash

# # 
#    Copyright (c) 2016 Open Connectivity Foundation and AllJoyn Open
#    Source Project Contributors and others.
#    
#    All rights reserved. This program and the accompanying materials are
#    made available under the terms of the Apache License, Version 2.0
#    which accompanies this distribution, and is available at
#    http://www.apache.org/licenses/LICENSE-2.0

#


#
# Builds a source tar of the gwagent project
#
#   GWAGENT_SRC_DIR - root directory of the gwagent git repo (defaults to relative location if not given)
#   GWAGENT_SDK_VERSION - version name to use in building the archive file (version number left out if not given)
#   ARTIFACTS_DIR - directory to copy build products (default to build/jobs/artifacts)
#   WORKING_DIR - directory for working with files (default to build/jobs/tmp)


set -o nounset
set -o errexit
set -o verbose
set -o xtrace


#========================================
# Set default values for any unset environment variables

if [ -z "${GWAGENT_SRC_DIR:-}" ]
then
    # set it to the top level directory for the git repo
    # (based on relative position of the build_scripts)
    export GWAGENT_SRC_DIR=$(dirname $(dirname $(readlink -f $0)))
fi

export ARTIFACTS_DIR=${ARTIFACTS_DIR:-$GWAGENT_SRC_DIR/build/jobs/artifacts}
export WORKING_DIR=${WORKING_DIR:-$GWAGENT_SRC_DIR/build/jobs/tmp}


#========================================

# create the directories needed
mkdir -p ${ARTIFACTS_DIR}
mkdir -p ${WORKING_DIR}

versionString=""
if [ -n "${GWAGENT_SDK_VERSION:-}" ]
then
    versionString="${GWAGENT_SDK_VERSION}-"
fi

outputTarFileName=alljoyn-gwagent-${versionString}src.tar

pushd ${GWAGENT_SRC_DIR}
git archive --prefix=alljoyn-gwagent-${versionString}src/gateway/gwagent/ HEAD^{tree} -o ${WORKING_DIR}/$outputTarFileName

cd ../../core/alljoyn
git archive --prefix=alljoyn-gwagent-${versionString}src/core/alljoyn/ HEAD^{tree} build_core -o ${WORKING_DIR}/core.tar

cd ../../services/base
git archive --prefix=alljoyn-gwagent-${versionString}src/services/base/ HEAD^{tree} sample_apps -o ${WORKING_DIR}/services.tar

cd ../..

tar --concatenate --file=${WORKING_DIR}/$outputTarFileName ${WORKING_DIR}/core.tar
tar --concatenate --file=${WORKING_DIR}/$outputTarFileName ${WORKING_DIR}/services.tar

gzip ${WORKING_DIR}/$outputTarFileName

pushd ${WORKING_DIR}
md5sum $outputTarFileName.gz > md5sum.txt
popd


cp ${WORKING_DIR}/$outputTarFileName.gz ${ARTIFACTS_DIR}
cp ${WORKING_DIR}/md5sum.txt ${ARTIFACTS_DIR}