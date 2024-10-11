#include "structs.h"

int main(int argc, char const *argv[])
{
	int nr_sensors = 0;
	sensor *sensors;

	// retain the data from the binary file
	read_sensors(argv[1], &sensors, &nr_sensors);

	char *command = malloc(MAX_COMMAND);
	if (!command) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}

	// array of pointers to functions
	void **operations = malloc(8 * sizeof(void *));
	if (!operations) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}
	get_operations(operations);

	int index;
	// command session
	while (true) {
		scanf("%s", command);
		if (strcmp(command, "print") == 0) {
			scanf("%d", &index);
			if (index < 0 || index >= nr_sensors)
				printf("Index not in range!\n");
			else
				print_sensors(index, sensors);
		} else if (strcmp(command, "analyze") == 0) {
			scanf("%d", &index);
			if (index < 0 || index >= nr_sensors)
				printf("Index not in range!\n");
			else
				analyze_sensors(index, &sensors, operations);
		} else if (strcmp(command, "clear") == 0) {
			clear_sensors(&sensors, &nr_sensors);
		} else if (strcmp(command, "exit") == 0) {
			// frees sensors and operations arrays
			exit_program(sensors, operations, nr_sensors);
			break;
		}
	}

	free(command);
	return 0;
}
