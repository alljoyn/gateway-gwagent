/******************************************************************************
 *  *    Copyright (c) Open Connectivity Foundation (OCF) and AllJoyn Open
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
 *     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *     PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *     PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#ifndef _ANNOUNCEMENTDATA_H_
#define _ANNOUNCEMENTDATA_H_

#include <vector>
#include <alljoyn/Status.h>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObjectDescription.h>

namespace ajn {
namespace gwc {
/**
 * Announcement data - information coming from the announcement
 * The application should create and maintain a vector of AnnouncementData objects based on the current set of announcement in the network. See sample application for more.
 */
class AnnouncementData {
  public:
    /**
     * AnnouncementData
     * @param portNumber Announcemt port number
     * @param aboutData As was received from the announcement handler
     * @param objectDescriptions As was received from the announcement handler
     */
    AnnouncementData(uint16_t portNumber, const ajn::AboutData& aboutData, const ajn::AboutObjectDescription& objectDescriptions);

    /**
     * GetObjectDescriptions
     * @return objectDescriptions As was received from the announcement handler
     */

    const ajn::AboutObjectDescription& GetObjectDescriptions() const { return m_ObjectDescriptions; }

    /**
     * GetAboutData
     * @return aboutData As was received from the announcement handler
     */

    const ajn::AboutData& GetAboutData() const { return m_AboutData; }

    const short getPortNumber() const { return m_PortNumber; }

  private:
    ajn::AboutData m_AboutData;
    ajn::AboutObjectDescription m_ObjectDescriptions;
    short m_PortNumber;


};
}
}



#endif /* defined(_ANNOUNCEMENTDATA_H_) */