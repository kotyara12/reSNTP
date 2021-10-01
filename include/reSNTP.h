/* 
   EN: Launcher for automatic time synchronization with SNTP servers
   RU: Модуль запуска автоматической синхронизации времени с SNTP серверами
   --------------------------
   (с) 2021 Разживин Александр | Razzhivin Alexander
   kotyara12@yandex.ru | https://kotyara12.ru | tg: @kotyara1971
*/

#ifndef __RE_SNTP_H__
#define __RE_SNTP_H__ 

#ifdef __cplusplus
extern "C" {
#endif

void sntpStartSNTP();
void sntpStopSNTP();

bool sntpTaskCreate(bool createSuspended);

bool sntpEventHandlerRegister();

#ifdef __cplusplus
}
#endif

#endif // __RE_SNTP_H__