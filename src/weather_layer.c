#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "util.h"
#include "weather_layer.h"

static uint8_t WEATHER_ICONS[] = {
	RESOURCE_ID_ICON_CLEAR_DAY,
	RESOURCE_ID_ICON_CLEAR_NIGHT,
	RESOURCE_ID_ICON_RAIN,
	RESOURCE_ID_ICON_SNOW,
	RESOURCE_ID_ICON_SLEET,
	RESOURCE_ID_ICON_WIND,
	RESOURCE_ID_ICON_FOG,
	RESOURCE_ID_ICON_CLOUDY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_DAY,
	RESOURCE_ID_ICON_PARTLY_CLOUDY_NIGHT,
	RESOURCE_ID_ICON_ERROR,
};

void weather_layer_init(WeatherLayer* weather_layer, GPoint pos) {
	layer_init(&weather_layer->layer, GRect(pos.x, pos.y, 144, 68));
	// Add temperature layer
	text_layer_init(&weather_layer->temp_layer, GRect(70, 9, 64, 68));
	text_layer_set_text_alignment(&weather_layer->temp_layer, GTextAlignmentCenter);
	text_layer_set_font(&weather_layer->temp_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_40)));
	layer_add_child(&weather_layer->layer, &weather_layer->temp_layer.layer);
	// Note absence of icon layer
	weather_layer->has_weather_icon = false;
	
	// Set up the precipitation forecast
	graph_layer_init(&weather_layer->graph_layer, GRect(10, 10, 60, 50));
	graph_layer_set_max_value(&weather_layer->graph_layer, 255);
	graph_layer_set_point_width(&weather_layer->graph_layer, 2);
	graph_layer_set_axis_colour(&weather_layer->graph_layer, GColorBlack);
	graph_layer_set_graph_colour(&weather_layer->graph_layer, GColorBlack);
	graph_layer_set_tick_colour(&weather_layer->graph_layer, GColorBlack);
	graph_layer_set_background_colour(&weather_layer->graph_layer, GColorWhite);
	static uint8_t vticks[2] = {85, 171};
	graph_layer_set_vertical_ticks(&weather_layer->graph_layer, vticks, ARRAY_LENGTH(vticks));
	static uint8_t hticks[2] = {10, 20};
	graph_layer_set_horizontal_ticks(&weather_layer->graph_layer, hticks, ARRAY_LENGTH(hticks));
}

void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon) {
	if(!weather_layer->has_forecast) {
		if(weather_layer->has_weather_icon) {
			layer_remove_from_parent(&weather_layer->icon_layer.layer.layer);
			bmp_deinit_container(&weather_layer->icon_layer);
			weather_layer->has_weather_icon = false;
		}
		bmp_init_container(WEATHER_ICONS[icon], &weather_layer->icon_layer);
		layer_add_child(&weather_layer->layer, &weather_layer->icon_layer.layer.layer);
		layer_set_frame(&weather_layer->icon_layer.layer.layer, GRect(10, 4, 60,60));
		weather_layer->has_weather_icon = true;
	}
	weather_layer->current_icon = icon;
}

void weather_layer_set_temperature(WeatherLayer* weather_layer, int16_t t) {
	memcpy(weather_layer->temp_str, itoa(t), 4);
	int degree_pos = strlen(weather_layer->temp_str);
	memcpy(&weather_layer->temp_str[degree_pos], "°", 3);
	text_layer_set_text(&weather_layer->temp_layer, weather_layer->temp_str);
}

void weather_layer_deinit(WeatherLayer* weather_layer) {
	if(weather_layer->has_weather_icon)
		bmp_deinit_container(&weather_layer->icon_layer);
}

void weather_layer_set_precipitation_forecast(WeatherLayer* weather_layer, uint8_t* forecast, uint8_t length) {
	weather_layer->forecast = forecast;
	weather_layer->forecast_count = length;
	// Check whether we actually have precipitation in the next half hour.
	bool has_forecast = false;
	for(uint8_t i = 0; i < 30 && i < length; ++i) {
		if(forecast[i]) {
			has_forecast = true;
			break;
		}
	}
	if(has_forecast) {
		// Get rid of the weather icon, if we have one.
		if(weather_layer->has_weather_icon) {
			layer_remove_from_parent(&weather_layer->icon_layer.layer.layer);
			bmp_deinit_container(&weather_layer->icon_layer);
			weather_layer->has_weather_icon = false;
		}
		graph_layer_set_data(&weather_layer->graph_layer, forecast, length);
		layer_add_child(&weather_layer->layer, &weather_layer->graph_layer.layer);
		weather_layer->has_forecast = true;
	} else {
		weather_layer_clear_precipitation_forecast(weather_layer);
	}
}
void weather_layer_clear_precipitation_forecast(WeatherLayer* weather_layer) {
	if(weather_layer->has_forecast) {
		weather_layer->forecast = NULL;
		weather_layer->forecast_count = -1;
		weather_layer->has_forecast = false;
		layer_remove_from_parent(&weather_layer->graph_layer.layer);
		graph_layer_set_data(&weather_layer->graph_layer, NULL, 0);
		weather_layer_set_icon(weather_layer, weather_layer->current_icon);
	}
}