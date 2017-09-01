#include "base/common.h"

#include <stdio.h>

static char buff[BUILD_TIME_LNG];

char *build_time(void)
{
snprintf(buff,BUILD_TIME_LNG,"(Built: %s %s)",__DATE__,__TIME__);

return buff;
}
