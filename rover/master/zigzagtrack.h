#ifndef ZIGZAGTRACK_H
#define ZIGZAGTRACK_H

/* TODO: store this in FLASH memory, not SRAM */

struct checkpoint track[] = {
	/* Turn 45deg and go straight for 400m */
	{
		.angle = 20,
		.direction = 2,
		.distance = 250,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 45deg and go straight for 400m */
	{
		.angle = 40,
		.direction = 1,
		.distance = 250,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 45deg and go straight for 400m */
	{
		.angle = 40,
		.direction = 2,
		.distance = 250,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 45deg and go straight for 400m */
	{
		.angle = 40,
		.direction = 1,
		.distance = 250,
		.radius = 20,
		.sensor_flags = 0x00
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

#endif /* end of include guard: ZIGZAGTRACK_H */
