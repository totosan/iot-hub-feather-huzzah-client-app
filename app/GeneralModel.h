#ifndef _GENERALMODEL_H
#define _GENERALMODEL_H

typedef struct GENERAL_STATE_TAG
{
    const char *version;
    int reported_interval;
} GeneralState;

typedef struct GENERAL_SETTINGS_TAG
{
    int desired_interval;
    const char *update_url;
} GeneralSettings;

typedef struct GENERAL_TAG
{
    GeneralSettings settings;
    GeneralState state;
} General;
#endif