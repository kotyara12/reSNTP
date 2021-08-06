#include <sys/time.h> 
#include "esp_sntp.h"
#include "project_config.h"
#include "rLog.h"
#include "reSNTP.h"
#include "reEvents.h"

static const char * sntpTAG = "SNTP";

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- SNTP sychronization ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void sntpSyncNotification(struct timeval *tv)
{
  time_t now = 0;
  struct tm timeinfo;
  char strftime_buf[20];

  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year < (1970 - 1900)) {
    eventLoopPost(RE_SNTP_EVENTS, RE_SNTP_SYNC_FAILED, nullptr, 0, portMAX_DELAY);
    rlog_e(sntpTAG, "Time synchronization failed!");
  }
  else {
    eventLoopPost(RE_SNTP_EVENTS, RE_SNTP_SYNC_OK, nullptr, 0, portMAX_DELAY);
    strftime(strftime_buf, sizeof(strftime_buf), "%d.%m.%Y %H:%M:%S", &timeinfo);
    rlog_i(sntpTAG, "Time synchronization completed, current time: %s", strftime_buf);
  };
}

void sntpStopSNTP()
{
  if (sntp_enabled()) {
    sntp_stop();
    eventLoopPost(RE_SNTP_EVENTS, RE_SNTP_STOPPED, nullptr, 0, portMAX_DELAY);
    rlog_i(sntpTAG, "Time synchronization stopped");
  };
}

void sntpStartSNTP()
{
  // Stop time synchronization if it was started
  sntpStopSNTP();

  rlog_i(sntpTAG, "Starting time synchronization with SNTP servers for a zone %s...", CONFIG_SNTP_TIMEZONE);

  // Configuring synchronization parameters
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  #if defined(CONFIG_SNTP_TIMEZONE)
  setenv("TZ", CONFIG_SNTP_TIMEZONE, 1);
  tzset(); 
  #endif
  #if defined(CONFIG_SNTP_SERVER0)
  sntp_setservername(0, (char*)CONFIG_SNTP_SERVER0);
  #endif
  #if defined(CONFIG_SNTP_SERVER1)
  sntp_setservername(1, (char*)CONFIG_SNTP_SERVER1);
  #endif
  #if defined(CONFIG_SNTP_SERVER2)
  sntp_setservername(2, (char*)CONFIG_SNTP_SERVER2);
  #endif
  #if defined(CONFIG_SNTP_SERVER3)
  sntp_setservername(3, (char*)CONFIG_SNTP_SERVER3);
  #endif
  #if defined(CONFIG_SNTP_SERVER4)
  sntp_setservername(4, (char*)CONFIG_SNTP_SERVER4);
  #endif
  sntp_set_time_sync_notification_cb(sntpSyncNotification); 

  sntp_init();
  
  eventLoopPost(RE_SNTP_EVENTS, RE_SNTP_STARTED, nullptr, 0, portMAX_DELAY);
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- WiFi event handler -------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

static void sntpEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  // STA connected
  if (event_id == RE_WIFI_STA_CONNECTED) {
    sntpStartSNTP();
  }

  // STA disconnected
  else if ((event_id == RE_WIFI_STA_DISCONNECTED) || (event_id == RE_WIFI_STA_STOPPED)) {
    sntpStopSNTP();
  }
}

bool sntpRegister()
{
  return eventHandlerRegister(RE_WIFI_EVENTS, ESP_EVENT_ANY_ID, &sntpEventHandler, nullptr);
}


