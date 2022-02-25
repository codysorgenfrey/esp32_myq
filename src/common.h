#ifndef __MYQCOMMON_H__
#define __MYQCOMMON_H__

#ifndef MYQ_DEBUG
#define MYQ_DEBUG true
#endif

// API constants
#define MYQ_API ""

#define MYQ_USER_DATA_FILE "/MYQ_USER_DATA.json"

#define MYQ_TIME_GMT_OFFSET -8 * 3600 // - 8 hours PST
#define MYQ_DST_OFFSET 1 * 3600
#define MYQ_NTP_SERVER "pool.ntp.org"

#define MYQ_REFRESH_BUFFER 300000 // 5 minutes

// Logging
#if MYQ_DEBUG
#define MYQ_LOG(message, ...) printf(">>> [%7d][%.2fkb] MyQ: " message , millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#define MYQ_LOG_LINE(message, ...) printf(">>> [%7d][%.2fkb] MyQ: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
#define MYQ_LOG(message, ...)
#define MYQ_LOG_LINE(message, ...)
#endif

#define MYQ_OAUTH_CA_CERT \
""

#define MYQ_API_CERT \
""

#endif