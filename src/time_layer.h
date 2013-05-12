#ifndef TIME_LAYER_H
#define TIME_LAYER_H

typedef struct {
	Layer layer;
	BmpContainer digit_layers[4];
	BmpContainer colon_layer;
	int8_t layer_states[4];
	PblTm current_time;
} TimeLayer;

void time_layer_set_time(TimeLayer* time_layer, PblTm time);
void time_layer_init(TimeLayer* time_layer, GPoint top_left);
void time_layer_deinit(TimeLayer* time_layer);

#endif // TIME_LAYER_H
