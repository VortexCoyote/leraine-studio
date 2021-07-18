#include "global-functions.h"

#include <math.h>

bool GlobalFunctions::FloatCompare(const float InA, const float InB, const float InThreshold)
{
    return fabs(InA - InB) <= InThreshold;
}
