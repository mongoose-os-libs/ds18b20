#ifndef _MGOS_DS18B20_H
#define _MGOS_DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

bool mgos_ds18b20_init();
double mgos_ds18b20_get();
double mgos_ds18b20_getF();

#ifdef __cplusplus
}
#endif

#endif
