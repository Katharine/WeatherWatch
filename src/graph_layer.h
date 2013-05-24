#ifndef GRAPH_LAYER_H
#define GRAPH_LAYER_H

typedef struct {
	Layer layer;
	uint8_t max_value;
	uint8_t point_width;
	uint8_t* data;
	uint8_t data_count;
	GColor axis_colour;
	GColor graph_colour;
	GColor tick_colour;
	GColor background_colour;
	uint8_t* v_ticks;
	uint8_t v_tick_count;
	uint8_t* h_ticks;
	uint8_t h_tick_count;
} GraphLayer;

void graph_layer_init(GraphLayer* graph_layer, GRect frame);
void graph_layer_set_max_value(GraphLayer* graph_layer, uint8_t max_value);
void graph_layer_set_point_width(GraphLayer* graph_layer, uint8_t width);
void graph_layer_set_axis_colour(GraphLayer* graph_layer, GColor axis_colour);
void graph_layer_set_graph_colour(GraphLayer* graph_layer, GColor graph_colour);
void graph_layer_set_tick_colour(GraphLayer* graph_layer, GColor tick_colour);
void graph_layer_set_background_colour(GraphLayer* graph_layer, GColor background_colour);
void graph_layer_set_data(GraphLayer* graph_layer, uint8_t* data, uint8_t length);
void graph_layer_set_vertical_ticks(GraphLayer* graph_layer, uint8_t* ticks, uint8_t length);
void graph_layer_set_horizontal_ticks(GraphLayer* graph_layer, uint8_t* ticks, uint8_t length);

#endif // GRAPH_LAYER_H
