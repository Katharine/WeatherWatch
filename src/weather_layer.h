#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

#include "graph_layer.h"

typedef enum {
	WEATHER_ICON_CLEAR_DAY = 0,
	WEATHER_ICON_CLEAR_NIGHT,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_SLEET,
	WEATHER_ICON_WIND,
	WEATHER_ICON_FOG,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_PARTLY_CLOUDY_DAY,
	WEATHER_ICON_PARTLY_CLOUDY_NIGHT,
	WEATHER_ICON_NO_WEATHER,
	WEATHER_ICON_COUNT
} WeatherIcon;

typedef struct {
	Layer layer;
	BmpContainer icon_layer;
	TextLayer temp_layer;
	GraphLayer graph_layer;
	uint8_t* forecast;
	uint8_t forecast_count;
	bool has_forecast;
	bool has_weather_icon;
	WeatherIcon current_icon;
	char temp_str[5];
} WeatherLayer;

void weather_layer_init(WeatherLayer* weather_layer, GPoint pos);
void weather_layer_deinit(WeatherLayer* weather_layer);
void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon);
void weather_layer_set_temperature(WeatherLayer* weather_layer, int16_t temperature);
void weather_layer_set_precipitation_forecast(WeatherLayer* weather_layer, uint8_t* forecast, uint8_t length);
void weather_layer_clear_precipitation_forecast(WeatherLayer* weather_layer);

#endif
