#ifndef RTOS_APP_H
#define RTOS_APP_H

void Rtos_Start(void);
void Rtos_PrintStatus(void);
void Rtos_AssertFailed(const char *file, int line);

#endif /* RTOS_APP_H */
