#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include<pthread.h>
#include <stdbool.h>
#include "my_malloc.h"

//pthread_rwlock_t  lock_ts = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t lock_sbrk;


void printfreeList(){
  block * b=head;
  printf("free in list:\n");
  while(b!=NULL){
    printf("free.size %d\n",(int)(b->size));
    b=b->next;
  }
} 

void * get_data(block * b){
  block *temp=b;
  temp++;
  return temp;
}

void *bf_malloc(size_t size){
  size=align_size(size);
  block * new_block;
  block * min=NULL;
  block * prev_min=NULL;
  block *prev_curr=NULL;
  block *curr=head;
   while((curr!=NULL)){
   // if((curr->size>=size)&&(curr->is_used==0)){
     if(curr->size>size){
         if(min==NULL){
          prev_min=prev_curr;
          min=curr;
        }
        else{
            if(curr->size<min->size){
              prev_min=prev_curr;
              min=curr;
              }
        }
    }
    else if(curr->size==size){
      prev_min=prev_curr; 
      min=curr;
      break;
    }
    prev_curr=curr;
    curr=curr->next;
  }
  if(min==NULL){
    new_block=creat_block(size);
    if (new_block==NULL){
      return NULL;
    }
  }
  else if((min->size)>(sizeof(block)+size)){
    new_block=split_block(min,size);
  }
  else {
    new_block=min;
    if(min!=head)
      prev_min->next=min->next;
    else
      head=min->next;
    min->next=NULL;
  }

  //  printf("malloc\n");
  // printfreeList();
  return get_data(new_block);
}


size_t align_size(size_t size){
  if(size%8==0)
    return size;
  else{
    return ((size_t)(size/8)+1)*8;
    }
}

block *creat_block(size_t size){
  block *new_block=sbrk(0);
  if(sbrk(sizeof(block)+size)==(void *)-1)
    return NULL;
  //block * block_end=sbrk(0);
  new_block->size=size;
  new_block->next=NULL;
  return new_block;    
}

block *split_block(block *curr,size_t size){
  block *new_block=NULL;
    curr->size-=size+sizeof(block);
    new_block= (block *)((size_t)curr + curr->size + sizeof(block));
    new_block->size=size;
    new_block->next=NULL;
  return new_block;
}

/*  printf("malloc\n");
  printList();
  printfreeList();
*/

block * is_valid(void *address){
  if(address<=sbrk(0)&&address>(void *)head){
    block * to_free=(block *)(address-sizeof(block));
    return to_free;
  }
  else
    return NULL;
}

block * merge_block(block *curr){
 if(((size_t)curr->next-curr->size-sizeof(block))==(size_t)curr){
//  if ((size_t)((size_t)curr + curr->size + sizeof(block))== (size_t)curr->next) {
//    printf("merge\n");
    curr->size+=sizeof(block)+curr->next->size;
    block *curr_next=curr->next;
    curr->next=curr_next->next;
    curr_next->next=NULL;
  }
  return curr;
}

void add_to_free(block *to_free){
    block *curr=head;
    block *prev=NULL;
  if(head==NULL){
    head=to_free;
    to_free->next=NULL; 
  }
  else{
    while((curr!=NULL)&&(curr<to_free)){
      prev=curr;
      curr=curr->next;
    }
    if((curr!=NULL)&&(curr>to_free)){
      if(head==curr){
        head=to_free;
        prev=NULL;
      }
      else{
        prev->next=to_free;
      }
      to_free->next=curr;
    }
    if(curr==NULL)
    {
        prev->next=to_free;
        to_free->next=NULL;
    }
  }
  
  if(prev!=NULL){
    // printf("p1 m\n");
       to_free=merge_block(prev);
  }
  if(to_free->next!=NULL){
    // printf("p2 m\n");
      merge_block(to_free); 
  } 
}


void bf_free(void *ptr){
    if(!ptr) return;
    block *to_free=ptr;
    to_free--; 
    if(!to_free) 
      return;
    add_to_free(to_free);
    //    printf("free\n");
    // printfreeList();
}

/*block *creat_block_nolock(size_t size){
  if(head_tls==NULL){
    pthread_mutex_lock(&lock_sbrk);
    head_tls= sbrk(sizeof(block)+size);
    pthread_mutex_unlock(&lock_sbrk);
    head_tls->size=size;
    head_tls->is_used=1;
    head_tls->next = NULL;
    head_tls->prev=NULL;
    head_tls->next_f=NULL;
    head_tls->prev_f=NULL;
    tail_tls=head_tls;
    return head_tls;
  }
  pthread_mutex_lock(&lock_sbrk);
  block *new_block=sbrk(0);
  if(sbrk(sizeof(block)+size)==(void *)-1){
    pthread_mutex_unlock(&lock_sbrk);
    return NULL;
  }
  pthread_mutex_unlock(&lock_sbrk);
  
  new_block->size=size;
  new_block->is_used=1;
  new_block->prev=tail_tls;
  new_block->next=NULL;
  new_block->next_f=NULL;
  new_block->prev_f=NULL;
  tail_tls->next=new_block;
  tail_tls=new_block;
  return new_block;    
}

block *split_block_nolock(block *curr,size_t size){
  block *new_block=NULL;
  if((curr->size)>(sizeof(block)+size)){
    new_block=(block *)(get_data(curr)+size);
    new_block->size=curr->size-size-sizeof(block);
    new_block->is_used=0;
    new_block->prev=curr;
    new_block->next=curr->next;
    if(curr->next==NULL){
      tail_tls=new_block;
    }
    else{
      curr->next->prev=new_block;
    }
    curr->next=new_block;
    curr->size=size;
  }
  return new_block;
}

block * is_valid_nolock(void *address){
  pthread_mutex_lock(&lock_sbrk);
  void *loca_now=sbrk(0);
  pthread_mutex_unlock(&lock_sbrk);
  
  if(address<=loca_now&&address>(void *)head_tls){
    block * to_free=(block *)(address-sizeof(block));
    return to_free;
  }
  else
    return NULL;
}

block * merge_block_nolock(block *curr){
      delete_free_nolock(curr->next);
      curr->size+=sizeof(block)+curr->next->size;
      curr->next=curr->next->next;
      if(curr->next!=NULL){
        curr->next->prev=curr;
      }
      else{
       tail_tls=curr; 
      }
  return curr;
}
void delete_free_nolock(block *curr){ 
    if(free_head_tls==free_tail_tls){
      free_head_tls=NULL;
      free_tail_tls=NULL;
    }
    else{
    if(curr==free_head_tls)
      free_head_tls=curr->next_f;
    else
      curr->prev_f->next_f=curr->next_f;   
    if(curr==free_tail_tls)
      free_tail_tls=curr->prev_f;  
    else
      curr->next_f->prev_f=curr->prev_f;  
    }
    curr->next_f=NULL;
    curr->prev_f=NULL;
}

void add_to_free_nolock(block *to_free){
  to_free->prev_f=free_tail_tls;
  if(free_head_tls==NULL){
    free_head_tls=to_free;
  }
  else{
    free_tail_tls->next_f=to_free;
  }
  free_tail_tls=to_free;
  to_free->next_f=NULL; 
}
 

void *ts_malloc_nolock(size_t size){
  size=align_size(size);
  block * new_block;
  block * min=NULL;
  block * curr=free_head_tls;
  while((curr!=NULL)){
   // if((curr->size>=size)&&(curr->is_used==0)){
     if(curr->size>size){
         if(min==NULL){
          min=curr;
        }
        else{
            if(curr->size<min->size){
              min=curr;
              }
        }
    }
    else if(curr->size==size){
      min=curr;
      break;
    }
    curr=curr->next_f;
  }
  if(min==NULL){
    new_block=creat_block_nolock(size);
    if (new_block==NULL){
      return NULL;
    }
  }
  else{
      new_block=min;
      block *newsplit=split_block_nolock(min,size);
      if(newsplit!=NULL){
        add_to_free_nolock(newsplit);
      }
    min->is_used=1;
    delete_free_nolock(min);
  }
  return get_data(new_block);
}


void ts_free_nolock(void *ptr){
    if(!ptr) return;
    block *to_free=(block *)(ptr-sizeof(block));
    if(!to_free) 
      return;
    to_free->is_used=0;
    add_to_free_nolock(to_free);
    if((to_free->prev!=NULL)&&(to_free->prev->is_used==0)){
      if(to_free->prev+to_free->prev->size+sizeof(block)==to_free){
           to_free=merge_block_nolock(to_free->prev);
      }
    }
    if((to_free->next!=NULL)&&(to_free->next->is_used==0)){
      if(to_free+to_free->size+sizeof(block)==to_free->next){
	merge_block_nolock(to_free);
      }
    }
    printf("free\n");
    printList();
    printfreeList();

}*/


unsigned long get_data_segment_size(){
    unsigned long data_size=0;
    block *curr=head;
    data_size=(unsigned long)(sbrk(0)-(void *)head);

    return data_size;
}
unsigned long get_data_segment_free_space_size(){
    unsigned long free_size=0;
   block *curr=head;
  while(curr){
        free_size+=(unsigned long)curr->size+sizeof(block);
    curr=curr->next;
  }
  return free_size;
}


