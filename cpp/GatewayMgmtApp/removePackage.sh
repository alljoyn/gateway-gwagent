#!/bin/sh 

# # 
#    Copyright (c) 2016 Open Connectivity Foundation and AllJoyn Open
#    Source Project Contributors and others.
#    
#    All rights reserved. This program and the accompanying materials are
#    made available under the terms of the Apache License, Version 2.0
#    which accompanies this distribution, and is available at
#    http://www.apache.org/licenses/LICENSE-2.0

#

# sample script to remove an application until package manager is available
# 
# this just removes the directory, but also reminds to delete the user

# exit if connectorId wasn't given
if [ -z "$1" ]; then
    echo "Usage: $(basename $0) [connectorId]"
    exit 1
fi

# the base installation directory for alljoyn and the gateway agent
baseDir=/opt/alljoyn
connectorId=$1

connectorAppDir=$baseDir/apps/$connectorId


if [ -d "$connectorAppDir" ]; then
    echo "Removing connectorApp with connectorId: $connectorId"; 
    rm -fr $connectorAppDir || { echo "Error: unable to remove directory: $connectorAppDir" 1>&2; exit 1; }
else
    echo "Error: a connectorApp with connectorId: $connectorId is not installed" 1>&2
    exit 1
fi

echo "Successfully removed connectorApp with connectorId: $connectorId"; 

id -u "$connectorId" 1> /dev/null
if [ $? = 0 ]; then
    echo "To remove user use: userdel $connectorId"
fi

exit 0
