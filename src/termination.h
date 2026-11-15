#pragma once
#include <csignal>

// The host may wait synchronously for a preview to stop. Embedded renderers
// must retain default termination even when the GL driver is blocked.
inline void configureTermination(bool embedded,void (*graceful)(int)){
    std::signal(SIGTERM,embedded?SIG_DFL:graceful);
    std::signal(SIGINT,embedded?SIG_DFL:graceful);
}
