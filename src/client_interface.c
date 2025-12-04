#include "raylib.h"

#include "client_jpeg.h"
#include "client_button.h"


void control_setup() {
	printf("CONTROL: SETUP\n");
}
void control_play() {
	printf("CONTROL: PLAY\n");
}
void control_pause() {
	printf("CONTROL: PAUSE\n");
}
void control_teardown() {
	printf("CONTROL: TEARDOWN\n");
}


int main() {
	/*
	 button sector
	*/
//	Rectangle button_setup, button_play, button_pause, button_teardown;
	struct button_archive archive = {
		.arr = NULL,
		.size = 0,
	};
//	void (*control_command[])() = {
//		control_setup,
//		control_play,
//		control_pause,
//		control_teardown,
//	};
	struct control_info controls[] = {
		{.command = control_setup, .name = "Setup"},
		{.command = control_play, .name = "Play"},
		{.command = control_pause, .name = "Pause"},
		{.command = control_teardown, .name = "Teardown"},
	};
	size_t num_commands = sizeof(controls) / sizeof(controls[0]);
	assign_controls(&archive, controls, num_commands);


	const int init_width = 800;
        const int init_height = 600;

	SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(init_width, init_height, "Client Interface");
	SetWindowMinSize(init_width, init_height);

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(RAYWHITE);

			rescale_buttons(&archive);
			draw_buttons(archive);
			update_buttons(archive);
		EndDrawing();
	}

	CloseWindow();


	return 0;
}
