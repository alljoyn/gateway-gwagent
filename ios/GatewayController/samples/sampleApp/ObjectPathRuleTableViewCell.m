/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
 *    Source Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#import "ObjectPathRuleTableViewCell.h"


@implementation ObjectPathRuleTableViewCell

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];
}
- (IBAction)configureButtonTouchUpInside:(id)sender {
    [self.rules switchObjectPathConfiguration:_objPathInfo];

    [self updateCell];
}
- (IBAction)allowSubObjectsTouchUpInside:(id)sender {
    [self.rules switchObjectPathAllowSubObjects:_objPathInfo];

    [self updateCell];
}

-(void)setObjPathInfo:(VisualObjPathInfo *)objPathInfo
{
    _objPathInfo = objPathInfo;

    NSString *friendlyName = [objPathInfo.objectPath friendlyName];

    if (![friendlyName isEqual:@""]) {
        self.ruleNameLbl.text = friendlyName;
    } else {
        self.ruleNameLbl.text = [objPathInfo.objectPath path];
    }

    [self updateCell];
}

-(void)updateCell
{
    if (_objPathInfo.configured) {
        [self.configureButton setTitle:@"Added" forState:UIControlStateNormal];
    } else {
        [self.configureButton setTitle:@"Add" forState:UIControlStateNormal];
    }

    if ([_objPathInfo.objectPath isPrefix]) {
        [self.allowSubObjectsButton setTitle:@"Sub Objects Allowed" forState:UIControlStateNormal];
    } else {
        [self.allowSubObjectsButton setTitle:@"Allow Sub Objects" forState:UIControlStateNormal];
    }

    self.configureButton.enabled = _objPathInfo.enabled;
    if (![_objPathInfo.objectPath isPrefixAllowed]) {
        [self.allowSubObjectsButton setTitle:@"Sub Obj Not Allowed" forState:UIControlStateNormal];
        self.allowSubObjectsButton.enabled = NO;
    } else {
        self.allowSubObjectsButton.enabled = _objPathInfo.enabled;
    }
}
@end