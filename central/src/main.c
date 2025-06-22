/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#define NAME_LEN 30


struct adv_packet_data {
	char name[NAME_LEN];
	uint32_t packet_id;
};

static bool data_cb(struct bt_data *data, void *user_data)
{
	struct adv_packet_data *packet_data = user_data;

	switch (data->type) {
	case BT_DATA_NAME_SHORTENED:
	case BT_DATA_NAME_COMPLETE:
		memcpy(packet_data->name, data->data, MIN(data->data_len, NAME_LEN - 1));
		return true;
	case BT_DATA_MANUFACTURER_DATA:
		// Expected to be a 32-bit integer
		if (data->data_len != sizeof(uint32_t)) {
			return false;
		}
		// TODO: ensure endianness is correct
		packet_data->packet_id = *(uint32_t*)data->data;
		return false;
	default:
		return true;
	}
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	struct adv_packet_data packet_data = {0};

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));


	bt_data_parse(ad, data_cb, &packet_data);

	// print only devices with name "Test beacon"

	if (strcmp(packet_data.name, "Test beacon") == 0) {
		printk("Device found: %s : %s (RSSI %d)\n", packet_data.name, addr_str, rssi);
		printk("Packet id: %u\n", packet_data.packet_id);
		return;
	}
}

static void start_scan(void)
{
	int err;

	/* This demo doesn't require active scan */
	err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

int main(void)
{
	int err;

	printk("Starting Central Demo\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	return 0;
}
