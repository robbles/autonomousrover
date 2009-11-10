#ifndef SQUARETRACK_H
#define SQUARETRACK_H

/* TODO: store this in FLASH memory, not SRAM */

struct checkpoint track[] = {
	/* First straight stretch */
	{
		.angle = 0,
		.direction = 0,
		.distance = 400,
		.radius = 0,
		.sensor_flags = 0
	},
	{
		.angle = 90,
		.direction = 2,
		.distance = 400,
		.radius = 0,
		.sensor_flags = 0
	},
	{
		.angle = 90,
		.direction = 2,
		.distance = 400,
		.radius = 0,
		.sensor_flags = 0
	},
	{
		.angle = 90,
		.direction = 2,
		.distance = 400,
		.radius = 0,
		.sensor_flags = 0
	},
	/* End of track */
	{
		.angle = 0,
		.direction = 0,
		.distance = 0,
		.radius = 0,
		.sensor_flags = 0
	}
};

#endif /* end of include guard: SQUARETRACK_H */
