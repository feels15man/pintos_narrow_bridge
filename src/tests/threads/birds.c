/*
  File for 'birds' task implementation.
*/

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/timer.h"

struct lock lock_dish;
unsigned int portions_last;
unsigned int f;
struct semaphore sema_bird, sema_chick;

static void init(unsigned int dish_size UNUSED)
{ 
  f = dish_size; //portions to full a dish
  portions_last = dish_size;

  sema_init(&sema_bird, 0);
  sema_init(&sema_chick, 1);

  lock_init(&lock_dish);
}

static void bird(void* arg UNUSED)
{
    msg("bird created.");

    while(1)
    {
      sema_down(&sema_bird);
      lock_acquire(&lock_dish);
      msg("Bird waked up and putted %d portions to dish", f);
      // if(!portions_last)
      portions_last = f;
      sema_up(&sema_chick);
      lock_release(&lock_dish);
    }
}

static void chick(void* arg)
{
  msg("chick %d created.", (int) arg);

  while(1)
  {
    sema_down(&sema_chick);
    lock_acquire(&lock_dish);
    portions_last--;//if f == 0 there is nothing to eat and fulfill with the dish
    msg("chick %d ate one portion", (int) arg);
    if(!portions_last)
      sema_up(&sema_bird);
    else
     sema_up(&sema_chick);
   
    lock_release(&lock_dish);
    timer_sleep(10);
  }
}


void test_birds(unsigned int num_chicks, unsigned int dish_size)
{
  unsigned int i;
  init(dish_size);

  thread_create("bird", PRI_DEFAULT, &bird, NULL);

  for(i = 0; i < num_chicks; i++)
  {
    char name[32];
    snprintf(name, sizeof(name), "chick_%d", i + 1);
    thread_create(name, PRI_DEFAULT, &chick, (void*) (i+1) );
  }

  timer_msleep(5000);
  pass();    
}
