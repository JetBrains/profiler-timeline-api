#pragma once

#include "events.hpp"
#include <guiddef.h>

namespace events {

    ///////////////////////////////
    //// Debug output provider ////
    ///////////////////////////////

    PROVIDER_BEGIN(JetBrains_Common_DebugOutput, { 0x945680D2, 0x0B5F, 0x4CA9, { 0x90, 0xD8, 0xE0, 0x8C, 0xBD, 0xFC, 0x27, 0x91 } })
        EVENT(DebugOutput, 1, 0,
            uint32_t ProcessId;
            const char * DebugString;

            TO_TUPLE(ProcessId, DebugString)
        )
    PROVIDER_END

}
