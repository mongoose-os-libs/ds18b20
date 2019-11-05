let DS18B20 = {
    get: ffi("double mgos_ds18b20_get()"),
	getF: ffi("double mgos_ds18b20_getF()"),
    connected: ffi("int mgos_ds18b20_connected()")
}
