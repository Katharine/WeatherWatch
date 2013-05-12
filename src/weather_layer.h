#ifndef WEATHER_LAYER_H
#define WEATHER_LAYER_H

typedef struct {
	Layer layer;
	BmpContainer icon_layer;
	TextLayer temp_layer;
	bool has_weather_icon;
	char temp_str[5];
} WeatherLayer;

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

void weather_layer_init(WeatherLayer* weather_layer, GPoint pos);
void weather_layer_deinit(WeatherLayer* weather_layer);
void weather_layer_set_icon(WeatherLayer* weather_layer, WeatherIcon icon);
void weather_layer_set_temperature(WeatherLayer* weather_layer, int16_t temperature);

#endif
