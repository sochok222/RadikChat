#ifndef CLIENT_C_CRITICAL_SECTION_H
#define CLIENT_C_CRITICAL_SECTION_H

#include <synchapi.h>

typedef PCRITICAL_SECTION CriticalSection;

bool initialize_critical_section(CriticalSection *criticalSection);
void delete_critical_section(CriticalSection *criticalSection);

#endif //CLIENT_C_CRITICAL_SECTION_H
