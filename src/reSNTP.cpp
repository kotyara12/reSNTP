#include <sys/time.h> 
#include "esp_sntp.h"
#include "project_config.h"
#include "def_sntp.h"
#include "rLog.h"
#include "reSNTP.h"
#include "reEvents.h"

static const char * logTAG = "SNTP";

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- SNTP sychronization ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void sntpSyncNotification(struct timeval *tv)
{
  struct tm timeinfo;
  #if CONFIG_RLOG_PROJECT_LEVEL >= RLOG_LEVEL_INFO
  char strftime_buf[20];
  #endif // CONFIG_RLOG_PROJECT_LEVEL

  localtime_r(&tv->tv_sec, &timeinfo);
  if (timeinfo.tm_year < (1970 - 1900)) {
    sntp_set_sync_interval(CONFIG_SNTP_DELAY_FAILED);
    rlog_e(logTAG, "Time synchronization failed!");
  }
  else {
    // Set timezone
    #if defined(CONFIG_SNTP_TIMEZONE)
    setenv("TZ", CONFIG_SNTP_TIMEZONE, 1);
    tzset(); 
    #endif
    // Notification
    eventLoopPost(RE_TIME_EVENTS, RE_TIME_SNTP_SYNC_OK, nullptr, 0, portMAX_DELAY);
    // Set interval
    sntp_set_sync_interval(CONFIG_SNTP_DELAY_NORMAL);
    // Log
    #if CONFIG_RLOG_PROJECT_LEVEL >= RLOG_LEVEL_INFO
    strftime(strftime_buf, sizeof(strftime_buf), "%d.%m.%Y %H:%M:%S", &timeinfo);
    rlog_i(logTAG, "Time synchronization completed, current time: %s", strftime_buf);
    #endif // CONFIG_RLOG_PROJECT_LEVEL
  };
}

void sntpStopSNTP()
{
  if (sntp_enabled()) {
    sntp_stop();
    rlog_i(logTAG, "Time synchronization stopped");
  };
}

void sntpStartSNTP()
{
  // Stop time synchronization if it was started
  sntpStopSNTP();

  // Configuring synchronization parameters
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
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
  sntp_set_sync_interval(CONFIG_SNTP_DELAY_FAILED);

  sntp_init();

  if (sntp_enabled()) {
    rlog_i(logTAG, "Starting time synchronization with SNTP servers for a zone %s...", CONFIG_SNTP_TIMEZONE);
  } else {
    rlog_e(logTAG, "Failed to start time synchronization with SNTP servers!");
  };
}

bool sntpTaskCreate(bool createSuspended)
{
  if (createSuspended) {
    return sntpEventHandlerRegister();
  } else {
    sntpStartSNTP();
    return sntp_enabled();
  };
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------- WiFi event handler -------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

static void sntpWiFiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  // STA connected and Internet access is available
  if (event_id == RE_WIFI_STA_PING_OK) {
    if (!sntp_enabled()) {
      sntpStartSNTP();
    };
  }

  // STA disconnected or Internet access lost
  else if ((event_id == RE_WIFI_STA_PING_FAILED) || (event_id == RE_WIFI_STA_DISCONNECTED) || (event_id == RE_WIFI_STA_STOPPED)) {
    sntpStopSNTP();
  }
}

bool sntpEventHandlerRegister()
{
  return eventHandlerRegister(RE_WIFI_EVENTS, ESP_EVENT_ANY_ID, &sntpWiFiEventHandler, nullptr);
}


