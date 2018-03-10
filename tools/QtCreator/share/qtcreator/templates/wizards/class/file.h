#pragma once

@if '%{Base}'
#include <%{Base}.h>
@endif

namespace adb
{
@if '%{Base}'
class %{CN} : public %{Base}
@else
class %{CN}
@endif
{
public:
    
};
}
