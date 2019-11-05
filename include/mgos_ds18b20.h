#include "mgos_onewire_rmt.h"

#ifndef _MGOS_DS18B20_H
#define _MGOS_DS18B20_H

double mgos_ds18b20_reading;
uint8_t mgos_ds18b20_addr[8];
bool mgos_ds18b20_conn;
OnewireRmt* ow;

#ifdef __cplusplus
extern "C"{
#endif

    bool mgos_ds18b20_init();
    double mgos_ds18b20_get();
	double mgos_ds18b20_getF();

#ifdef __cplusplus
}
#endif

#endif
