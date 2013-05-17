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
#define WEATHER_KEY_ICON 1
#define WEATHER_KEY_TEMPERATURE 2

#define WEATHER_HTTP_COOKIE 1949327671

static Window window;
static TimeLayer time_layer;
static WeatherLayer weather_layer;

static int our_latitude, our_longitude;
static bool located;

void request_weather();
void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);

void failed(int32_t cookie, int http_status, void* context) {
	if(cookie == 0 || cookie == WEATHER_HTTP_COOKIE) {
		weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
	}
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
	if(cookie != WEATHER_HTTP_COOKIE) return;
	Tuple* icon_tuple = dict_find(received, WEATHER_KEY_ICON);
	if(icon_tuple) {
		int icon = icon_tuple->value->int8;
		if(icon >= 0 && icon < 10) {
			weather_layer_set_icon(&weather_layer, icon);
		} else {
			weather_layer_set_icon(&weather_layer, WEATHER_ICON_NO_WEATHER);
		}
	}
	Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE);
	if(temperature_tuple) {
		weather_layer_set_temperature(&weather_layer, temperature_tuple->value->int16);
	}
}

void reconnect(void* context) {
	request_weather();
}

void handle_tick(AppContextRef app_ctx, PebbleTickEvent *event) {
	time_layer_set_time(&time_layer, *(event->tick_time));
}

void location(float latitude, float longitude, float altitude, float accuracy) {
	// Fix the floats
	our_latitude = latitude * 10000;
	our_longitude = longitude * 10000;
	located = true;
	request_weather();
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
	
	http_register_callbacks((HTTPCallbacks){.failure=failed,.success=success,.reconnect=reconnect,.location=location}, (void*)ctx);
	
	// Request weather information in a second.
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
				.inbound = 256,
				.outbound = 256,
			}
		}
	};
	app_event_loop(params, &handlers);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	//request_weather();
	http_location_request();
	// Update again in fifteen minutes.
	if(cookie)
		app_timer_send_event(ctx, 900000, 1);
}

void request_weather() {
	if(!located) {
		http_location_request();
		return;
	}
	// Build the HTTP request
	DictionaryIterator *body;
	HTTPResult result = http_out_get("http://pwdb.kathar.in/pebble/weather2.php", WEATHER_HTTP_COOKIE, &body);
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
