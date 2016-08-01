
#include <stdio.h>

#include <artik_module.h>
#include <artik_platform.h>
#include <artik_i2c.h>

/*
 * This test only works if the CW2015 Linux driver is unbound first:
 * artik520 : $ echo 1-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * artik1020: $ echo 0-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * artik710 : $ echo 8-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 */

static artik_i2c_config config = {
	1,
	2000,
	I2C_8BIT,
	0x62
};

#define CW201x_REG_VERSION	0x0
#define CW201x_REG_CONFIG	0x8

static artik_error i2c_test_cw2015(int platid)
{
	artik_i2c_module *i2c = (artik_i2c_module *)artik_request_api_module("i2c");
	artik_i2c_handle cw2015;
	char version, conf;
	artik_error ret;

	if (platid == ARTIK520)
		config.id = 1;
	else if (platid == ARTIK1020)
		config.id = 0;
	else
		config.id = 8;

	fprintf(stdout, "TEST: %s starting\n", __func__);
	ret = i2c->request(&cw2015, &config);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to request I2C %d@0x%02x (%d)\n",
			config.id, config.address, ret);
		goto exit;
	}
	fprintf(stdout, "Reading version register...");
	ret = i2c->read_register(cw2015, CW201x_REG_VERSION, &version, 1);
	if (ret != S_OK) {
		fprintf(stderr,
			"FAILED\nFailed to read I2C %d@0x%02x register 0x%04x (%d)\n",
			config.id, config.address, CW201x_REG_VERSION, ret);
		goto exit;
	}
	fprintf(stdout, "OK - val=0x%02x\n", version);
	if (version != 0x6f) {
		fprintf(stderr,
			"%s: Wrong chip version read, expected 0x6f, got 0x%02x\n",
			__func__, version);
		ret = E_BAD_ARGS;
	} else
		fprintf(stdout, "CW2015 version: 0x%02x\n", version);
	fprintf(stdout, "Reading configuration register...");
	ret = i2c->read_register(cw2015, CW201x_REG_CONFIG, &conf, 1);
	if (ret != S_OK) {
		fprintf(stderr,
			"FAILED\nFailed to read I2C %d@0x%02x register 0x%04x (%d)\n",
			config.id, config.address, CW201x_REG_CONFIG, ret);
		goto exit;
	}
	fprintf(stdout, "OK - val=0x%02x\n", conf);
	fprintf(stdout, "Writing configuration register...");
	conf = 0xff;
	ret = i2c->write_register(cw2015, CW201x_REG_CONFIG, &conf, 1);
	if (ret != S_OK) {
		fprintf(stderr,
			"FAILED\nFailed to write I2C %d@0x%02x register 0x%04x (%d)\n",
			config.id, config.address, CW201x_REG_CONFIG, ret);
		goto exit;
	}
	fprintf(stdout, "OK\n");
	fprintf(stdout, "Reading configuration register...");
	ret = i2c->read_register(cw2015, CW201x_REG_CONFIG, &conf, 1);
	if (ret != S_OK) {
		fprintf(stderr,
			"FAILED\nFailed to read I2C %d@0x%02x register 0x%04x (%d)\n",
			config.id, config.address, CW201x_REG_CONFIG, ret);
		goto exit;
	}
	fprintf(stdout, "OK - val=0x%02x\n", conf);
	ret = i2c->release(cw2015);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to release I2C %d@0x%02x (%d)\n",
			config.id, config.address, ret);
		goto exit;
	}
exit:
	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" : "failed");

	artik_release_api_module(i2c);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;
	int platid = artik_get_platform();

	if ((platid == ARTIK520) || (platid == ARTIK1020) || (platid == ARTIK710))
		ret = i2c_test_cw2015(platid);

	return (ret == S_OK) ? 0 : -1;
}