#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#pragma once
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_COMMAND 30

enum sensor_type {
	TIRE,
	PMU
};

typedef struct {
	enum sensor_type sensor_type;
	void *sensor_data;
	int nr_operations;
	int *operations_idxs;
} sensor;

typedef struct __attribute__((__packed__)) {
	float voltage;
	float current;
	float power_consumption;
	int energy_regen;
	int energy_storage;
} power_management_unit;

typedef struct __attribute__((__packed__)) {
	float pressure;
	float temperature;
	int wear_level;
	int performace_score;
} tire_sensor;

void get_operations(void **operations);

// functions used in main & operations
void read_sensors(const char *fname, sensor **sensors, int *nr_sensors);
void print_sensors(int index, sensor *sensors);
void analyze_sensors(int index, sensor **sensors, void **operations);
void clear_sensors(sensor **sensors, int *nr_sensors);
void exit_program(sensor *sensors, void **operations, int nr_sensors);

#endif // _STRUCTS_H_