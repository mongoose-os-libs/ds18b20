#include "mgos.h"
#include "mg_rpc.h"
#include "mgos_onewire.h"
#include "mgos_rpc.h"

double mgos_ds18b20_reading;
uint8_t mgos_ds18b20_addr[8];
struct mgos_onewire *ow;
bool mgos_ds18b20_conn;

double mgos_ds18b20_get() {
  return mgos_ds18b20_reading;
}

double mgos_ds18b20_getF() {
  return ((mgos_ds18b20_reading * 1.8) + 32);
}

bool mgos_ds18b20_connected() {
  return mgos_ds18b20_conn;
}

static void mgos_ds18b20_read_cb(void *arg) {
  uint8_t scratchpad[9];
  int16_t rawvalue;
  double reading;
  char cfg;

  mgos_onewire_reset(ow);
  mgos_onewire_select(ow, mgos_ds18b20_addr);
  mgos_onewire_write(ow, 0xBE);
  mgos_onewire_read_bytes(ow, scratchpad, 9);

  rawvalue = (scratchpad[1] << 8) | scratchpad[0];
  cfg = (scratchpad[4] & 0x60);
  if (cfg == 0x00) {
    rawvalue = rawvalue & ~7;
  } else if (cfg == 0x20) {
    rawvalue = rawvalue & ~3;
  } else if (cfg == 0x40) {
    rawvalue = rawvalue & ~1;
  }

  reading = rawvalue * 0.0625;
  if (reading >= -25 && reading <= 125) {
    mgos_ds18b20_reading = reading;
  }
  mgos_ds18b20_conn = true;
  LOG(LL_DEBUG, ("OneWire: DS18B20 Reading %f", mgos_ds18b20_reading));
}

static void mgos_ds18b20_timer_cb(void *arg) {
  mgos_onewire_reset(ow);
  mgos_onewire_target_setup(ow, 0x28);
  if (mgos_onewire_next(ow, mgos_ds18b20_addr, 0)) {
    if (mgos_onewire_crc8(mgos_ds18b20_addr, 7) == mgos_ds18b20_addr[7]) {
      LOG(LL_DEBUG,
          ("OneWire: DS18B20 Found ADDRESS: %x%x%x%x%x%x%x%x",
           mgos_ds18b20_addr[0], mgos_ds18b20_addr[1], mgos_ds18b20_addr[2],
           mgos_ds18b20_addr[3], mgos_ds18b20_addr[4], mgos_ds18b20_addr[5],
           mgos_ds18b20_addr[6], mgos_ds18b20_addr[7]));
      mgos_onewire_reset(ow);
      mgos_onewire_select(ow, mgos_ds18b20_addr);
      mgos_onewire_write(ow, 0x44);
      mgos_set_timer(750, false, mgos_ds18b20_read_cb, NULL);
    } else {
      LOG(LL_ERROR, ("OneWire: CRC Mismatch"));
      mgos_ds18b20_conn = false;
    }
  } else {
    mgos_onewire_reset(ow);
    LOG(LL_DEBUG, ("OneWire: No DS18B20 Found"));
    mgos_ds18b20_conn = false;
  }
}

static void mgos_ds18b20_get_handler(struct mg_rpc_request_info *ri,
                                     void *cb_arg, struct mg_rpc_frame_info *fi,
                                     struct mg_str args) {
  mg_rpc_send_responsef(ri, "%f", mgos_ds18b20_get());
}

static void mgos_ds18b20_getF_handler(struct mg_rpc_request_info *ri,
                                      void *cb_arg,
                                      struct mg_rpc_frame_info *fi,
                                      struct mg_str args) {
  mg_rpc_send_responsef(ri, "%f", mgos_ds18b20_getF());
}

static void mgos_ds18b20_Poll_handler(struct mg_rpc_request_info *ri,
                                      void *cb_arg,
                                      struct mg_rpc_frame_info *fi,
                                      struct mg_str args) {
  mgos_ds18b20_timer_cb(NULL);
  mg_rpc_send_responsef(ri, NULL);
}

static void mgos_ds18b20_Connected_handler(struct mg_rpc_request_info *ri,
                                           void *cb_arg,
                                           struct mg_rpc_frame_info *fi,
                                           struct mg_str args) {
  mg_rpc_send_responsef(ri, "%B", mgos_ds18b20_conn);
}

bool mgos_ds18b20_init() {
  if (!mgos_sys_config_get_ds18b20_enable()) return true;
  mgos_ds18b20_conn = false;
  mgos_ds18b20_reading = 0;
  ow = mgos_onewire_create(mgos_sys_config_get_ds18b20_pin());
  if (mgos_sys_config_get_ds18b20_poll_period() != -1)
    mgos_set_timer(mgos_sys_config_get_ds18b20_poll_period(), true,
                   mgos_ds18b20_timer_cb, NULL);
  struct mg_rpc *c = mgos_rpc_get_global();
  if (mgos_sys_config_get_ds18b20_register_rpc()) {
    mg_rpc_add_handler(c, "Ds18b20.get", NULL, mgos_ds18b20_get_handler, NULL);
    mg_rpc_add_handler(c, "Ds18b20.getF", NULL, mgos_ds18b20_getF_handler,
                       NULL);
    mg_rpc_add_handler(c, "Ds18b20.Poll", NULL, mgos_ds18b20_Poll_handler,
                       NULL);
    mg_rpc_add_handler(c, "Ds18b20.Connected", NULL,
                       mgos_ds18b20_Connected_handler, NULL);
  }
  return true;
}
