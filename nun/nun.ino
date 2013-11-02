#include <FastSPI_LED2.h> 

// limit max number of LEDs
#define MAX_NUM_LEDS	300

struct CRGB leds[MAX_NUM_LEDS];

CRGB interpolate(CRGB from, CRGB to, byte alpha) {
	CRGB ret;
    ret.r = map(alpha, 0, 255, from.r, to.r);
    ret.g = map(alpha, 0, 255, from.g, to.g);
    ret.b = map(alpha, 0, 255, from.b, to.b);
	return ret;
}

//---------------------------------------------------------------------------------------

#define FADE_LEN 7000
#define FADE_COLORS 4
int fade_offsets[MAX_NUM_LEDS] = {0};
CRGB fades1[][FADE_COLORS]={
  {CRGB(255,100,0),CRGB(255,140,0),CRGB(200,30,0),CRGB(0,0,0)}
};

CRGB fades2[][FADE_COLORS]={
  {CRGB::Blue,CRGB::Purple,CRGB::Magenta,CRGB::Cyan}
};


CRGB fades3[][FADE_COLORS]={
  {CRGB::Green,CRGB::Yellow,CRGB::Lime,CRGB::Turquoise}
};

float scales[6] = {0.742,0.9134,1.0,1.13,1.211,0.9577};

void uneven_fade(CRGB *fade_colors) {
  unsigned long ticks = millis();
  for(int i = 0; i < MAX_NUM_LEDS; i++) {
    unsigned int new_offset = (ticks + fade_offsets[i]) * scales[fade_offsets[i] % 6];
    int color = new_offset/(FADE_LEN/FADE_COLORS);
    int phase = new_offset%(FADE_LEN/FADE_COLORS);
    leds[i] = interpolate(fade_colors[color%FADE_COLORS], fade_colors[(color+1)%FADE_COLORS], map(phase,0,7000/FADE_COLORS,0,255));
  } 
}

//---------------------------------------------------------------------------------------

CRGB pattern1[] = {
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
    CRGB::Blue,
    CRGB::Blue,    
    CRGB::Blue,
    CRGB::Blue,    
    CRGB::Blue,
    CRGB::Blue,    
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
};

CRGB pattern2[] = {
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::White,
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
    CRGB::Purple,
    CRGB::Purple,    
    CRGB::Purple,
    CRGB::Purple,    
    CRGB::Purple,
    CRGB::Purple,    
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
};

CRGB pattern3[] = {
    CRGB::Cyan,
    CRGB::Cyan,
    CRGB::Cyan,
    CRGB::Cyan,
    CRGB::Cyan,
    CRGB::Cyan,
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
    CRGB::Orange,
    CRGB::Orange,    
    CRGB::Orange,
    CRGB::Orange,    
    CRGB::Orange,
    CRGB::Orange,    
    CRGB::Black,
    CRGB::Black,
    CRGB::Black,
};


float repeat_offset = 0;
float repeat_leds_per_second = 3.0;
  
void repeat(CRGB *pattern, int pattern_size) {
   repeat_offset = (float)millis() / 1000.0 * repeat_leds_per_second;
   for (int i=0; i<MAX_NUM_LEDS; i++) {
      float alpha = fmod(repeat_offset, 1) * 255.0;
      int offset1 = i + floor(repeat_offset);
      CRGB color1 = pattern[offset1 % pattern_size];
      CRGB color2 = pattern[(offset1 + 1) % pattern_size];
      leds[i] = interpolate(color1, color2, alpha);
   }
}

void repeat_strobe(CRGB *pattern, int pattern_size) {
  repeat(pattern, pattern_size);
  int v = map(sin16(millis() * 39), -32767, 32767, 30, 255);
  FastLED.setBrightness(v);
}

//---------------------------------------------------------------------------------------

int render_mode = 0;
int future_render_mode = 0;
int transition_mode = 0;
bool rendering = false;

unsigned int transition_start = 0;
bool transition = false;

enum RENDERING_MODE {
    NO_RENDERING = 0,
    BLUE_WHITE,
    BLUE_WHITE_STROBE,
    PURPLE_WHITE,
    PURPLE_WHITE_STROBE,
    CYAN_PINK,
    CYAN_PINK_STROBE,
    FIRE,
    BLUE_MAGIC,
    YELLOW_GREEN,
};

#define TRANSITION_TIME 750

void render() {

  FastLED.setBrightness(255);

  memset(leds, 0,  MAX_NUM_LEDS * sizeof(struct CRGB)); 
  switch (render_mode) {
    case BLUE_WHITE:
        repeat(pattern1, sizeof(pattern1) / sizeof(pattern1[0]));
        break;
    case BLUE_WHITE_STROBE:
        repeat_strobe(pattern1, sizeof(pattern1) / sizeof(pattern1[0]));
        break;

    case PURPLE_WHITE:
        repeat(pattern2, sizeof(pattern2) / sizeof(pattern2[0]));
        break;
    case PURPLE_WHITE_STROBE:
        repeat_strobe(pattern2, sizeof(pattern2) / sizeof(pattern2[0]));
        break;

    case CYAN_PINK:
        repeat(pattern3, sizeof(pattern3) / sizeof(pattern3[0]));
        break;
    case CYAN_PINK_STROBE:
        repeat_strobe(pattern3, sizeof(pattern3) / sizeof(pattern3[0]));
        break;

    case FIRE:
        uneven_fade(fades1[0]);
        break;
    case BLUE_MAGIC:
        uneven_fade(fades2[0]);
        break;
    case YELLOW_GREEN:
        uneven_fade(fades3[0]);
        break;
  }
  
  
  if (transition) {
      int d = millis() - transition_start;
      int v = map(d, 0, TRANSITION_TIME, 255, -255);
      if (v < 0) {
         render_mode = future_render_mode;
      }
      if (v <= -255) {
        transition = false;
        v = 255;
      }
      
      FastLED.setBrightness(abs(v));
  }
}

//---------------------------------------------------------------------------------------

enum PROTOCOL {
	PROTOCOL_ESCAPE = 0x13,
	PROTOCOL_START = 0x37,
};

enum STATE {
	STATE_WAITING_FOR_START = 0,
	STATE_WAITING_FOR_COMMAND,
	STATE_WAITING_FOR_LED_COUNT,
	STATE_LED_DATA,
        STATE_WAITING_FOR_COMMAND_DATA,
};

enum COMMAND {
	COMMAND_HELLO = 1,
	COMMAND_LEDS = 2,
        COMMAND_CLEAR = 3,
        COMMAND_RENDER = 4,
        COMMAND_TRANSITION = 5,
};

#define COMMAND_DATA_SIZE 512

#pragma pack(1)
struct RENDER_COMMAND {
    byte mode;
};

struct TRANSITION_COMMAND {
    byte mode;
};

// state
int state = STATE_WAITING_FOR_START;

byte input = 0;
byte prev_input = 0;

int led_count = 0;
int current_pos = 0;
int target_pos = 0;

int command;
void * command_data;
int command_current_pos;
int command_data_size;

void wait_for_command_data(int c, int data_size) {
    memset(command_data, 0, COMMAND_DATA_SIZE);
    command = c;
    command_current_pos = 0;
    command_data_size = data_size;
    state = STATE_WAITING_FOR_COMMAND_DATA;
}

void do_command(int c, void * data) {
    RENDER_COMMAND * render_data = (RENDER_COMMAND *) data;
    TRANSITION_COMMAND * transition_data = (TRANSITION_COMMAND *) data;
  
    switch (c) {
        case COMMAND_RENDER:
            rendering = true;
            render_mode = render_data->mode;
            break;
            
        case COMMAND_TRANSITION:
            rendering = true;
            future_render_mode = transition_data->mode;
            transition_start = millis();
            transition = true;
            break;
    }
}

void setup() {
      Serial.begin(115200);
      memset(leds, 0, sizeof(struct CRGB) * MAX_NUM_LEDS);
      FastLED.addLeds<UCS1903, 10, GRB>(leds, MAX_NUM_LEDS);  
//      FastLED.addLeds<WS2811, 10, GRB>(leds, MAX_NUM_LEDS);  
      FastLED.show();
      
      for (int i = 0; i < MAX_NUM_LEDS; i++) {
        fade_offsets[i]=random(FADE_LEN);
      }
     
     command_data = malloc(COMMAND_DATA_SIZE);
     
     Serial.println("Hello, world!");
     
     ((RENDER_COMMAND *)command_data)->mode = 1;
     do_command(COMMAND_RENDER, command_data);
}

//bool transitioned = false;

void loop() {

	while (Serial.available() > 0) {
		prev_input = input;
		input = Serial.read();

		// reset state machine if we need to
		if (PROTOCOL_ESCAPE == prev_input && PROTOCOL_START == input) {
			state = STATE_WAITING_FOR_COMMAND;
			continue;
		}

		// if we got the escape char, lets see what the next one will be
		if (PROTOCOL_ESCAPE != prev_input && PROTOCOL_ESCAPE == input) {
			continue;
		}

		// state machine
		switch (state) {
		case STATE_WAITING_FOR_START: break;
		case STATE_WAITING_FOR_COMMAND:
			switch (input) {
			case COMMAND_HELLO:
				Serial.println("Hello, world!");
				break;

			case COMMAND_LEDS:
				state = STATE_WAITING_FOR_LED_COUNT;
				memset(leds, 0,  MAX_NUM_LEDS * sizeof(struct CRGB)); 
				break;
                        
                        case COMMAND_CLEAR:
				memset(leds, 0,  MAX_NUM_LEDS * sizeof(struct CRGB)); 
				FastLED.show();
                                break;
                       
                        case COMMAND_RENDER:
                              wait_for_command_data(COMMAND_RENDER, sizeof(struct RENDER_COMMAND));
                              break;

                        case COMMAND_TRANSITION:
                              wait_for_command_data(COMMAND_TRANSITION, sizeof(struct TRANSITION_COMMAND));
                              break;

                        }
                          
			break;

                case STATE_WAITING_FOR_COMMAND_DATA:
                        ((byte*)command_data)[command_current_pos] = input;
                        command_current_pos++;
                        if (command_current_pos == command_data_size) {
                            do_command(command, command_data);
                            state = STATE_WAITING_FOR_START;                        
                        }
                        break;
                
                case STATE_WAITING_FOR_LED_COUNT:
			led_count = min(input, MAX_NUM_LEDS);
			current_pos = 0;
			target_pos = led_count * sizeof(struct CRGB);
			state = STATE_LED_DATA;
			break;

		case STATE_LED_DATA:
			((byte *)leds)[current_pos] = input;
			current_pos++;

			// show the LEDs if we reached the end of the data
			if (current_pos == target_pos) {
                              rendering = false;
			      FastLED.show();  
		              state = STATE_WAITING_FOR_START;
			}
			break;
		}
	}


    //int rmode = ((millis() / 1000 / 5) % 9) + 1;
    //((RENDER_COMMAND *)command_data)->mode = rmode;
    //do_command(COMMAND_RENDER, command_data);

    /*if (millis() / 1000 > 5 && !transitioned) {
        ((TRANSITION_COMMAND *)command_data)->mode = 5;
        do_command(COMMAND_TRANSITION, command_data);
        transitioned = true;
    }*/

    if (rendering) {
       render();
      FastLED.show();  
    }
}
