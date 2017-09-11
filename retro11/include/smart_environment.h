#ifndef SMART_ENVIRONMENT_H_
#define SMART_ENVIRONMENT_H_

#define SERVER_CONN_PORT  2017
#define PING_TIMEOUT    60

// statische globals, nach außen hin nicht sichtbar
static const char app_id[] = "riot-swp-2017-se";
#define APP_ID_LEN      sizeof(app_id)

#endif
