#include "structs.h"

static void tire_pressure_status(void *data)
{
	tire_sensor *t = (tire_sensor *)data;
	if (t->pressure >= 21 && t->pressure <= 26) {
		printf("Tire has normal pressure.\n");
	} else if (t->pressure > 26 && t->pressure <= 28) {
		printf("Tire has high pressure.\n");
	} else if (t->pressure >= 19 && t->pressure < 21) {
		printf("Tire has low pressure.\n");
	} else {
		printf("Tire has abnormal pressure.\n");
	}
}

static void tire_temperature_status(void *data)
{
	tire_sensor *t = (tire_sensor *)data;
	if (t->temperature >= 0 && t->temperature <= 120) {
		printf("Tire has normal temperature.\n");
	} else {
		printf("Tire has abnormal temperature.\n");
	}
}

static void tire_wear_level_status(void *data)
{
	tire_sensor *t = (tire_sensor *)data;
	if (t->wear_level >= 0 && t->wear_level <= 20) {
		printf("Tire is new.\n");
	} else if (t->wear_level >= 20 && t->wear_level <= 40) {
		printf("Tire is slightly used.\n");
	} else if (t->wear_level >= 40 && t->wear_level <= 60) {
		printf("Tire is used. Consider a pit stop!\n");
	} else {
		printf("Tire is extremely used. Box this lap!\n");
	}
}

static void tire_performance_score(void *data)
{
	tire_sensor *t = (tire_sensor *)data;
	int score = 0;

	// Check pressure
	if (t->pressure >= 21 && t->pressure <= 26) {
		score += 4;
	} else if (t->pressure >= 19 && t->pressure < 21) {
		score += 2;
	} else if (t->pressure > 26 && t->pressure <= 28) {
		score += 3;
	} else {
		score -= 1;
	}

	// Check temperature
	if (t->temperature >= 80 && t->temperature <= 100) {
		score += 4;
	} else if (t->temperature >= 100 && t->temperature < 120) {
		score += 3;
	} else if (t->temperature > 60 && t->temperature <= 80) {
		score += 2;
	} else {
		score -= 1;
	}

	// Check wear level
	if (t->wear_level >= 0 && t->wear_level < 20) {
		score += 4;
	} else if (t->wear_level >= 20 && t->wear_level < 40) {
		score += 2;
	} else if (t->wear_level > 40 && t->wear_level < 60) {
		score -= 1;
	} else {
		score -= 2;
	}

	// Cap the score at 10
	if (score > 10) {
		score = 10;
	} else if (score < 1) {
		score = 1;
	}

	t->performace_score = score;
	printf("The tire performance score is: %d\n", score);
}

static void pmu_compute_power(void *data)
{
	power_management_unit *pmu = (power_management_unit *)data;
	float power = pmu->voltage * pmu->current;

	printf("Power output: %.2f kW.\n", power);
}

static void pmu_regenerate_energy(void *data)
{
	power_management_unit *pmu = (power_management_unit *)data;

	pmu->energy_storage += pmu->energy_regen;
	pmu->energy_regen = 0;

	// Cap the energy storage to 100%
	if (pmu->energy_storage > 100)
		pmu->energy_storage = 100;

	printf("Energy left in storage: %d\n", pmu->energy_storage);
}

static void pmu_get_energy_usage(void *data)
{
	power_management_unit *pmu = (power_management_unit *)data;
	float energy = pmu->power_consumption * pmu->current;

	printf("Energy usage: %.2f kW\n", energy);
}

static void pmu_is_battery_healthy(void *data)
{
	power_management_unit *pmu = (power_management_unit *)data;
	int ok = 0;

	if (pmu->voltage < 10 || pmu->voltage > 20) {
		printf("Battery voltage out of range: %.2fV\n", pmu->voltage);
		ok = 1;
	}

	if (pmu->current < -100 || pmu->current > 100) {
		printf("Battery current out of range: %.2fA\n", pmu->current);
		ok = 1;
	}

	if (pmu->power_consumption < 0 || pmu->power_consumption > 1000) {
		printf("Power consumption out of range: %.2f kW\n",
			   pmu->power_consumption);
		ok = 1;
	}

	if (pmu->energy_regen < 0 || pmu->energy_regen > 100) {
		printf("Energy regeneration out of range: %d%%\n", pmu->energy_regen);
		ok = 1;
	}

	if (pmu->energy_storage < 0 || pmu->energy_storage > 100) {
		printf("Energy storage out of range: %d%%\n", pmu->energy_storage);
		ok = 1;
	}

	if (ok == 1)
		return;

	printf("Battery working as expected!\n");
}

void get_operations(void **operations)
{
	operations[0] = tire_pressure_status;
	operations[1] = tire_temperature_status;
	operations[2] = tire_wear_level_status;
	operations[3] = tire_performance_score;
	operations[4] = pmu_compute_power;
	operations[5] = pmu_regenerate_energy;
	operations[6] = pmu_get_energy_usage;
	operations[7] = pmu_is_battery_healthy;
}

// returns an allocated power management unit
sensor *init_pmu(FILE *in)
{
	sensor *s = malloc(sizeof(*s));
	if (!s) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}
	s->sensor_type = PMU;

	// method: allocate, read, then copy the pmu to sensor data field
	power_management_unit *pmu = malloc(sizeof(*pmu));
	if (!pmu) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}

	fread(&pmu->voltage, sizeof(float), 1, in);
	fread(&pmu->current, sizeof(float), 1, in);
	fread(&pmu->power_consumption, sizeof(float), 1, in);
	fread(&pmu->energy_regen, sizeof(int), 1, in);
	fread(&pmu->energy_storage, sizeof(int), 1, in);

	s->sensor_data = malloc(sizeof(*pmu));
	if (!s->sensor_data) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}
	memcpy(s->sensor_data, (const void *)pmu, sizeof(*pmu));

	free(pmu);

	fread(&s->nr_operations, sizeof(int), 1, in);

	s->operations_idxs = calloc(s->nr_operations + 1, sizeof(int));
	if (!s->operations_idxs) {
		fprintf(stderr, "Calloc failed!\n");
		exit(1);
	}

	for (int i = 0; i < s->nr_operations; i++)
		fread(&s->operations_idxs[i], sizeof(int), 1, in);

	return s;
}

// returns an allocated tire
sensor *init_tire(FILE *in)
{
	sensor *s = malloc(sizeof(*s));
	if (!s) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}

	s->sensor_type = TIRE;

	// method: allocate, read, then copy the tire to sensor data field
	tire_sensor *tire = malloc(sizeof(*tire));
	if (!tire) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}

	fread(&tire->pressure, sizeof(float), 1, in);
	fread(&tire->temperature, sizeof(float), 1, in);
	fread(&tire->wear_level, sizeof(int), 1, in);
	fread(&tire->performace_score, sizeof(int), 1, in);

	s->sensor_data = malloc(sizeof(*tire));
	if (!s->sensor_data) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}
	memcpy(s->sensor_data, (const void *)tire, sizeof(*tire));

	free(tire);

	fread(&s->nr_operations, sizeof(int), 1, in);

	s->operations_idxs = calloc(s->nr_operations + 1, sizeof(int));
	if (!s->operations_idxs) {
		fprintf(stderr, "Calloc failed!\n");
		exit(1);
	}

	for (int i = 0; i < s->nr_operations; i++)
		fread(&s->operations_idxs[i], sizeof(int), 1, in);
	return s;
}

// frees an array of sensors from the heap, with a given number of sensors
void free_sensors(sensor *sensors, int nr_sensors)
{
	for (int i = 0; i < nr_sensors; i++) {
		if (sensors[i].sensor_type == TIRE) {
			if (sensors[i].sensor_data) {
				tire_sensor *tire = (tire_sensor *)sensors[i].sensor_data;
				free(tire);
			}
			if (sensors[i].operations_idxs)
				free(sensors[i].operations_idxs);
		} else {
			if (sensors[i].sensor_data) {
				power_management_unit *pmu = (power_management_unit *)
											sensors[i].sensor_data;
				free(pmu);
			}
			if (sensors[i].operations_idxs)
				free(sensors[i].operations_idxs);
		}
	}
	if (sensors)
		free(sensors);
}

// reads the data from the binary file
void read_sensors(const char *fname, sensor **sensors, int *nr_sensors)
{
	// fname is argv[1]; argv[0] is the name if the executable
	FILE *in = fopen(fname, "rb");
	if (!in) {
		fprintf(stderr, "Could not open the file!\n");
		exit(1);
	}

	fread(&(*nr_sensors), sizeof(int), 1, in);

	sensor *pmu = calloc(*nr_sensors, sizeof(sensor));
	if (!pmu) {
		fprintf(stderr, "Calloc failed!\n");
		exit(1);
	}

	sensor *tire = calloc(*nr_sensors, sizeof(sensor));
	if (!tire) {
		fprintf(stderr, "Calloc failed!\n");
		exit(1);
	}

	int p = 0, t = 0, stype = -1;
	for (int i = 0; i < *nr_sensors; i++) {
		fread(&stype, sizeof(int), 1, in);
		if (stype == TIRE)
			tire[t++] = *init_tire(in);
		else
			pmu[p++] = *init_pmu(in);
	}

	fclose(in);

	// method: sensors = (pmu | tire) to solve the priority rule
	*sensors = calloc(*nr_sensors + 1, sizeof(sensor));
	if (!(*sensors)) {
		fprintf(stderr, "Calloc failed!\n");
		exit(1);
	}

	int idx = 0, i = 0;
	while (i < p)
		(*sensors)[idx++] = pmu[i++];
	i = 0;
	while (i < t)
		(*sensors)[idx++] = tire[i++];
}

// prints the sensor data depending on sensor type
void print_sensors(int index, sensor *sensors)
{
	if (sensors[index].sensor_type == PMU) {
		printf("Power Management Unit\n");
		power_management_unit *pmu = (power_management_unit *)
									sensors[index].sensor_data;
		printf("Voltage: %.2f\n", pmu->voltage);
		printf("Current: %.2f\n", pmu->current);
		printf("Power Consumption: %.2f\n", pmu->power_consumption);
		printf("Energy Regen: %d%%\n", pmu->energy_regen);
		printf("Energy Storage: %d%%\n", pmu->energy_storage);
	} else {
		printf("Tire Sensor\n");
		tire_sensor *tire = (tire_sensor *)sensors[index].sensor_data;
		printf("Pressure: %.2f\n", tire->pressure);
		printf("Temperature: %.2f\n", tire->temperature);
		printf("Wear Level: %d%%\n", tire->wear_level);
		// condition to check if performance score was calculated before
		if (tire->performace_score < 1 || tire->performace_score > 10)
			printf("Performance Score: Not Calculated\n");
		else
			printf("Performance Score: %d\n", tire->performace_score);
	}
}

void analyze_sensors(int index, sensor **sensors, void **operations)
{
	for (int i = 0; i < (*sensors)[index].nr_operations; i++) {
		// cast operations[i] to pointer to function
		// type (void (*)(void *))
		switch ((*sensors)[index].operations_idxs[i]) {
		case 0:
			((void (*)(void *))operations[0])((*sensors)[index].sensor_data);
			break;
		case 1:
			((void (*)(void *))operations[1])((*sensors)[index].sensor_data);
			break;
		case 2:
			((void (*)(void *))operations[2])((*sensors)[index].sensor_data);
			break;
		case 3:
			((void (*)(void *))operations[3])((*sensors)[index].sensor_data);
			break;
		case 4:
			((void (*)(void *))operations[4])((*sensors)[index].sensor_data);
			break;
		case 5:
			((void (*)(void *))operations[5])((*sensors)[index].sensor_data);
			break;
		case 6:
			((void (*)(void *))operations[6])((*sensors)[index].sensor_data);
			break;
		case 7:
			((void (*)(void *))operations[7])((*sensors)[index].sensor_data);
			break;
		}
	}
}

/* using classical array element deleting, the function
removes all sensors with abnormal values */
void clear_sensors(sensor **sensors, int *nr_sensors)
{
	for (int i = 0; i < *nr_sensors; i++) {
		bool valid = true;
		if ((*sensors)[i].sensor_type == TIRE) {
			tire_sensor *tire = (tire_sensor *)((*sensors)[i].sensor_data);
			if (tire->pressure < 19 || tire->pressure > 28)
				valid = false;
			if (tire->temperature < 0 || tire->temperature > 120)
				valid = false;
			if (tire->wear_level < 0 || tire->wear_level > 100)
				valid = false;
		} else {
			power_management_unit *pmu = (power_management_unit *)
										((*sensors)[i].sensor_data);
			if (pmu->voltage < 10 || pmu->voltage > 20)
				valid = false;
			if (pmu->current < -100 || pmu->current > 100)
				valid = false;
			if (pmu->power_consumption < 0 || pmu->power_consumption > 1000)
				valid = false;
			if (pmu->energy_regen < 0 || pmu->energy_regen > 100)
				valid = false;
			if (pmu->energy_storage < 0 || pmu->energy_storage > 100)
				valid = false;
		}
		if (!valid) {
			for (int j = i; j < *nr_sensors - 1; j++) {
				(*sensors)[j] = (*sensors)[j + 1];
			}
			(*nr_sensors)--;
			/* important in the case where there are two consecutive
			abnormal sensors */
			i--;
		}
	}
	sensor *p = (sensor *)realloc(*sensors, (*nr_sensors) * sizeof(sensor));
	if (!p) {
		fprintf(stderr, "Realloc failed!\n");
		exit(1);
	}
	*sensors = p;
}

// deallocates the memory
void exit_program(sensor *sensors, void **operations, int nr_sensors)
{
	free(operations);
	free_sensors(sensors, nr_sensors);
}
