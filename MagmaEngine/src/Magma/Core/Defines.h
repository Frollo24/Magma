#pragma once

#define MGM_BIT(x) (1 << x)

#define MGM_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }