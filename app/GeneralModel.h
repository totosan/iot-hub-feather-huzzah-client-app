#ifndef _GENERALMODEL_H
#define _GENERALMODEL_H

typedef struct GENERAL_STATE_TAG
{
    char version[10];
    int reported_interval;
    char update_state[URL_MAX_LENGTH];
} GeneralState;

typedef struct GENERAL_SETTINGS_TAG
{
    int desired_interval;
    char update_url[128];
} GeneralSettings;

typedef struct GENERAL_TAG
{
    GeneralSettings settings;
    GeneralState state;
} General;

#endif