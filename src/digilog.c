#include <pebble.h>
#include "digilog.h"
	
enum Settings { setting_secondHand, setting_BT, setting_vibrate, setting_bat };	
static int BTDisp=1;
static int vibrate=1;
static int batDisp=1;
static int secHandsDisp=0;
	
int minTextBoxSize = 28;
int hourTextBoxSize = 24;
int secTextBoxSize = 20;
int digitPad = 4;
static AppSync app;
static uint8_t buffer[256];

static struct digilogData 
{
	Window *window;
	
	Layer *bg_layer;
	
	Layer *date_layer;
	TextLayer *day_label;
	char day_buffer[6];
	TextLayer *num_label;
	char num_buffer[4];

	Layer *hands_layer;
	TextLayer *sec_layer;
	char sec_buffer[3];
	TextLayer *min_layer;
	char min_buffer[3];
	TextLayer *hour_layer;
	char hour_buffer[3];

	GPath *minute_arrow, *hour_arrow;
	GPath *tick_paths[NUM_CLOCK_TICKS];
	GPath *bt_paths[NUM_BT_LINES];
	GPath *bat_paths[NUM_BAT_LINES];
} s_data;

void bg_update_proc(Layer* me, GContext* ctx) 
{

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, layer_get_bounds(me), 0, GCornerNone);

	graphics_context_set_fill_color(ctx, GColorWhite);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "draw tick_paths");		
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
  		gpath_draw_filled(ctx, s_data.tick_paths[i]);
	}
	
	if(BTDisp)
	{
		if(bluetooth_connection_service_peek())
		{	
			graphics_context_set_stroke_color(ctx, GColorWhite);
			graphics_context_set_fill_color(ctx, GColorWhite);	
			for (int i = 0; i < NUM_BT_LINES; ++i) 
			{
				gpath_draw_outline(ctx, s_data.bt_paths[i]);
			}
		}
	}
	
	if(batDisp)
	{	
		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_fill_color(ctx, GColorWhite);	
	
		BatteryChargeState bat_stat = battery_state_service_peek();
		uint8_t bat_level = bat_stat.charge_percent;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "bat : %d", bat_level);
		int BatNumToFill = 0;
		if( bat_level > 87.5 ) BatNumToFill = 10;
		else if( bat_level > 75 ) BatNumToFill = 9;
		else if( bat_level > 62.5 ) BatNumToFill = 8;
		else if( bat_level > 50 ) BatNumToFill = 7;
		else if( bat_level > 37.5 ) BatNumToFill = 6;
		else if( bat_level > 25 ) BatNumToFill = 5;
		else if( bat_level > 12.5 ) BatNumToFill = 4;
		else BatNumToFill = 3;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "batnumtofill : %d", BatNumToFill);
		//BatNumToFill = BatNumToFill + 1;
		for (int i = 0; i < BatNumToFill; ++i) 
		{
			gpath_draw_outline(ctx, s_data.bat_paths[i]);
		}
	}
}

void date_update_proc(Layer *me, GContext* ctx) 
{
	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	
	strftime(s_data.day_buffer, sizeof(s_data.day_buffer), "%a", current_time);
	text_layer_set_text(s_data.day_label, s_data.day_buffer);

	strftime(s_data.num_buffer, sizeof(s_data.num_buffer), "%d", current_time);
	text_layer_set_text(s_data.num_label, s_data.num_buffer);
}

void hands_update_proc(Layer* me, GContext* ctx) 
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	GPoint secondHand;
	GPoint minHand;
	GPoint hourHand;
	GPoint secText;
	GPoint minText;
	GPoint hourText;

	GRect cp = layer_get_bounds(me);
	const GPoint center = grect_center_point(&cp);
	
	const int16_t secondHandLength = 45;
	const int16_t secondHandNumLen = secondHandLength + (secTextBoxSize/2) + digitPad;
	
	//const int16_t minHandLength = (me->bounds.size.w / 4);
	const int16_t minHandLength = 30;
	const int16_t minHandNumLen = minHandLength + (minTextBoxSize/2) + digitPad;
	
	//const int16_t hourHandLength = (me->bounds.size.w / 2) * (.25);
	const int16_t hourHandLength = 10;
	const int16_t hourHandNumLen = hourHandLength + (hourTextBoxSize/2) + digitPad;

	//second hand
	int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
	secondHand.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
	secText.y    = (int16_t)(-cos_lookup(second_angle) * (int32_t)secondHandNumLen / TRIG_MAX_RATIO) + center.y;
	secondHand.x = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;
	secText.x    = (int16_t)(sin_lookup(second_angle) * (int32_t)secondHandNumLen / TRIG_MAX_RATIO) + center.x;

	//min hand
	int32_t min_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
	//int32_t min_angle = TRIG_MAX_ANGLE * 30 / 60;
	minHand.y = (int16_t)(-cos_lookup(min_angle) * (int32_t)minHandLength / TRIG_MAX_RATIO) + center.y;
	minText.y = (int16_t)(-cos_lookup(min_angle) * (int32_t)minHandNumLen / TRIG_MAX_RATIO) + center.y;
	minHand.x = (int16_t)(sin_lookup(min_angle) * (int32_t)minHandLength / TRIG_MAX_RATIO) + center.x;
	minText.x = (int16_t)(sin_lookup(min_angle) * (int32_t)minHandNumLen / TRIG_MAX_RATIO) + center.x;

	//hour hand
	int32_t hour_angle = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10)) / (12 * 6);
	//int32_t hour_angle = TRIG_MAX_ANGLE * (((6 % 12) * 6) + (6 / 10)) / (12 * 6);
	hourHand.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.y;
	hourText.y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)hourHandNumLen / TRIG_MAX_RATIO) + center.y;
	hourHand.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandLength / TRIG_MAX_RATIO) + center.x;
	hourText.x = (int16_t)(sin_lookup(hour_angle) * (int32_t)hourHandNumLen / TRIG_MAX_RATIO) + center.x;

	// draw hands
	graphics_context_set_stroke_color(ctx, GColorWhite);
	if(secHandsDisp) graphics_draw_line(ctx, secondHand, center);
	graphics_draw_line(ctx, minHand, center);
	graphics_draw_line(ctx, hourHand, center);

	// dot in the middle
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect((layer_get_bounds(me).size.w) / 2-1, (layer_get_bounds(me).size.h) / 2-1, 3, 3), 0, GCornerNone);

	if(secHandsDisp) 
	{
		// update sec
		strftime(s_data.sec_buffer, sizeof(s_data.sec_buffer), "%S", t);
		//s_data.sec_buffer[0] = '3';
		//s_data.sec_buffer[1] = '0';
		//s_data.sec_buffer[2] = '\0';
		text_layer_set_text(s_data.sec_layer, s_data.sec_buffer);
		layer_set_frame(text_layer_get_layer(s_data.sec_layer), GRect((secText.x - (secTextBoxSize/2)), (secText.y - (secTextBoxSize/2)), secTextBoxSize,secTextBoxSize));
	}
	
	// update min
	strftime(s_data.min_buffer, sizeof(s_data.min_buffer), "%M", t);
	//s_data.min_buffer[0] = 'X';
	//s_data.min_buffer[1] = 'X';
	//s_data.min_buffer[2] = '\0';
	text_layer_set_text(s_data.min_layer, s_data.min_buffer);
	layer_set_frame(text_layer_get_layer(s_data.min_layer), GRect((minText.x - (minTextBoxSize/2)), (minText.y - (minTextBoxSize/2)), minTextBoxSize, minTextBoxSize));

	// update hour
	strftime(s_data.hour_buffer, sizeof(s_data.hour_buffer), "%l", t);
	//s_data.hour_buffer[0] = 'X';
	//s_data.hour_buffer[1] = 'X';
	//s_data.hour_buffer[2] = '\0';
	text_layer_set_text(s_data.hour_layer, s_data.hour_buffer);
	layer_set_frame(text_layer_get_layer(s_data.hour_layer), GRect((hourText.x - (hourTextBoxSize/2)), (hourText.y - (hourTextBoxSize/2)), hourTextBoxSize,hourTextBoxSize));
}

static void handle_bluetooth(bool connected)
{
	if(!connected)
	{
		if(vibrate) vibes_short_pulse();
	}
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) 
{
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "tick");
	layer_mark_dirty(window_get_root_layer(s_data.window));
}

static void tuple_changed_callback(const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context) 
{
	//  we know these values are uint8 format
	int value = tuple_new->value->uint8;
  	switch (key) 
	{
    	case setting_secondHand:
		{
      		APP_LOG(APP_LOG_LEVEL_DEBUG, "setting_secondHand %d", value);
      		break;
		}
    	case setting_BT:
		{
      		APP_LOG(APP_LOG_LEVEL_DEBUG, "setting_BT %d", value);
			break;
		}
    	case setting_vibrate:
		{
      		APP_LOG(APP_LOG_LEVEL_DEBUG, "setting_vibrate %d", value);
			break;
		}
    	case setting_bat:
		{
      		APP_LOG(APP_LOG_LEVEL_DEBUG, "setting_bat %d", value);
			break;
		}
	}
}

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) 
{
  	APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

void handle_init(void) 
{
	//fonts
	GFont bold18 = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
	GFont norm18 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
	GFont g28    = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
	GFont g24    = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
	
	//init buffers
	s_data.day_buffer[0] = '\0';
	s_data.num_buffer[0] = '\0';
	//s_data.sec_buffer[0] = '\0';
	s_data.min_buffer[0] = '\0';
	s_data.hour_buffer[0] = '\0';

	Tuplet tuples[] = 
	{
		TupletInteger(setting_secondHand, secHandsDisp),
		TupletInteger(setting_BT, BTDisp),
		TupletInteger(setting_vibrate, vibrate),
		TupletInteger(setting_bat, batDisp)
	};

	app_sync_init(&app, buffer, sizeof(buffer), tuples, ARRAY_LENGTH(tuples),
				  	tuple_changed_callback, app_error_callback, NULL);	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "app sync init");	
	
	// Create a window and text layer
	s_data.window = window_create();
	window_set_fullscreen(s_data.window, true);
	s_data.bg_layer = layer_create(layer_get_frame(window_get_root_layer(s_data.window)));
	s_data.date_layer = layer_create(layer_get_frame(s_data.bg_layer));
	s_data.hands_layer = layer_create(layer_get_frame(s_data.bg_layer));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created window and layers");

	// init clock face paths
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
		s_data.tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "init clock face paths");

	// init BT icon paths
	for (int i = 0; i < NUM_BT_LINES; ++i) 
	{
		s_data.bt_paths[i] = gpath_create(&BLUETOOTH_POINTS[i]);
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "init bt icon paths");

	// init BT icon paths
	for (int i = 0; i < NUM_BAT_LINES; ++i) 
	{
		s_data.bat_paths[i] = gpath_create(&BATTERY_POINTS[i]);
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "init bt icon paths");

	// init day
	s_data.day_label = text_layer_create(GRect(2, 151, 40, 20));
	text_layer_set_text(s_data.day_label, s_data.day_buffer);
	text_layer_set_background_color(s_data.day_label, GColorClear);
	text_layer_set_text_color(s_data.day_label, GColorWhite);
	text_layer_set_font(s_data.day_label, norm18);
	layer_add_child(s_data.date_layer, text_layer_get_layer(s_data.day_label));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created day_label textLayer");

	// init num
	s_data.num_label = text_layer_create(GRect(144 - 16, 151, 18, 20));
	text_layer_set_text(s_data.num_label, s_data.num_buffer);
	text_layer_set_background_color(s_data.num_label, GColorClear);
	text_layer_set_text_color(s_data.num_label, GColorWhite);
	text_layer_set_font(s_data.num_label, norm18);
	layer_add_child(s_data.date_layer, text_layer_get_layer(s_data.num_label));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created num_label textLayer");
		
	//init sec -> child layer of hands_layer
	s_data.sec_layer = text_layer_create(GRect(1, 1, secTextBoxSize, secTextBoxSize));
	text_layer_set_text(s_data.sec_layer, s_data.sec_buffer);
	text_layer_set_text_alignment(s_data.sec_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_data.sec_layer, GColorClear);
	text_layer_set_text_color(s_data.sec_layer, GColorWhite);
	text_layer_set_font(s_data.sec_layer, norm18);
	layer_add_child(s_data.hands_layer, text_layer_get_layer(s_data.sec_layer));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created sec_label textLayer");

	//init min -> child layer of hands_layer
	s_data.min_layer = text_layer_create(GRect(1, 1, minTextBoxSize, minTextBoxSize));
	text_layer_set_text(s_data.min_layer, s_data.min_buffer);
	text_layer_set_text_alignment(s_data.min_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_data.min_layer, GColorClear);
	text_layer_set_text_color(s_data.min_layer, GColorWhite);
	text_layer_set_font(s_data.min_layer, g28);
	layer_add_child(s_data.hands_layer, text_layer_get_layer(s_data.min_layer));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created min_label textLayer");

	//init hour -> child layer of hands_layer
	s_data.hour_layer = text_layer_create(GRect(1, 1, hourTextBoxSize, hourTextBoxSize));	
	text_layer_set_text(s_data.hour_layer, s_data.hour_buffer);
	text_layer_set_text_alignment(s_data.hour_layer, GTextAlignmentCenter);
	text_layer_set_background_color(s_data.hour_layer, GColorClear);
	text_layer_set_text_color(s_data.hour_layer, GColorWhite);
	text_layer_set_font(s_data.hour_layer, g24);
	layer_add_child(s_data.hands_layer, text_layer_get_layer(s_data.hour_layer));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "created hour_label textLayer");

	//update proc
	layer_set_update_proc(s_data.bg_layer, bg_update_proc);
	layer_set_update_proc(s_data.date_layer, date_update_proc);	
	layer_set_update_proc(s_data.hands_layer, hands_update_proc);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "set update_procs");
	
	//add layers to window
	layer_add_child(s_data.bg_layer, s_data.date_layer);
	layer_add_child(s_data.bg_layer, s_data.hands_layer);
	layer_add_child(window_get_root_layer(s_data.window), s_data.bg_layer);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "finished Layer setup");
	
	// Push the window
	window_stack_push(s_data.window, true);	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "secHandsDisp %d", secHandsDisp);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "BTDisp %d", BTDisp);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "vibrate %d", vibrate);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "batDisp %d", batDisp);

	if(secHandsDisp)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "handle second tick");
		handle_tick(NULL, SECOND_UNIT);
		tick_timer_service_subscribe(SECOND_UNIT, handle_tick);	
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "handle minute tick");
		handle_tick(NULL, MINUTE_UNIT);
		tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);		
	}
	
	bluetooth_connection_service_subscribe(handle_bluetooth);
}

void handle_deinit(void) 
{
	tick_timer_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	// Destroy the date layer
	text_layer_destroy(s_data.day_label);
	text_layer_destroy(s_data.num_label);
	layer_destroy(s_data.date_layer);

	//destroy hands layer
	text_layer_destroy(s_data.sec_layer);
	text_layer_destroy(s_data.min_layer);
	text_layer_destroy(s_data.hour_layer);
	layer_destroy(s_data.hands_layer);

	//destroy bg layer
	layer_destroy(s_data.bg_layer);

	//destroy paths
	for (int i = 0; i < NUM_CLOCK_TICKS; ++i) 
	{
		gpath_destroy(s_data.tick_paths[i]);
	}	

	//destroy paths
	for (int i = 0; i < NUM_BT_LINES; ++i) 
	{
		gpath_destroy(s_data.bt_paths[i]);
	}	

	//destroy paths
	for (int i = 0; i < NUM_BAT_LINES; ++i) 
	{
		gpath_destroy(s_data.bat_paths[i]);
	}	
	// Destroy the window
	window_destroy(s_data.window);
	
	app_sync_deinit(&app);
}

int main(void) 
{
	handle_init();
	app_event_loop();
	handle_deinit();
}
