MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_array, mp_module_array);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_binascii, mp_module_binascii);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_bluetooth, mp_module_bluetooth);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_collections, mp_module_collections);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_cryptolib, mp_module_cryptolib);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_errno, mp_module_errno);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_hashlib, mp_module_hashlib);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_heapq, mp_module_heapq);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_io, mp_module_io);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_json, mp_module_json);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_machine, mp_module_machine);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_os, mp_module_os);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_platform, mp_module_platform);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_random, mp_module_random);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_re, mp_module_re);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_select, mp_module_select);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_socket, mp_module_lwip);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_socket, mp_module_socket);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_ssl, mp_module_ssl);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_struct, mp_module_struct);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_time, mp_module_time);

MP_REGISTER_EXTENSIBLE_MODULE(MP_QSTR_websocket, mp_module_websocket);

MP_REGISTER_MODULE(MP_QSTR___main__, mp_module___main__);

MP_REGISTER_MODULE(MP_QSTR__asyncio, mp_module_asyncio);

MP_REGISTER_MODULE(MP_QSTR__espnow, mp_module_espnow);

MP_REGISTER_MODULE(MP_QSTR__onewire, mp_module_onewire);

MP_REGISTER_MODULE(MP_QSTR__rp2, mp_module_rp2);

MP_REGISTER_MODULE(MP_QSTR__thread, mp_module_thread);

MP_REGISTER_MODULE(MP_QSTR__webrepl, mp_module_webrepl);

MP_REGISTER_MODULE(MP_QSTR_ble, ble_module);

MP_REGISTER_MODULE(MP_QSTR_board, board_module);

MP_REGISTER_MODULE(MP_QSTR_btree, mp_module_btree);

MP_REGISTER_MODULE(MP_QSTR_builtins, mp_module_builtins);

MP_REGISTER_MODULE(MP_QSTR_cexample, example_user_cmodule);

MP_REGISTER_MODULE(MP_QSTR_cmath, mp_module_cmath);

MP_REGISTER_MODULE(MP_QSTR_cppexample, cppexample_user_cmodule);

MP_REGISTER_MODULE(MP_QSTR_deflate, mp_module_deflate);

MP_REGISTER_MODULE(MP_QSTR_esp, esp_module);

MP_REGISTER_MODULE(MP_QSTR_esp32, esp32_module);

MP_REGISTER_MODULE(MP_QSTR_example_package, example_package_user_cmodule);

MP_REGISTER_MODULE(MP_QSTR_ffi, mp_module_ffi);

MP_REGISTER_MODULE(MP_QSTR_framebuf, mp_module_framebuf);

MP_REGISTER_MODULE(MP_QSTR_gc, mp_module_gc);

MP_REGISTER_MODULE(MP_QSTR_jni, mp_module_jni);

MP_REGISTER_MODULE(MP_QSTR_lwip, mp_module_lwip);

MP_REGISTER_MODULE(MP_QSTR_math, mp_module_math);

MP_REGISTER_MODULE(MP_QSTR_microbit, microbit_module);

MP_REGISTER_MODULE(MP_QSTR_micropython, mp_module_micropython);

MP_REGISTER_MODULE(MP_QSTR_mimxrt, mp_module_mimxrt);

MP_REGISTER_MODULE(MP_QSTR_music, music_module);

MP_REGISTER_MODULE(MP_QSTR_network, mp_module_network);

MP_REGISTER_MODULE(MP_QSTR_nrf, nrf_module);

MP_REGISTER_MODULE(MP_QSTR_pyb, pyb_module);

MP_REGISTER_MODULE(MP_QSTR_samd, mp_module_samd);

MP_REGISTER_MODULE(MP_QSTR_stm, stm_module);

MP_REGISTER_MODULE(MP_QSTR_sys, mp_module_sys);

MP_REGISTER_MODULE(MP_QSTR_termios, mp_module_termios);

MP_REGISTER_MODULE(MP_QSTR_ubluepy, mp_module_ubluepy);

MP_REGISTER_MODULE(MP_QSTR_uctypes, mp_module_uctypes);

MP_REGISTER_MODULE(MP_QSTR_wipy, wipy_module);

MP_REGISTER_MODULE(MP_QSTR_zephyr, mp_module_zephyr);

MP_REGISTER_MODULE(MP_QSTR_zsensor, mp_module_zsensor);

MP_REGISTER_MODULE_DELEGATION(mp_module_sys, mp_module_sys_attr);
