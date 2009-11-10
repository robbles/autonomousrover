#ifndef SPINTRACK_H
#define SPINTRACK_H

/* TODO: store this in FLASH memory, not SRAM */

struct checkpoint track[] = {
	/* Turn for 360 degrees */
	{
		.angle = 563,
		.direction = 2,
		.distance = 0,
		.radius = 0,
		.sensor_flags = 0
	},
	{
		.angle = 0,
		.direction = 0,
		.distance = 0,
		.radius = 0,
		.sensor_flags = 0
	}
};

#endif /* end of include guard: SPINTRACK_H */
