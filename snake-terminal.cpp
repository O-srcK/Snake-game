#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
using namespace std;

class Snake;

const static int BOARD_SIZE = 8;
enum Direction { 
    DOWN, 
    UP, 
    LEFT, 
    RIGHT 
};

////////////////////////////////

class Pixel {
    private:
    int x, y;           // Position of the pixel in the matrix
    int index;           // Attribute (int variable)
    
    public:             // Access specifier
        int color[3];        // GRB [Green, Red, Blue]
        string display;

        Pixel() { //default constructor
            x = 0;
            y = 0;
            index = 0;
            color[0] = 0; //green
            color[1] = 0; //red
            color[2] = 0; //blue
            display = " ";
        }

        Pixel(int x, int y) {
            this->x = x;
            this->y = y;
            index = x + y * BOARD_SIZE;
            color[0] = 0; //green
            color[1] = 0; //red
            color[2] = 0; //blue
            display = ". ";
        }

        int get_x () {
            return x;
        }
        int get_y () {
            return y;
        }
        int get_index () {
            return index;
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
            }
        }
    }

    void drawFrame() {
        for (int y = 0; y < BOARD_SIZE; y++) {
            for (int x = 0; x < BOARD_SIZE; x++) {
                cout << pixels[y][x].display << " ";
            }
            cout << endl;
        }
    }

    Pixel& getPixel(int x, int y) {
        return pixels[y][x];
    }

    void setDisplay(Pixel& pixel, string display) {
        pixel.display = display;
    }

    void setColor(Pixel& pixel, int r, int g, int b) {
        pixel.color[0] = g;
        pixel.color[1] = r;
        pixel.color[2] = b;
    }
};

////////////////////////////////

struct Body_segment {
    int x, y;
    int position_id; // = x + y * BOARD_SIZE
    Body_segment* next;
    Body_segment* prev;

    Body_segment(int x, int y) //constructor for a body segment
    : next(nullptr), prev(nullptr) {
        this->x = x;
        this->y = y;
        position_id = x + y * BOARD_SIZE;
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
        int random_index = rand() % (BOARD_SIZE * BOARD_SIZE);
        position_id =random_index;
        x = position_id % BOARD_SIZE;
        y = position_id / BOARD_SIZE;
    }

    int get_pos() { return position_id; }

    void spawn_apple(Snake& snake);
};

////////////////////////////////

class Snake {
    private:
        int snake_size = 1;
        Matrix& matrix;

        Body_segment* head;
        Body_segment* tail;    

    public:
        Snake(int x, int y, Matrix& m): matrix(m)  {
            head = tail = new Body_segment(x, y);
        }

        int get_head_pos() {
            return head->position_id;
        }

        int get_size() {
            return snake_size;
        }
        Body_segment* get_head() {
            return head;
        }
        Body_segment* get_tail() {
            return tail;
        }

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

        void set_segment_display(Body_segment* segment, string val) {
            int x = segment->x;
            int y = segment->y;
            Pixel& segment_pixel = matrix.getPixel(x, y);
            matrix.setDisplay(segment_pixel, val);
        }

        void draw_initial_snake() {
            set_segment_display(head, "S ");
        }

        void remove_tail() {
            if (snake_size > 1) {
                Body_segment* old_tail = tail;
                Body_segment* new_tail = tail->next;
    
                set_segment_display(old_tail, ". ");
                set_segment_display(new_tail, "* ");
                delete old_tail;
                tail = new_tail;
                tail->prev = nullptr;
                snake_size --;
            }
        }

        void add_head(Body_segment* new_head) {
            Body_segment* old_head = head;

            set_segment_display(old_head, "s ");
            set_segment_display(new_head, "S ");
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
};

////////////////////////////////

void Apple::spawn_apple(Snake& snake) {
    int random_index = rand() % (BOARD_SIZE * BOARD_SIZE);
    Body_segment* current = snake.get_head();

    while (current != nullptr) {
        if (current->position_id == random_index) {
            random_index = rand() % (BOARD_SIZE * BOARD_SIZE);
            current = snake.get_head();
            continue;
        }
        current = current->prev;
    }

    position_id = random_index;
    x = random_index % BOARD_SIZE;
    y = random_index / BOARD_SIZE;

    Pixel& apple_pixel = matrix_ref.getPixel(x, y);
    matrix_ref.setDisplay(apple_pixel, "A ");
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

        void drawframe() {
            for (int y = 0; y < BOARD_SIZE; y++) {
                for (int x = 0; x < BOARD_SIZE; x++) {
                    Pixel& pixel = matrix.getPixel(x, y);
                    cout << pixel.display << " ";
                    }
                cout<<endl;
            }
        }

        void update(Direction dir) {
            Body_segment* possible_head = snake.predict_head(dir);

            bool growing = (possible_head->position_id == apple.get_pos());//if it ate the apple

            game_LOST = (snake.hit_wall(possible_head) || snake.self_bite(possible_head, growing));

            if (game_LOST)   { gameLOST();}
            else snake.add_head(possible_head);

            if (growing) {
                game_WON = (snake.get_size() == BOARD_SIZE*BOARD_SIZE);

                if (game_WON) { gameWON(); }  
                else apple.spawn_apple(snake);

            } else { 
                snake.remove_tail(); 
            }

        }

        void gameLOST() {
            cout<<"You lost the game";
            exit(1);
        };
        void gameWON() {
            cout<<"You Won the game";
            exit(1);
        };

        void setup() {
            snake.draw_initial_snake();
            apple.spawn_apple(snake);
        }
};

////////////////////////////////

void readInput(Direction& dir) {
    if (_kbhit()) {
        switch (_getch()) {
            case 'w': case 'W': if (dir != DOWN) dir = UP; break;
            case 's': case 'S': if (dir != UP) dir = DOWN; break;
            case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
            case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
        }
    }
}

int main() {
    Direction dir = LEFT;

    Matrix matrix;
    Apple apple(matrix);

    Snake snake(5, 2, matrix); //x, y
    Game game(matrix, snake, apple);

    game.setup();
    game.drawframe();


    while (true) {
        readInput(dir);
        game.update(dir);

        system("cls");   // Windows only
        game.drawframe();
        Sleep(800);
    }

    return 0;
}
