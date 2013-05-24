#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
	
#include "http.h"
#include "util.h"
#include "time_layer.h"
#include "weather_layer.h"
#include "config.h"

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }
PBL_APP_INFO(MY_UUID,
             "Weather Watch", "Katharine Berry",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

// POST variables
#define WEATHER_KEY_LATITUDE 1
#define WEATHER_KEY_LONGITUDE 2
#define WEATHER_KEY_UNIT_SYSTEM 3
// Received variables
#define WEATHER_KEY_CURRENT 1
#define WEATHER_KEY_PRECIPITATION 3

#define WEATHER_HTTP_COOKIE 1949327671

static Window window;
static TimeLayer time_layer;
static WeatherLayer weather_layer;

static int our_latitude, our_longitude;
static bool located;

static uint8_t precip_forecast[60];
static int8_t precip_forecast_index;

void request_weather();
void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);

void failed(int32_t cookie, int http_status, void* context) {
	if(cookie == 0 || cookie == WEATHER_HTTP_COOKIE) {
		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
	}
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
	if(cookie != WEATHER_HTTP_COOKIE) return;
	Tuple* data_tuple = dict_find(received, WEATHER_KEY_CURRENT);
	if(data_tuple) {
		// The below bitwise dance is so we can actually fit our precipitation forecast.
		uint16_t value = data_tuple->value->int16;
		uint8_t icon = value >> 11;
		if(icon < 10) {
			weather_layer_set_icon(&weather_layer, icon);
		} else {
			weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
		}
		int16_t temp = value & 0x3ff;
		if(value & 0x400) temp = -temp;
		weather_layer_set_temperature(&weather_layer, temp);
	}
	Tuple* forecast_tuple = dict_find(received, WEATHER_KEY_PRECIPITATION);
	if(forecast_tuple) {
		// It's going to rain!
		memset(precip_forecast, 0, 60);
		memcpy(precip_forecast, forecast_tuple->value->data, forecast_tuple->length > 60 ? 60 : forecast_tuple->length);
		precip_forecast_index = 0;
		weather_layer_set_precipitation_forecast(&weather_layer, precip_forecast, 60);
	} else {
		weather_layer_clear_precipitation_forecast(&weather_layer);
	}
}

void reconnect(void* context) {
	request_weather();
}

void handle_tick(AppContextRef app_ctx, PebbleTickEvent *event) {
	time_layer_set_time(&time_layer, *(event->tick_time));
	if(precip_forecast_index >= 0) {
		++precip_forecast_index;
		if(precip_forecast_index > 60) {
			weather_layer_clear_precipitation_forecast(&weather_layer);
			precip_forecast_index = -1;
		} else {
			weather_layer_set_precipitation_forecast(&weather_layer, &precip_forecast[precip_forecast_index], 60 - precip_forecast_index);
		}
	}
}

void set_timer(AppContextRef ctx) {
	app_timer_send_event(ctx, 1740000, 1);
}

void location(float latitude, float longitude, float altitude, float accuracy, void* context) {
	// Fix the floats
	our_latitude = latitude * 10000;
	our_longitude = longitude * 10000;
	located = true;
	request_weather();
	set_timer((AppContextRef)context);
}

void handle_init(AppContextRef ctx) {
	resource_init_current_app(&APP_RESOURCES);
	window_init(&window, "Weather Watch");
	window_stack_push(&window, true /* Animated */);
	window_set_fullscreen(&window, true);
	
	// Add time layer.
	time_layer_init(&time_layer, GPoint(0, 0));
	layer_add_child(&window.layer, &time_layer.layer);
	// Set initial time.
	PblTm time;
	get_time(&time);
	time_layer_set_time(&time_layer, time);
	// Add weather layer
	weather_layer_init(&weather_layer, GPoint(0, 100));
	layer_add_child(&window.layer, &weather_layer.layer);
	
	http_register_callbacks((HTTPCallbacks){
		.failure=failed,
		.success=success,
		.reconnect=reconnect,
		.location=location
	}, (void*)ctx);
	
	// Request weather
	precip_forecast_index = -1;
	located = false;
	request_weather();
}

void handle_deinit(AppContextRef ctx) {
	time_layer_deinit(&time_layer);
	weather_layer_deinit(&weather_layer);
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,
		.tick_info = {
			.tick_handler = &handle_tick,
			.tick_units = MINUTE_UNIT
		},
		.timer_handler = handle_timer,
		.messaging_info = {
			.buffer_sizes = {
				.inbound = 124,
				.outbound = 256,
			}
		}
	};
	app_event_loop(params, &handlers);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	request_weather();
	// Update again in fifteen minutes.
	if(cookie)
		set_timer(ctx);
}

void request_weather() {
	if(!located) {
		http_location_request();
		return;
	}
	// Build the HTTP request
	DictionaryIterator *body;
	HTTPResult result = http_out_get("http://pwdb.kathar.in/pebble/weather3.php", WEATHER_HTTP_COOKIE, &body);
	if(result != HTTP_OK) {
		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
		return;
	}
	dict_write_int32(body, WEATHER_KEY_LATITUDE, our_latitude);
	dict_write_int32(body, WEATHER_KEY_LONGITUDE, our_longitude);
	dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM);
	// Send it.
	if(http_out_send() != HTTP_OK) {
		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
		return;
	}
}
