#ifndef __CLIENT_BUTTON_H
#define __CLIENT_BUTTON_H


#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>


const int button_height = 40;


struct button {
	Rectangle shape;
	char* text;
	void (*func)();
};
struct button_archive {	
	size_t size;
	struct button* arr;
};

struct control_info {
	void (*command)();
	const char* name;
};


void assign_controls(struct button_archive* archive, const struct control_info* controls, const size_t num_commands) {
	if (NULL == archive->arr)
		archive->arr = malloc(num_commands * sizeof(struct button));
	else if (num_commands > archive->size) {
		if(NULL == realloc(archive->arr, num_commands * sizeof(struct button)))
			perror("assign_funcs()");
	}

	archive->size = num_commands;
	for (size_t i = 0; i < num_commands; i++) {
//		if (NULL != archive->arr[i].func)
//			free(archive->arr[i].func);

		archive->arr[i].func = controls[i].command;
		archive->arr[i].text = controls[i].name;
	}
}

void static inline rescale_buttons(struct button_archive* archive) {
	int width = GetScreenWidth(), height = GetScreenHeight();
	int n = archive->size;
	
	float button_width = width / n;
	float button_gap = (float)(width - button_width * n) / (n - 1);

	float start_height = height - button_height;

	for (size_t i = 0; i < archive->size; i++) {
		archive->arr[i].shape = (Rectangle) {
			.x = (button_width + button_gap) * i,
			.y = start_height,
			.width = button_width,
			.height = button_height
		};
	}
}

void static inline draw_buttons(const struct button_archive archive) {
	const int font_size = 20;

	for (size_t i = 0; i < archive.size; i++) {
		DrawRectangleLinesEx(archive.arr[i].shape, 1.0f, BLACK);

		int text_width = MeasureText(archive.arr[i].text, font_size);
		int text_x = archive.arr[i].shape.x + (archive.arr[i].shape.width - text_width)/2;
		int text_y = archive.arr[i].shape.y + (archive.arr[i].shape.height - font_size)/2;
		
		DrawText(archive.arr[i].text, text_x, text_y, font_size, BLACK);
	}
}

void static inline update_buttons(const struct button_archive archive) {
	for (size_t i = 0; i < archive.size; i++) {
		Vector2 mouse_pos = GetMousePosition();

		bool hovered = CheckCollisionPointRec(mouse_pos, archive.arr[i].shape);

		if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			if (archive.arr[i].func)
				archive.arr[i].func();
		}
	}
}

#endif
