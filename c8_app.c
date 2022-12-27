#include "c8_app.h"

bool c8_app_update()
{
	C8_Rgb rgb = { 255, 0, 0 };

	return c8_plat_push_rect(0.0, 10.0, 200.0, 300.0, rgb);

}