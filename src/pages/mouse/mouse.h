// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace settings {

// Client of the compositor's org.cutefish.Mouse1 service.
class Mouse
{
public:
    Mouse();

    // Whether the service runs and a mouse is connected.
    bool isAvailable() const;

    bool leftHanded() const;
    void setLeftHanded(bool enabled);

    bool acceleration() const;
    void setAcceleration(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    // libinput's pointer speed, -1 to 1.
    double pointerAcceleration() const;
    void setPointerAcceleration(double value);

    cutefish::base::Signal<>& changed() { return m_service.changed; }

private:
    CompositorService m_service;
};

} // namespace settings
