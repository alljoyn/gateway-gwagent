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

#import <Foundation/Foundation.h>
#import "alljoyn/Status.h"
#import "AJNBusAttachment.h"
#import "AJGWCGatewayCtrlGateway.h"

/**
 * This class includes the main functionality for the Gateway Controller Application
 */
@interface AJGWCGatewayCtrlGatewayController : NSObject

/**
 * GetInstance
 * @return The {@link AJGWCGatewayCtrlGatewayController} object
 */
+ (id)sharedInstance;

/**
 * Initialize the gateway controller
 * @param bus {@link AJNBusAttachment} to use
 */
+ (void)startWithBus:(AJNBusAttachment *) bus;

/**
 * Shutdown the gateway controller
 */
- (void)shutdown;

/**
 * @return {@link AJNBusAttachment} that is used by the {@link AJGWCGatewayController}
 */
- (AJNBusAttachment*)busAttachment;

/**
 * create a Gateway by parsing announce descriptions.
 * @param gatewayBusName BusName of device received in announce
 * @param objectDescs ObjectDescriptions received in announce
 * @param aboutData The data sent with the Announcement
 * @return a {@link AJGWCGatewayCtrlGateway} object
 */
- (AJGWCGatewayCtrlGateway*)createGatewayWithBusName:(NSString*) gatewayBusName objectDescs:(NSDictionary *) objectDescs  aboutData:(NSDictionary *) aboutData;

/**
 * getGateway - get a Gateway using the busName
 * @param gatewayBusName gatewayBusName to get
 * @return the {@link AJGWCGatewayCtrlGateway} or NULL if not found
 */
- (AJGWCGatewayCtrlGateway*)gateway:(NSString *) gatewayBusName;

/**
 * deleteGateway - delete a GateWay from the map
 * @param gatewayBusName AJGWCGatewayCtrlGateway to delete
 * @return Returns the deleteAllGateways status {@link QStatus}
 */
- (QStatus)deleteGateway:(NSString *) gatewayBusName;

/**
 * deleteAllGateways - shutdown and delete all Gateways from the controller
 * @return Returns the deleteAllGateways status {@link QStatus}
 */
- (QStatus)deleteAllGateways;

/**
 * Get a dictionary of all Gateways
 * @return Gateways dictionary
 */
- (NSDictionary*)gateways;

@end
