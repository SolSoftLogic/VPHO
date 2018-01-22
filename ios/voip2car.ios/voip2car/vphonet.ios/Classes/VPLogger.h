//
//  VphonetLogger.h
//  Vphonet


#define VP_LOG_ERROR 1
#define VP_LOG_WARN  2
#define VP_LOG_DEBUG 4

#define VP_LOG_SEVERETY (VP_LOG_ERROR | VP_LOG_WARN | VP_LOG_DEBUG)

#define VP_LOG_MSG(severety, format_str, ...) \
{\
    if (severety & VP_LOG_SEVERETY) \
		NSLog(@"%s %d : "format_str, __FILE__, __LINE__, ##__VA_ARGS__); \
}

