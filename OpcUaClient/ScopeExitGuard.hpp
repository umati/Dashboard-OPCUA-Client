 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

class ScopeExitGuard
{
public:
    ScopeExitGuard(std::function<void(void)> onExit)
        : m_onExit(onExit)
    {
    }
    ~ScopeExitGuard()
    {
        if (m_onExit)
            m_onExit();
    }
private:
    std::function<void(void)> m_onExit;
};