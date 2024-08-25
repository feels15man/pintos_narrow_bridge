
/* File for 'narrow_bridge' task implementation.  
   SPbSTU, IBKS, 2017 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "narrow-bridge.h"

#define MAX_VEHICLES_ON_BRIDGE 2

int direction = -1;
int on_bridge = 0; // number of vehicles on bridge
int waiting[2][2] = {{0, 0}, {0, 0}}; // vehicles waiting[direction][priority]
struct lock bridge_lock; //lock controlling vars
struct semaphore wait_lock[2][2]; //semaphores control crossing [direction][priority]


// Called before test. Can initialize some synchronization objects.
void narrow_bridge_init(void)
{
	 lock_init(&bridge_lock); //still lock

	 // semas
    sema_init(&wait_lock[dir_left][car_normal], 0); // no initial use
    sema_init(&wait_lock[dir_left][car_emergency], 0);
    sema_init(&wait_lock[dir_right][car_normal], 0);
    sema_init(&wait_lock[dir_right][car_emergency], 0);
}

void arrive_bridge(enum car_priority prio UNUSED, enum car_direction dir UNUSED)
{
   lock_acquire(&bridge_lock);
   waiting[dir][prio]++;
   //set up an initial direction depanding on first arrived car
	if(direction == -1)
		direction = dir;
	//can we go on
   if(direction == dir && on_bridge < MAX_VEHICLES_ON_BRIDGE)
   {	//sem.value++ for our priority
		if(prio == car_emergency)
			sema_up(&wait_lock[dir][car_emergency]);
		else if(!waiting[direction][car_emergency] && waiting[dir][car_normal])
			sema_up(&wait_lock[dir][car_normal]);
   }
   lock_release(&bridge_lock);//let other to come in
   sema_down(&wait_lock[dir][prio]);//take our turn
   on_bridge++;
   waiting[dir][prio]--;
}

void exit_bridge(enum car_priority prio UNUSED, enum car_direction dir UNUSED)
{
	lock_acquire(&bridge_lock);
	on_bridge--;
	//we were last car
	if(!on_bridge)
	{
	 	//check direction

	 	if((waiting[1 - direction][car_emergency] >= waiting[direction][car_emergency] && 
	 		 	 waiting[1 - direction][car_emergency]) ||
	 		(!waiting[direction][car_emergency] &&
	 		 waiting[1 - direction][car_normal] > waiting[direction][car_normal]))
	 		direction = 1 - direction;

	 	//wake up who waits
	 	int to_wake_up = MAX_VEHICLES_ON_BRIDGE - on_bridge; //how much need to wake up
	 	for (int i = 1; i <= waiting[direction][car_emergency] && to_wake_up > 0; i++)
	   {
	   	sema_up(&wait_lock[direction][car_emergency]);
	   	to_wake_up--;
	   }

   	for (int i = 1; i <= waiting[direction][car_normal] && to_wake_up > 0; i++)
		{
		  	sema_up(&wait_lock[direction][car_normal]);
			to_wake_up--;
		}
	}
   lock_release(&bridge_lock);
}
