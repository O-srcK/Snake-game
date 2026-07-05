#include <FastLED.h>

#define DATA_PIN 6
#define NUM_LEDS 64
#define BRIGHTNESS 15
#define PI 3.1415926535897932384626433832795

////////////////////////////////

unsigned long lastStepTime = 0;
unsigned long StepDelay = 3000;
bool firstStep = true;
const uint8_t ANIM_FRAMES = 16;
const unsigned long animDelay = 80;
unsigned long lastAnimTime = 0;
uint8_t current_frame = 0;

const static int BOARD_SIZE = 8;

////////////////////////////////

enum SnakeColor{ REGULAR, GRADIENT, STRIPED};
enum GameState { PLAYING, WIN_ANIM, LOSE_ANIM };
enum Direction { DOWN, UP, LEFT, RIGHT };

SnakeColor mode = STRIPED;
GameState state = PLAYING;
Direction dir = RIGHT;

CRGB leds[NUM_LEDS];

class Snake;
class Pixel;

struct Body_segment {
	int x, y;
	int position_id; // = x + y * BOARD_SIZE
	Body_segment* next;
	Body_segment* prev;

	Body_segment(int x, int y):next(nullptr),prev(nullptr) {//constructor for a body segment
		this->x = x;
		this->y = y;
		position_id = x + y * BOARD_SIZE;
	}
};

uint16_t XY(uint8_t x, uint8_t y) {
	return x + 8 * y;
}

////////////////////////////////

class Pixel {
private:
	int x, y;           // Position of the pixel in the matrix
	int index;           // Attribute (int variable)
	
public:             // Access specifier
	Pixel() { //default constructor
		x = 0;
		y = 0;
		index = 0;
	}

	Pixel(int x, int y) {
		this->x = x;
		this->y = y;
		index = x + y * BOARD_SIZE;
	}

	int get_x () {return x;}
	int get_y () {return y;}
	int get_index () {return index;}

	void setColorRGB(uint8_t r, uint8_t g, uint8_t b) {
		leds[XY(x, y)] = CRGB(r, g, b);
	}

	void setColor(CRGB color) {
		leds[XY(x, y)] = color;
	}

	void setColorHSV(uint8_t h, uint8_t s, uint8_t v) {
		leds[XY(x, y)] = CHSV(h, s, v);
	}
};

class Matrix {
private:
	Pixel pixels[BOARD_SIZE][BOARD_SIZE]; //y, x

public:
	Matrix() { //constructor to intitialize it
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				pixels[y][x] = Pixel(x, y);
				Pixel& pixel = pixels[y][x];
				pixel.setColorRGB(0, 0, 0); // Set all pixels to black
			}
		}
	};

	Pixel& getPixel(int x, int y) {
		return pixels[y][x];
	}
};

////////////////////////////////

class Apple {
public:
	int x, y;
	int position_id; //x + y*BOARD_SIZE
	Matrix& matrix_ref;

	//constructor to spawn the apple
	Apple(Matrix& m) : matrix_ref(m) {
		int random_index = random(BOARD_SIZE * BOARD_SIZE);
		position_id =random_index;
		x = position_id % BOARD_SIZE;
		y = position_id / BOARD_SIZE;
	}

	int get_pos() { return position_id; }

	void spawn_apple(Snake& snake);

	void color_apple();
};

////////////////////////////////

class Snake {
private:
	int snake_size = 1;
	Matrix& matrix;

	Body_segment* head;
	Body_segment* tail;    

public:
	Snake(Matrix& m): matrix(m)  {
		spawn_snake_head_random();
	}

	~Snake() {
		clear();
	}

	int get_head_pos() 		{	return head->position_id;}
	int get_size() 			{	return snake_size;}
	Body_segment* get_head(){	return head;}
	Body_segment* get_tail(){	return tail;}

	bool hit_wall(Body_segment* future_head) {
		int x = future_head->x;
		int y = future_head->y;
		if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
			return true;
		}
		return false;
	}
	
	bool self_bite(Body_segment* future_head, bool growing) {
		Body_segment* current = head;

		while (current != nullptr) {
			if (!growing && current == tail) {
				break;   // skip tail because it will move away
			}

			if (current->position_id == future_head->position_id) {
				return true;
			}

			current = current->prev;   // head -> ... -> tail
		}

		return false;
	}

	void clear() {
		Body_segment* current = tail;
		while (current != nullptr) {
			Body_segment* next = current->next;
			delete current;
			current = next;
		}
		head = nullptr;
		tail = nullptr;
		snake_size = 0;
	}

	void spawn_snake_head_random() {
		int random_index = random(BOARD_SIZE * BOARD_SIZE);
		int x = random_index % BOARD_SIZE;
		int y = random_index / BOARD_SIZE;
		head = tail = new Body_segment(x, y);

		snake_size = 1;
	}

	void remove_tail() {
		if (snake_size > 1) {
			Body_segment* old_tail = tail;
			Body_segment* new_tail = tail->next;

			delete old_tail;
			tail = new_tail;
			tail->prev = nullptr;
			snake_size --;
		}
	}

	void add_head(Body_segment* new_head) {
		head->next = new_head;
		new_head->prev = head;
		head = new_head;
		snake_size ++;
	}

	Body_segment* predict_head(Direction dir) {
		int dx = 0, dy = 0;
		switch (dir) 
		{
		case DOWN:
				dy = 1;
				break;
		case UP:
				dy = -1;
				break;
		case LEFT:
				dx = -1;
				break;
		case RIGHT:
				dx = 1;
				break;
		}
		int future_head_x = head->x + dx;
		int future_head_y = head->y + dy;

		Body_segment* future_head = new Body_segment(future_head_x, future_head_y);
		return future_head;
	} 

	void reset_snake() {
		clear();
		spawn_snake_head_random();
	}

	void set_segment_color(Body_segment* segment, CRGB color) {
		int x = segment->x;
		int y = segment->y;
		Pixel& segment_pixel = matrix.getPixel(x, y);
		// matrix.setDisplay(segment_pixel, val);
		segment_pixel.setColor(color);  // Set snake color 
	}

	CRGB calculate_segment_color(int index) {

		const uint8_t head_hue = 96;   // green hue
		const uint8_t sat     = 255;
		const uint8_t val     = 255;

		if (mode == GRADIENT) {
			return CHSV(head_hue + (index*8), sat, val);

		} else if (mode == STRIPED) {
			CRGB first_color = CRGB(237, 112, 90); //Light Pink
			CRGB second_color = CRGB(230, 53, 100); //Bright Pink
			if (index % 4 <= 1 ) {	return first_color;} 
			else {					return second_color;}

		} else { // mode == REGULAR
			return CHSV(head_hue, sat, val);
		}
	}

	void color_snake() {
		Body_segment* current = head;
		int index = 0;
		while (current != nullptr) {
			matrix.getPixel(current->x, current->y).setColor(calculate_segment_color(index));
			current = current->prev;
			index++;
		}
	}

};

////////////////////////////////

void Apple::spawn_apple(Snake& snake) {
	int random_index = random(BOARD_SIZE * BOARD_SIZE);
	Body_segment* current = snake.get_head();

	while (current != nullptr) {
		if (current->position_id == random_index) {
			random_index = random(BOARD_SIZE * BOARD_SIZE);
			current = snake.get_head();
			continue;
		}
		current = current->prev;
	}

	position_id = random_index;
	x = random_index % BOARD_SIZE;
	y = random_index / BOARD_SIZE;
}

void Apple::color_apple() {
	Pixel& apple_pixel = matrix_ref.getPixel(x, y);
	apple_pixel.setColor(CRGB::Red); // Set apple color to red
}

////////////////////////////////

class Game {
private:
	bool game_WON = false;
	bool game_LOST = false;
	Matrix& matrix;
	Snake& snake;
	Apple& apple;

public:
	Game(Matrix& m, Snake& s, Apple& a): matrix(m), snake(s), apple(a) {}

	void clear_frame() {
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Pixel& pixel = matrix.getPixel(x, y);
				pixel.setColor(CRGB::Black); // Set all pixels to black
			}
		}
	}

	void show_frame() {
		FastLED.show();
	}

	void drawframe() {
		clear_frame();
		apple.color_apple();
		snake.color_snake();
		show_frame();
	}

	void update(Direction dir, GameState& state) {
		Body_segment* possible_head = snake.predict_head(dir);

		bool growing = (possible_head->position_id == apple.get_pos());//if it ate the apple

		game_LOST = (snake.hit_wall(possible_head) || snake.self_bite(possible_head, growing));

		if (game_LOST)   { 
			state = LOSE_ANIM; 
			delete possible_head; 
			return;
		}
		else snake.add_head(possible_head);

		if (growing) {
			game_WON = (snake.get_size() == BOARD_SIZE*BOARD_SIZE);

			if (game_WON) { 
				state = WIN_ANIM;
				return;
			}  
			else apple.spawn_apple(snake);

		} else { 
			snake.remove_tail(); 
		}
	}

	void restart() {
		// Reset game state
		game_WON = false;
		game_LOST = false;
		state = PLAYING;
		current_frame = 0;
		lastAnimTime = 0;
		lastStepTime = 0;

		// Clear the matrix
		clear_frame();

		// Reset snake position
		snake.reset_snake();
		apple.spawn_apple(snake);

		drawframe(); // Draw the initial frame after restart
	};

	void setup() {
		apple.spawn_apple(snake);
	}

	void drawWinFrame(uint8_t frame_nr) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				color_win_pixel(x, y, frame_nr);
			}
		}
		show_frame();
	};

	void drawLoseFrame(uint8_t frame_nr) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				color_lose_pixel(x, y, frame_nr);
			}
		}
		show_frame();
	};


	float clamp(float v) { 
		return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v; 
	};

	void color_win_pixel(int x, int y, int frame) {
		constexpr float CX = 3.5f;

		float dx = x - CX;
		float dy = y - CX;
		float d  = hypotf(dx, dy);
		float th = atan2f(fabsf(dy), fabsf(dx));   // folded → 4-fold symmetry

		float phi = 8.0f * th  - PI * frame / 8.0f;    // star phase
		float psi = 1.6f * d   - PI * frame / 4.0f;    // ring phase


		float value_f = clamp(0.75f + 0.225f*cosf(psi) + 0.175f*cosf(phi));
		float sat_f   = clamp(0.62f + 0.28f *cosf(phi));   
		float hue_f   = 35.0f * d + 22.5f * frame;

		uint8_t h = (uint8_t)hue_f;
		uint8_t s = (uint8_t)(sat_f * 255.0f);
		uint8_t v = (uint8_t)(value_f * 255.0f);

		Pixel& pixel = matrix.getPixel(x, y);
		pixel.setColorHSV(h, s, v);
	};

	void color_lose_pixel(int x, int y, int frame) {
		constexpr float CX   = 3.5f;
		constexpr float MAXD = 4.9497f;   // hypotf(3.5, 3.5)

		float dx = x - CX;
		float dy = y - CX;
		float d  = hypotf(dx, dy);

		float value = clamp(0.575f
						+ 0.175f * cosf(PI * frame / 8.0f)
						+ 0.150f * cosf(1.3f * d - PI * frame / 8.0f)
						+ 0.060f * sinf(3.0f * d + PI * frame / 2.0f)
						- 0.300f * d / MAXD);

		float tint = (0.5f + 0.5f * cosf(0.9f * d - PI * frame / 8.0f)) * 0.4f;

		uint8_t r = (uint8_t)roundf(10.0f + (140.0f - 35.2f * tint) * value);
		uint8_t g = (uint8_t)roundf( 3.0f +  15.0f                  * value);
		uint8_t b = (uint8_t)roundf( 3.0f + ( 11.0f + 50.4f * tint) * value);

		Pixel& pixel = matrix.getPixel(x, y);
		pixel.setColorRGB(r, g, b);
	}
};

////////////////////////////////

void handleSerial( Game& game) {
    if (Serial.available() <= 0) return;

    char c = Serial.read();

    switch (c) {
        case 'r': case 'R':
            game.restart();
            dir = LEFT;
            firstStep = true;
            StepDelay = 3000;
            lastStepTime = millis();
            break;

        case 'w': case 'W':
            if (state == PLAYING && dir != DOWN) dir = UP;
            break;
        case 's': case 'S':
            if (state == PLAYING && dir != UP) dir = DOWN;
            break;
        case 'a': case 'A':
            if (state == PLAYING && dir != RIGHT) dir = LEFT;
            break;
        case 'd': case 'D':
            if (state == PLAYING && dir != LEFT) dir = RIGHT;
            break;
    }
};

////////////////////////////////

Matrix matrix;
Apple apple(matrix);

Snake snake(matrix); //x, y
Game game(matrix, snake, apple);

void setup() {
	FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(BRIGHTNESS);
	FastLED.clear();
	Serial.begin(9600);
	randomSeed(analogRead(A0));

	game.setup();
	game.drawframe();
}

void loop() {
	handleSerial(game);
	
	if (state == PLAYING) {
		if (millis() - lastStepTime >= StepDelay) {
			lastStepTime = millis();
			game.update(dir, state);
			game.drawframe();

			if (firstStep) {
				StepDelay = 500;
				firstStep = false;
			}

		} 
	} else if (state == WIN_ANIM) {
		if (millis() - lastAnimTime >= animDelay) {
			lastAnimTime = millis();
			game.drawWinFrame(current_frame);
			current_frame++;

			if (current_frame >= ANIM_FRAMES) {current_frame = 0;}
		}
	} else if (state == LOSE_ANIM) {
		if (millis() - lastAnimTime >= animDelay) {
			lastAnimTime = millis();
			game.drawLoseFrame(current_frame);
			current_frame++;

			if (current_frame >= ANIM_FRAMES) {current_frame = 0;}
		}
	}
	
}
