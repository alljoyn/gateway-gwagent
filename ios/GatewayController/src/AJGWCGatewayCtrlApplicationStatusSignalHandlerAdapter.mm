/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#import "AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter.h"
#import "AJGWCGatewayCtrlTPApplicationStatus.h"
#import "alljoyn/about/AJNConvertUtil.h"

AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter::AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter(id <AJGWCGatewayCtrlApplicationStatusSignalHandler> handle)
{
    applicationStatusSignalHandler = handle;
}

AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter::~AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter()
{
}


void AJGWCGatewayCtrlApplicationStatusSignalHandlerAdapter::onStatusChanged(qcc::String appId, const ajn::services::GatewayCtrlTPApplicationStatus *TPApplicationStatus)
{
    AJGWCGatewayCtrlTPApplicationStatus* gwTPApplicationStatus = [[AJGWCGatewayCtrlTPApplicationStatus alloc] initWithHandle:const_cast<ajn::services::GatewayCtrlTPApplicationStatus*>(TPApplicationStatus)];
    [applicationStatusSignalHandler onStatusChanged:[AJNConvertUtil convertQCCStringtoNSString:appId] status:gwTPApplicationStatus];
}