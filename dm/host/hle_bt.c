// Pokemon Violet: the game's own Bluetooth manager (main+0x930588) calls nn::bluetooth's BLE entry points, which
// connect to the bt / btm services. Offline answers as in hle_offline.c: initialization fails, no devices, events
// never fire. Constructors / getters that only touch the game's buffers keep running (not hooked).
#include "hle.h"

HLE_FORCE("_ZN2nn9bluetooth13InitializeBleEv", "BLE init (cut): fails, the game takes its offline branch", Result, h_bt_InitBle, void) { return RES_FAIL; }
HLE_FORCE("_ZN2nn9bluetooth23StartBleScanSmartDeviceERKNS0_17GattAttributeUuidE", "BLE scan (cut): fails", Result, h_bt_Scan, const void *u) { (void)u; return RES_FAIL; }
HLE_FORCE("_ZN2nn9bluetooth22StopBleScanSmartDeviceEv", "BLE scan stop (cut)", Result, h_bt_ScanStop, void) { return RES_FAIL; }
HLE_FORCE("_ZN2nn9bluetooth16GetBleScanResultEPNS0_13BleScanResultEi", "BLE scan results (cut): none", int, h_bt_ScanRes, void *r, int max) { (void)r; (void)max; return 0; }
HLE_FORCE("_ZN2nn9bluetooth24GetBleConnectionInfoListEPNS0_17BleConnectionInfoEi", "BLE connections (cut): none", int, h_bt_Conns, void *r, int max) { (void)r; (void)max; return 0; }
HLE_FORCE("_ZN2nn9bluetooth19ConnectToGattServerERKNS0_7AddressE", "BLE connect (cut): fails", Result, h_bt_Connect, const void *a) { (void)a; return RES_FAIL; }
HLE_FORCE("_ZN2nn9bluetooth15ConfigureBleMtuEjt", "BLE MTU (cut): fails", Result, h_bt_Mtu, unsigned a, unsigned short m) { (void)a; (void)m; return RES_FAIL; }
HLE_FORCE("_ZN2nn9bluetooth19AcquireBleScanEventEPNS_2os15SystemEventTypeE", "BLE scan event: bound, never fires", Result, h_bt_EvScan, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn9bluetooth19AcquireBleConnectionStateChangedEventEPNS_2os15SystemEventTypeE", "BLE connection event: bound, never fires", Result, h_bt_EvConn, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn9bluetooth31AcquireBleServiceDiscoveryEventEPNS_2os15SystemEventTypeE", "BLE discovery event: bound, never fires", Result, h_bt_EvDisc, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn9bluetooth24AcquireBleMtuConfigEventEPNS_2os15SystemEventTypeE", "BLE MTU event: bound, never fires", Result, h_bt_EvMtu, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn9bluetooth28AcquireBleGattOperationEventEPNS_2os15SystemEventTypeE", "BLE operation event: bound, never fires", Result, h_bt_EvOp, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }

// nn::btm::user (Bluetooth manager service) - the game's peripheral manager calls these after the nn::bluetooth layer
HLE_FORCE("_ZN2nn3btm4user25AcquireBleConnectionEventEPNS_2os15SystemEventTypeE", "btm BLE connection event: bound, never fires", Result, h_btm_EvConn, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn3btm4user22AcquireBlePairingEventEPNS_2os15SystemEventTypeE", "btm BLE pairing event: bound, never fires", Result, h_btm_EvPair, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn3btm4user19AcquireBleScanEventEPNS_2os15SystemEventTypeE", "btm BLE scan event: bound, never fires", Result, h_btm_EvScan, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn3btm4user24AcquireBleMtuConfigEventEPNS_2os15SystemEventTypeE", "btm BLE MTU event: bound, never fires", Result, h_btm_EvMtu, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn3btm4user31AcquireBleServiceDiscoveryEventEPNS_2os15SystemEventTypeE", "btm BLE discovery event: bound, never fires", Result, h_btm_EvDisc, void *ev) { hle_bind_system_event(ev, 0); return RES_OK; }
HLE_FORCE("_ZN2nn3btm4user23StartBleScanForSmartDeviceERKNS_9bluetooth17GattAttributeUuidE", "btm BLE scan (cut): fails", Result, h_btm_Scan, const void *u) { (void)u; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user21StopBleScanForGeneralEv", "btm BLE scan stop (cut)", Result, h_btm_ScanStopG, void) { return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user20StopBleScanForPairedEv", "btm BLE scan stop (cut)", Result, h_btm_ScanStopP, void) { return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user15ConfigureBleMtuEjt", "btm BLE MTU (cut): fails", Result, h_btm_Mtu, unsigned a, unsigned short m) { (void)a; (void)m; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user23RegisterBleGattDataPathERKNS1_11BleDataPathE", "btm BLE data path (cut): fails", Result, h_btm_RegPath, const void *p) { (void)p; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user25UnregisterBleGattDataPathERKNS1_11BleDataPathE", "btm BLE data path (cut)", Result, h_btm_UnregPath, const void *p) { (void)p; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user13BlePairDeviceEjRKNS_9bluetooth27BleAdvertisePacketParameterE", "btm BLE pairing (cut): fails", Result, h_btm_Pair, unsigned h, const void *p) { (void)h; (void)p; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user15BleUnPairDeviceEjRKNS_9bluetooth27BleAdvertisePacketParameterE", "btm BLE unpairing (cut): fails", Result, h_btm_Unpair, unsigned h, const void *p) { (void)h; (void)p; return RES_FAIL; }
HLE_FORCE("_ZN2nn3btm4user23GetBelongingGattServiceEPNS1_11GattServiceEjt", "btm BLE GATT service (cut): fails", Result, h_btm_GetSvc, void *s, unsigned h, unsigned t) { (void)s; (void)h; (void)t; return RES_FAIL; }
