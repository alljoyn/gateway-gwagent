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
# Strips down the AllJoyn Core Android SDK to the minimum needed for building the gateway project
#
#   DEPENDENCIES_DIR - directory contains the zip files of the dependencies
#   CORE_VERSION - version of AllJoyn core
#   WORKING_DIR - directory for working with files
#   ARTIFACTS_DIR - directory to which to copy the prepared dependencies

set -o nounset
set -o errexit
set -o verbose
set -o xtrace


# check for required env variables
for var in DEPENDENCIES_DIR CORE_VERSION ARTIFACTS_DIR WORKING_DIR
do
    if [ -z "${!var:-}" ]
    then
        printf "$var must be defined!\n"
        exit 1
    fi
done

# set variables for different directories needed
extractedSdks=${WORKING_DIR}/unzipped_sdks
sdkStaging=${WORKING_DIR}/sdk_stage
sdksDir=${ARTIFACTS_DIR}/sdks

mkdir -p $sdksDir
mkdir -p $extractedSdks
mkdir -p $sdkStaging

# Extract subset of Android SDK for dbg and rel variants
for variantString in dbg rel
do
    # determine the name of the SDK
    sdkName=alljoyn-${CORE_VERSION}-android-sdk-$variantString

    # create directory to unzip into
    zipContent=$extractedSdks/$sdkName
    mkdir -p $zipContent

    zipFile=$sdkName.zip

    # unzip the SDK into the directory
    unzip ${DEPENDENCIES_DIR}/$zipFile -d $zipContent

    # create directory for staging content for stripped down zip file
    zipStage=$sdkStaging/$sdkName
    mkdir -p $zipStage

    # determine the source and destination directories for copying files
    javaDir=alljoyn-android/core/alljoyn-${CORE_VERSION}-$variantString/java
    srcJavaDir=$zipContent/$javaDir
    destJavaDir=$zipStage/$javaDir

    # create the destination directory structure and copy a subset of the files from the source
    mkdir -p $destJavaDir
    cp -r  $srcJavaDir/lib $destJavaDir/
    cp -r  $srcJavaDir/jar $destJavaDir/

    # create the stripped down zip file
    cd $zipStage
    zip $sdksDir/$zipFile -r alljoyn-android

done