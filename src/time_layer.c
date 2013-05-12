#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "time_layer.h"

static uint8_t DIGIT_RESOURCES[] = {
	RESOURCE_ID_TIME_DIGIT_ZERO,
	RESOURCE_ID_TIME_DIGIT_ONE,
	RESOURCE_ID_TIME_DIGIT_TWO,
	RESOURCE_ID_TIME_DIGIT_THREE,
	RESOURCE_ID_TIME_DIGIT_FOUR,
	RESOURCE_ID_TIME_DIGIT_FIVE,
	RESOURCE_ID_TIME_DIGIT_SIX,
	RESOURCE_ID_TIME_DIGIT_SEVEN,
	RESOURCE_ID_TIME_DIGIT_EIGHT,
	RESOURCE_ID_TIME_DIGIT_NINE,
};

// Unhelpfully, our images are not all the same size.
static GSize DIGIT_SIZES[10] = {
	{35, 78},
	{17, 76},
	{35, 75},
	{34, 76},
	{35, 75},
	{33, 76},
	{32, 76},
	{34, 75},
	{33, 76},
	{33, 77}
};
// This is easier than trying to calculate them on the fly.
static uint8_t SLOT_POSITIONS[] = {0, 19, 71, 108};

static void time_layer_set_slot_digit(TimeLayer* time_layer, uint8_t slot, int8_t digit);
static void time_layer_draw_backing(Layer* layer, GContext *ctx);

void time_layer_init(TimeLayer* time_layer, GPoint top_left) {
	// Root layer with black background.
	GRect frame = GRect(top_left.x, top_left.y, 144, 100);
	layer_init(&time_layer->layer, frame);
	time_layer->layer.update_proc = time_layer_draw_backing;
	
	// Set up our colon
	bmp_init_container(RESOURCE_ID_TIME_COLON, &time_layer->colon_layer);
	layer_set_frame(&time_layer->colon_layer.layer.layer, GRect(56, 37, 13, 52));
	layer_add_child(&time_layer->layer, &time_layer->colon_layer.layer.layer);
	
	for(int i = 0; i < 4; ++i)
		time_layer->layer_states[i] = -1;
}

void time_layer_set_time(TimeLayer* time_layer, PblTm time) {
	int hours = time.tm_hour % 12; // We can only do 24-hour due to space constraints.
	if(hours == 0) hours = 12;
	time_layer_set_slot_digit(time_layer, 0, hours / 10);
	time_layer_set_slot_digit(time_layer, 1, hours % 10);
	time_layer_set_slot_digit(time_layer, 2, time.tm_min / 10);
	time_layer_set_slot_digit(time_layer, 3, time.tm_min % 10);
}

void time_layer_deinit(TimeLayer* time_layer) {
	for(int i = 0; i < 4; ++i) {
		if(time_layer->layer_states[i] != -1) {
			bmp_deinit_container(&time_layer->digit_layers[i]);
		}
	}
	bmp_deinit_container(&time_layer->colon_layer);
}

static void time_layer_draw_backing(Layer* layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer->bounds, 0, GCornerNone);
}

static void time_layer_set_slot_digit(TimeLayer* time_layer, uint8_t slot, int8_t digit) {
	// No need to update it if we already have the right digit.
	if(time_layer->layer_states[slot] == digit) return;
	if(time_layer->layer_states[slot] != -1) {
		layer_remove_from_parent(&time_layer->digit_layers[slot].layer.layer);
		bmp_deinit_container(&time_layer->digit_layers[slot]);
		time_layer->layer_states[slot] = -1;
	}
	// Out of bounds digits are bad.
	if(digit < 0 || digit > 9) return;
	// We don't render the first digit if it's zero.
	if(!(slot == 0 && digit == 0)) {
		bmp_init_container(DIGIT_RESOURCES[digit], &time_layer->digit_layers[slot]);
		uint8_t height = DIGIT_SIZES[digit].h;
		uint8_t width = DIGIT_SIZES[digit].w;
		int h_offset = 0;
		if(slot != 0) {
			h_offset = (35 - width) / 2;
		}
		layer_set_frame(&time_layer->digit_layers[slot].layer.layer, GRect(SLOT_POSITIONS[slot] + h_offset, 10 + 79 - height, width, height));
		layer_add_child(&time_layer->layer, &time_layer->digit_layers[slot].layer.layer);
		time_layer->layer_states[slot] = digit;
	}
}
