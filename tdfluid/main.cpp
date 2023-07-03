#include <glad/glad.h>

#include "tdwindow.h"
#include "tdobject.h"

#include "tdui.h"

void simulation()
{
	td::window window;

	// create fluid field
	td::fluid_object object;

	// create static boundaries
	td::fluid_closed_border *border = new td::fluid_closed_border(object.get_width(), object.get_height());
	object.add_object(border);

	td::fluid_ball *ball = new td::fluid_ball(200, 200, 100);
	object.add_object(ball);

	// add fluid field to window
	window.include(&object);

	// and add ui
	td::ui ui;
	window.include(&ui);

	window.start();
	window.main();

	// cleanup
	delete border;
	delete ball;
	window.end();
}


int main(int argc, char *argv)
{
	simulation();
	return 0;
}