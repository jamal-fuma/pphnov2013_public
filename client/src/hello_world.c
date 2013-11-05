#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

typedef char time_str_buffer_t[32];

#define WINDOW_NAME 	("Window Name")
#define WINDOW_ANIMATED (true)
#define THIS_APP_NAME	("Template App")
#define THIS_ORG_NAME	("Your Company")
#define THIS_MAJOR_VER	(1)
#define THIS_MINOR_VER	(0)

#define MY_UUID { 0x96, 0x9D, 0x87, 0x77, 0xB1, 0x9A, 0x4D, 0x14, 0x9D, 0x78, 0xE0, 0x65, 0xE4, 0xF1, 0x34, 0xA1 }

enum
{
	MSG_IN_KEY = 0x00,
	MSG_OUT_KEY = 0x01
};

// cmds
enum
{
	MSG_CMD_VIBE = 0x00,
	MSG_CMD_UP = 0x01,
	MSG_CMD_DOWN = 0x02
} MessageCommand;


/* setup layers */
void init_handler( AppContextRef ctx);

/* cleanup layers */
void deinit_handler( AppContextRef ctx);

/* redraw graphics layers */
void update_layer_callback( Layer *me, GContext* ctx);

/* called once per tick to drive redraw */
void timer_handler( AppContextRef ctx, PebbleTickEvent *event);

/* wire up button handlers  */
void click_config_provider( ClickConfig **config, Window *window);

/* button handlers */
void up_single_click_handler( ClickRecognizerRef recognizer, Window *window);
void down_single_click_handler( ClickRecognizerRef recognizer, Window *window);
void select_single_click_handler( ClickRecognizerRef recognizer, Window *window);
void select_long_click_handler( ClickRecognizerRef recognizer, Window *window);

/* comms primitives */
void send_cmd(uint8_t cmd);
void pack_command(DictionaryIterator *p_dict_iterator, uint8_t cmd);

/* wire up comms handlers*/
bool register_callbacks();

/* comms handlers */
void app_send_failed( DictionaryIterator* failed, AppMessageResult reason, void* context);
void app_received_msg( DictionaryIterator* received, void* context);
void app_dropped_msg( void* context, AppMessageResult reason);


/* fill out application context */
PBL_APP_INFO(
		MY_UUID,
		THIS_APP_NAME,
		THIS_ORG_NAME,
		THIS_MAJOR_VER,
		THIS_MINOR_VER,
		DEFAULT_MENU_ICON,
		APP_INFO_STANDARD_APP
	    );


/* Globals */
Window 		  	g_window;
Layer 	  	  	g_layer;
time_str_buffer_t 	g_time_buf = {0};
AppMessageCallbacksNode g_app_callbacks;
bool			g_callbacks_registered = false;

/* main handlers */
void init_handler(
		AppContextRef ctx)
{
	/* setup a top-level window */
	window_init(
			&g_window,
			WINDOW_NAME
	);
	window_stack_push(
			&g_window,
			WINDOW_ANIMATED
	);

	/* add a child graphics layer bound to the top-level window */
  	layer_init(
			&g_layer,
			g_window.layer.frame
	);

  	g_layer.update_proc = update_layer_callback;

  	layer_add_child(
			&g_window.layer,
			&g_layer
	);
  	
	/* mark the layer as needing to be redrawn */
	layer_mark_dirty(
			&g_layer
	);

	/* wire up comms layer */
  	register_callbacks();

	/* wire up button handlers */
	window_set_click_config_provider(
			&g_window,
			(ClickConfigProvider)click_config_provider
	);
}

void deinit_handler(
		AppContextRef ctx)
{
}

void timer_handler(
		AppContextRef ctx,
		PebbleTickEvent *event)
{
	/* update the display with new time */
	string_format_time(
			g_time_buf, sizeof(time_str_buffer_t),
			"%I:%M:%S %p",
			event->tick_time
	);
	
	/* mark the layer as needing to be redrawn */
	layer_mark_dirty(
			&g_layer
	);

}

void update_layer_callback(
		Layer *me,
		GContext* ctx)
{
	graphics_context_set_text_color(
			ctx,
			GColorBlack
	);

	graphics_text_draw(
			ctx,
			g_time_buf,
			fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
			GRect(5, 5, 144-10, 100),
			GTextOverflowModeWordWrap,
			GTextAlignmentLeft,
			NULL
	);

	graphics_text_draw(
			ctx,
			"And text here as well.",
			fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
			GRect(90, 100, 144-95, 60),
			GTextOverflowModeWordWrap,
			GTextAlignmentRight,
			NULL
	);
}

void click_config_provider(
		ClickConfig **config,
		Window *window)
{
	config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;
	config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

	config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
	config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

	config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
	config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}

void up_single_click_handler(
		ClickRecognizerRef recognizer,
		Window *window)
{
	  vibes_short_pulse();
  	  send_cmd(MSG_CMD_UP);
}


void down_single_click_handler(
		ClickRecognizerRef recognizer,
		Window *window)
{
	  vibes_long_pulse();
  	  send_cmd(MSG_CMD_DOWN);
}


void select_single_click_handler(
		ClickRecognizerRef recognizer,
		Window *window)
{
	  vibes_double_pulse();
}

void select_long_click_handler(
		ClickRecognizerRef recognizer,
		Window *window)
{
}

void pack_command(
	DictionaryIterator *p_dict_iterator,
	uint8_t cmd)
{
	/* pack command into a tuplet */
	Tuplet value = TupletInteger(
			MSG_OUT_KEY,
			cmd
	);
	dict_write_tuplet(
		p_dict_iterator,
		&value
	);
	dict_write_end(
		p_dict_iterator
	);
}

void send_cmd(
	uint8_t cmd)
{
	DictionaryIterator *p_dict_iterator = NULL;
	
	/* accquire some temporary storage */
	app_message_out_get(&p_dict_iterator);
	if (p_dict_iterator)
	{
		/* write into temporary storage */
		pack_command(
				p_dict_iterator,
				cmd
		);

		app_message_out_send();

		/* cleanup the storage */
		app_message_out_release();
	}
}

void app_send_failed(
		DictionaryIterator* failed,
		AppMessageResult reason,
		void* context)
{
  // TODO: error handling
}

void app_received_msg(
		DictionaryIterator* received,
		void* context)
{
	Tuple* cmd_tuple = dict_find(
			received,
			MSG_IN_KEY
	);
  
	if (cmd_tuple == NULL) 
	{
	    // TODO: handle invalid message
	    return;
	}
	
 	uint8_t cmd = cmd_tuple->value->int8;
	if (cmd == MSG_CMD_VIBE)
	{
	    vibes_long_pulse();
	}
}

void app_dropped_msg(
		void* context,
		AppMessageResult reason)
{
	// TODO: error handling
}


bool register_callbacks()
{
	if (g_callbacks_registered)
	{
		if (app_message_deregister_callbacks(&g_app_callbacks) == APP_MSG_OK)
			g_callbacks_registered = false;

	}

	g_app_callbacks = (AppMessageCallbacksNode){
		.callbacks = {
			.out_failed = app_send_failed,
			.in_received = app_received_msg,
			.in_dropped = app_dropped_msg,
		},
		.context = NULL
	};

	if (app_message_register_callbacks(&g_app_callbacks) == APP_MSG_OK) {
		g_callbacks_registered = true;
	}

	return g_callbacks_registered;
}

void pbl_main(
		void *params)
{
	/* setup event handlers */
	PebbleAppHandlers handlers = {
		.init_handler   = &init_handler,
		.deinit_handler = &deinit_handler,

		.tick_info = {
			.tick_handler = &timer_handler,
			.tick_units = SECOND_UNIT
		},

		.messaging_info = {
			.buffer_sizes = {
				.inbound = 256,
				.outbound = 256,
			}
		}
  };

  /* enter the main event loop */
  app_event_loop(
  	params,
	&handlers
  );
}
