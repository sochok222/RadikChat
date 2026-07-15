#include "critical_section.h"

bool initialize_critical_section(CriticalSection *criticalSection)
{
    *criticalSection = malloc(sizeof(CRITICAL_SECTION));
    if (*criticalSection == NULL)
        return false;

    InitializeCriticalSection(*criticalSection);
    return true;
}

void delete_critical_section(CriticalSection *criticalSection)
{
    if (*criticalSection != NULL)
        free(*criticalSection);
    *criticalSection = NULL;
}
