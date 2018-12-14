#include <gb/gb.h>
#include <stdio.h>
#include "tiles.c"
#include "sprites.c"
#include "blank.c"
#include "dungeon.c"
#include "wizard.c"
#include "attack.c" //typedef 1
#include "attacks.c"
#include "mob.c"    //typedef 2
#include "powerup.c"//typedef 3
#include "port.c"   //typedef 4
#include "trap.c"   //typedef 5


void init();
void checkInput();
void updateSwitches();
void ai();
UINT8 player[2];
Attack inventory[5];

float enemy[2];
int facing = 1;
UINT32 time = 0;
Attack projectiles[6];
int i;
int response = 1;
void (*spell)();
void collision();
int distance(int, int);
enum State{BEGIN = 0, PLAY = 1, PAUSE = 2, GAME_OVER = 3} state;
void screen_blank();
int lives = 3;
void moveProjectiles();
Port portkey[5];
Powerup powerup[5];
Trap traps[5];
void(*equipped)();
void doubleAttack();

typedef struct {
	int alliance;
	int direction;
	int id;
	int x;
	int y;
	int active;
} fireball;

void main() {

	init();
	state = BEGIN;
	(*equipped) = defaultAttack;
	while(1) {
		updateSwitches();			// Make sure the SHOW_SPRITES and SHOW_BKG switches are on each loop
		wait_vbl_done();			// Wait until VBLANK to avoid corrupting memory
		switch(state){
			case BEGIN:
				screen_blank();
				printf("The Senile Wizard");
				checkInput();
				break;
			case PLAY:
				checkInput();
				moveProjectiles();
				ai();
				collision();
				time++;
				break;
			case PAUSE:
				checkInput();
				break;
			case GAME_OVER:
				screen_blank();
				printf("You lasted %d seconds!", time);
				checkInput();
				break;
		}
	}
	
}

//Display initializer taken from tutorial
void init() {
	
	DISPLAY_ON;						// Turn on the display
	set_bkg_data(0, 23, tiles);		// Load 23 tiles into background memory
	
	set_bkg_tiles(0,0,20,18,dungeon); 
	
	SPRITES_8x16;
	// Load the the 'sprites' tiles into sprite memory
	set_sprite_data(0, 32, wizard);
	
	// Set the first movable sprite (0) to be the first tile in the sprite memory (0)
	set_sprite_tile(0,0);
	//second half of sprite
	set_sprite_tile(1, 2);
	//move_sprite(1, 83, 75);
	
	//fireball sprite
	set_sprite_data(33,3, attacks);
	set_sprite_tile(4, 8);
	
	player[0] = 64;
	player[1] = 64;
	
	//enemy sprites
	//frog
	set_sprite_tile(2,6);
	
	enemy[0] = 128;
	enemy[1] = 128;
	move_sprite(2, 128, 128);
	SHOW_SPRITES;

}

//Taken from tutorial
void updateSwitches() {
	
	HIDE_WIN;
	SHOW_SPRITES;
	SHOW_BKG;
	
}

void defaultAttack(int x, int y){
	move_sprite(4, x, y);
	for(i = 0; i < 5; i++){
			if(projectiles[i].active == 0) break;
	}
	if(i >= 5) return;
	fireball newAttack;
	projectiles[i] = fireball newAttack;
	projectiles[i].alliance = 1;
	projectiles[1].id = 100 + i;
	projectiles[i].x = x;
	projectiles[i].y = y;
	if(facing){
		set_sprite_prop(4, get_sprite_prop(4) & ~S_FLIPX);
		projectiles[i].direction = 1;
	}
	else{
		set_sprite_prop(4, get_sprite_prop(4) | S_FLIPX);
		projectiles[i].direction = -1;
	}
	
}



void checkInput(){

    if(joypad() & J_B){
		if(facing) attack(player[0] + 16, player[1]+2);
		else attack(player[0]-8, player[1]+2);
    }
	
	// UP
	if(joypad() & J_UP && player[1] > 32){
		player[1]--;
	}
	
	// DOWN
	if(joypad() & J_DOWN && player[1] < 255 - 127 ){
		player[1]++;
	}
	// LEFT
	if(joypad() & J_LEFT && player[0] > 23){
		player[0]--;
		facing = 0;
	}
	// RIGHT
	if(joypad() & J_RIGHT && player[0] < 255 - 118){ 
		player[0]++;
		facing = 1;
	}
	
	if(joypad() & J_START){
		switch(state){
			case BEGIN:
				set_bkg_data(0, 23, tiles);
				set_bkg_tiles(0,0,20,18,dungeon); 
				state = PLAY;
				break;
			case PLAY:
				printf("PAUSE");
				state = PAUSE;
				break;
			case PAUSE:
				set_bkg_data(0, 23, tiles);
				set_bkg_tiles(0,0,20,18,dungeon); 
				state = PLAY;
				break;
			case GAME_OVER:
				state = BEGIN;
				lives = 3;
				break;
		}
	}
	
	if(facing){
		//set_sprite_tile(0,0);
		//set_sprite_tile(1,2);
		set_sprite_prop(0, get_sprite_prop(0) & ~S_FLIPX );
		set_sprite_prop(1, get_sprite_prop(1) & ~S_FLIPX );
		set_sprite_tile(0, 0);
		set_sprite_tile(1, 2);
	}
	else{
		//set_sprite_tile(0,4);
		//set_sprite_tile(1,6);
		set_sprite_prop(0, get_sprite_prop(0) | S_FLIPX );
		set_sprite_prop(1, get_sprite_prop(1) | S_FLIPX );
		set_sprite_tile(0, 2);
		set_sprite_tile(1, 0);
	}
	move_sprite(0, player[0], player[1]);
	move_sprite(1, player[0] + 8, player[1]);
}

void moveProjectiles(){
		for(i = 0; i < 5; i++){
			if(projectiles[i].active == 1){
				projectiles[i].x += projectiles[i].xDirection;
				projectiles[i].y += projectiles[i].yDirection;
				if(projectiles[i].x > 130 || projectiles[i].x < 0 || projectiles[i].y > 130 || projectiles[i].y < 0 ){
					projectiles[i].active = 0;
				}
			}
			if(projectiles[i+1].active == 1 && projectiles[i].active == 0){
				projectiles[i] = projectiles[i+1];
				projectiles[i+1].active = 0;
			}
		}
}

void aiChase(){
	//if((abs(player[0] - enemy[0]) < 10) && (abs(player[1] - enemy[1]) < 10)){
	if(time & 16){
		if(player[0] > enemy[0]) enemy[0]++;
		else if(player[0] < enemy[0]) enemy[0]--;
		
		if(player[1] > enemy[1]) enemy[1]++;
		else if(player[1] < enemy[1]) enemy[1]--;
		
		move_sprite(2, enemy[0], enemy[1]);
	}
	//}
}

void ai(){
	if(distance(enemy[1], player[1]) < 20 && distance(enemy[0], player[0]) < 20) aiChase();
	else{
		enemy[1]++;
		delay(1000);
		move_sprite(2, enemy[0], enemy[1]);
	}
}

void collision(){
	
	//enemy check
	if(distance(player[0], enemy[0]) < 5 && distance(player[1], enemy[1]) < 5){
		if(lives > 1) lives--;
		else state = GAME_OVER; //man, game over!
	}
	
	//trap check
	for(i = 0; i < 5; i++){
		if(distance(player[0], trap[i].x) < 5 && distance(player[1], trap[i].y) < 5 && (time % trap[i].cycle % 2)){
			if(lives > 1){
				lives--;
				trap[i].x = 200;
				trap[i].y = 200;
			}
			else state = GAME_OVER; //man, game over!
		}
	}
	
	//portkey check
	for(i = 0; i < 4; i++){
		if(distance(player[0], portkey[i].x) < 5 && distance(player[1], portkey[1].y) < 5){
			player[0] = portkey[i].destinationX;
			player[1] = portkey[i].destinationY;
			portkey[i].x = 200;
			portkey[i].y = 200;
			break;
		}
	}
	
	//powerup check
	for(i = 0; i < 4; i++){
		if(distance(player[0], powerup[i].x) < 5 && distance(player[1], powerup[i].y) < 5){
			if(powerup[i].ID == 0)lives ++;
			else if(powerup.ID == 1) (*equipped) = doubleAttack; //FPointer
			powerup[i].x = 200;
			powerup[i].y = 200;
			break;
		}
	}
}

void defaultAttack(int x, int y){
	move_sprite(4, x, y);
	for(i = 0; i < 5; i++){
			if(projectiles[i].active == 0) break;
	}
	if(i >= 5) return;
	fireball newAttack;
	projectiles[i] = fireball newAttack;
	projectiles[i].alliance = 1;
	projectiles[1].id = 100 + i;
	projectiles[i].x = x;
	projectiles[i].y = y;
	if(facing){
		set_sprite_prop(4, get_sprite_prop(4) & ~S_FLIPX);
		projectiles[i].direction = 1;
	}
	else{
		set_sprite_prop(4, get_sprite_prop(4) | S_FLIPX);
		projectiles[i].direction = -1;
	}
	
}


int distance(int a, int b){
	if(a == b) return 0;
	else if(a < b) return b - a;
	else return a - b;
}


int move(){
	if(x >=137 || x<= 23){
		move_sprite(id, 200, 200);
		return 0;
	}
	else move_sprite(id, x + direction, y);
	return 1;
}

void screen_blank(){
	set_bkg_tiles(0,0,20,18,blank);
}