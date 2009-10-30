#ifndef ZIGZAGTRACK_H
#define ZIGZAGTRACK_H

/* TODO: store this in FLASH memory, not SRAM */

struct checkpoint track[] = {
	/* First straight stretch */
	{
		.distance = 100,
		.angle = 90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 90deg and go straight for 100m */
	{
		.distance = 100,
		.angle = -90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 90deg and go straight for 100m */
	{
		.distance = 100,
		.angle = 90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* Turn 90deg and go straight for 100m */
	{
		.distance = 100,
		.angle = -90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* End of track */
	{
		.distance = 100,
		.angle = 90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* End of track */
	{
		.distance = 100,
		.angle = -90,
		.radius = 20,
		.sensor_flags = 0x00
	},
	/* End of track */
	{
		.distance = 0,
		.angle = 0,
		.radius = 0,
		.sensor_flags = 0
	}
};

#endif /* end of include guard: ZIGZAGTRACK_H */
