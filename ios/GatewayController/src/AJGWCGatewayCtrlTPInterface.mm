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

#import "AJGWCGatewayCtrlTPInterface.h"
#import "alljoyn/about/AJNConvertUtil.h"

@interface AJGWCGatewayCtrlTPInterface ()

@property (nonatomic) ajn::services::GatewayCtrlTPInterface* handle;

@end

@implementation AJGWCGatewayCtrlTPInterface

- (void)dealloc
{
    delete self.handle;
    self.handle = NULL;
}

- (id)initWithHandle:(ajn::services::GatewayCtrlTPInterface) handle
{
    self = [super init];
    if (self) {
        self.handle = new ajn::services::GatewayCtrlTPInterface(handle.GetName(), handle.GetFriendlyName(), handle.IsSecured());
    }
    return self;
}

- (id)initWithInterfaceName:(NSString*) name friendlyName:(NSString*) friendlyName  isSecured:(bool) isSecured
{
    self = [super init];
	if (self) {
		self.handle = new ajn::services::GatewayCtrlTPInterface([AJNConvertUtil convertNSStringToQCCString:name], [AJNConvertUtil convertNSStringToQCCString:friendlyName], isSecured);
        
	}
	return self;
}

- (NSString*)interfaceName
{
    qcc::String name = self.handle->GetName();
    
    return [AJNConvertUtil convertQCCStringtoNSString:name];
}

- (NSString*)friendlyName
{
    return [AJNConvertUtil convertQCCStringtoNSString:self.handle->GetFriendlyName()];
}

- (bool)isSecured
{
    return self.handle->IsSecured();
}

- (ajn::services::GatewayCtrlTPInterface*)handle
{
    return _handle;
}

- (id)copyWithZone:(NSZone *)zone {
    AJGWCGatewayCtrlTPInterface *objectCopy = [[AJGWCGatewayCtrlTPInterface allocWithZone:zone] initWithHandle:*(self.handle)];
    
    return objectCopy;
}

- (NSUInteger)hash {
    return [self.interfaceName hash];
}

- (BOOL)isEqual:(id)anObject {
    if (![anObject isKindOfClass:[AJGWCGatewayCtrlTPInterface class]]) return NO;
    AJGWCGatewayCtrlTPInterface *otherTPInterface = (AJGWCGatewayCtrlTPInterface *)anObject;
    return [otherTPInterface.interfaceName isEqualToString:self.interfaceName];
}



@end