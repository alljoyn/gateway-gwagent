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

#ifndef AJINITIALIZER_H_
#define AJINITIALIZER_H_

#include <alljoyn/Status.h>

/**
 * class AJInitializer
 * Utility class for handling AllJoyn lifecycle methods
 */

namespace ajn {
namespace gw {
namespace common {

class AJInitializer {
  public:
    /**
     * Calls AllJoynInit(). If bundled router is enabled it also calls AllJoynRouterInit()
     * @return ER_OK if initialization succeeded
     */
    AJInitializer();

    /**
     * ~AJInitializer
     * Calls AllJoynRouterShutdown() if bundled router is enabled and then calls AllJoynShutdown()
     */
    ~AJInitializer();

    /*
     * Return the status of AllJoynInit and AllJoynRouterInit
     * @return QStatus
     */
    QStatus Status() const;

  private:
    /*
     * Status of AllJoynInit and AllJoynRouterInit
     */
    QStatus m_Status;
};

}
}
}
#endif /* AJINITIALIZERGWAGENT_H_ */