#include <glad/glad.h>

#include "tdwindow.h"
#include "tdobject.h"

#include "tdui.h"

void simulation()
{
	td::window window;

	td::fluid_object object;

	td::fluid_closed_border *border = new td::fluid_closed_border(object.get_width(), object.get_height());
	object.add_object(border);

	td::fluid_ball *ball = new td::fluid_ball(200, 200, 100);
	object.add_object(ball);

	window.include(&object);
	

	td::ui ui;
	window.include(&ui);

	window.start();

	window.main();

	delete border;
	delete ball;
	window.end();
}


int main(int argc, char *argv)
{
	simulation();
	return 0;
}