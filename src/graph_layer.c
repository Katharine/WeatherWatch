#include "pebble_os.h"
#include "graph_layer.h"

static void graph_layer_draw(Layer* layer, GContext* ctx);

void graph_layer_init(GraphLayer* graph_layer, GRect frame) {
	layer_init(&graph_layer->layer, frame);
	graph_layer->layer.update_proc = graph_layer_draw;
	graph_layer->data = NULL;
	graph_layer->data_count = 0;
	graph_layer->v_ticks = NULL;
	graph_layer->v_tick_count = 0;
	graph_layer->axis_colour = GColorBlack;
	graph_layer->tick_colour = GColorBlack;
	graph_layer->graph_colour = GColorBlack;
	graph_layer->background_colour = GColorWhite;
}

void graph_layer_set_max_value(GraphLayer* graph_layer, uint8_t max_value) {
	graph_layer->max_value = max_value;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_point_width(GraphLayer* graph_layer, uint8_t width) {
	graph_layer->point_width = width;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_axis_colour(GraphLayer* graph_layer, GColor axis_colour) {
	graph_layer->axis_colour = axis_colour;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_graph_colour(GraphLayer* graph_layer, GColor graph_colour) {
	graph_layer->graph_colour = graph_colour;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_tick_colour(GraphLayer* graph_layer, GColor tick_colour) {
	graph_layer->tick_colour = tick_colour;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_data(GraphLayer* graph_layer, uint8_t* data, uint8_t length) {
	graph_layer->data = data;
	graph_layer->data_count = length;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_vertical_ticks(GraphLayer* graph_layer, uint8_t* ticks, uint8_t length) {
	graph_layer->v_ticks = ticks;
	graph_layer->v_tick_count = length;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_horizontal_ticks(GraphLayer* graph_layer, uint8_t* ticks, uint8_t length) {
	graph_layer->h_ticks = ticks;
	graph_layer->h_tick_count = length;
	layer_mark_dirty(&graph_layer->layer);
}

void graph_layer_set_background_colour(GraphLayer* graph_layer, GColor background_colour) {
	graph_layer->background_colour = background_colour;
	layer_mark_dirty(&graph_layer->layer);
}

static void graph_layer_draw(Layer* layer, GContext* ctx) {
	// Draw the background, if we have one
	GraphLayer* graph_layer = (GraphLayer*)layer;
	GRect frame = layer_get_frame(layer);
	frame.origin.x = 0;
	frame.origin.y = 0;
	if(graph_layer->background_colour != GColorClear) {
		graphics_context_set_fill_color(ctx, graph_layer->background_colour);
		graphics_fill_rect(ctx, frame, 0, GCornerNone);
	}
	// Draw the axes
	if(graph_layer->axis_colour != GColorClear) {
		graphics_context_set_stroke_color(ctx, graph_layer->axis_colour);
		graphics_draw_line(ctx, GPoint(0, frame.size.h - 1), GPoint(frame.size.w, frame.size.h - 1));
		graphics_draw_line(ctx, GPoint(0, frame.size.h - 1), GPoint(0, 0));
	}
	// Draw the graph
	if(graph_layer->graph_colour != GColorClear && graph_layer->data && graph_layer->data_count) {
		graphics_context_set_stroke_color(ctx, graph_layer->graph_colour);
		graphics_context_set_fill_color(ctx, graph_layer->graph_colour);
		// Iterate over each point
		uint8_t max_points = frame.size.w / graph_layer->point_width;
		for(uint8_t i = 0; i < graph_layer->data_count && i < max_points; ++i) {
			// Draw one line per pixel, interpolating between points.
			uint8_t datum = graph_layer->data[i];
			uint8_t height = (datum * frame.size.h) / graph_layer->max_value;
			uint8_t next_datum = i+1 < graph_layer->data_count ? graph_layer->data[i+1] : datum;
			uint8_t next_height = (next_datum * frame.size.h) / graph_layer->max_value;
			float step_size = ((next_height - height) / graph_layer->point_width);
			for(uint8_t j = 0; j < graph_layer->point_width; ++j) {
				uint8_t x = graph_layer->point_width * i + j;
				uint8_t y = height + step_size * j;
				graphics_draw_line(ctx, GPoint(x, frame.size.h), GPoint(x, frame.size.h - 1 - y));
			}
		}
	}
	// Draw the ticks
	graphics_context_set_fill_color(ctx, graph_layer->tick_colour);
	graphics_context_set_stroke_color(ctx, graph_layer->tick_colour);
	if(graph_layer->v_ticks && graph_layer->v_tick_count && graph_layer->tick_colour != GColorClear) {
		for(uint8_t i = 0; i < graph_layer->v_tick_count; ++i) {
			uint8_t tick = graph_layer->v_ticks[i];
			uint8_t height = (tick * frame.size.h) / graph_layer->max_value;
			// Draw a dashed line.
			for(uint8_t j = 0; j < frame.size.w; j += 2) {
				// For some reason drawing pixels did nothing, but drawing length-one lines works.
				graphics_draw_line(ctx, GPoint(j, frame.size.h - height), GPoint(j, frame.size.h - height));
			}
		}
	}
	if(graph_layer->h_ticks && graph_layer->h_tick_count && graph_layer->tick_colour != GColorClear) {
		for(uint8_t i = 0; i < graph_layer->h_tick_count; ++i) {
			uint8_t tick = graph_layer->h_ticks[i];
			uint8_t x = tick * graph_layer->point_width;
			// Draw a dashed line.
			for(uint8_t j = 0; j < frame.size.h; j += 2) {
				// For some reason drawing pixels did nothing, but drawing length-one lines works.
				graphics_draw_line(ctx, GPoint(x, j), GPoint(x, j));
			}
		}
	}
}
