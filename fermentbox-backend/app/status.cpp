#include "status.h"

static FermentboxStatus status;

static const char messages[][80] = {"Normal",
                                    "Temperature/Humidity Sensor failed"};

FermentboxStatus get_status() { return status; }

const char *get_status_message() { return messages[status]; }

void set_status(FermentboxStatus stat) { status = stat; }