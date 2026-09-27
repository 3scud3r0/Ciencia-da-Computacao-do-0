#include "vector.h"
#include <stdint.h>
#include <stdlib.h>
#define VECTOR_INITIAL_CAPACITY 4u
static int vector_reserve(IntVector *v,size_t n){
 if(n<=v->capacity)return 0;
 if(n>SIZE_MAX/sizeof(*v->data))return -1;
 int *p=realloc(v->data,n*sizeof(*v->data));
 if(p==NULL)return -1;
 v->data=p;v->capacity=n;return 0;
}
int vector_init(IntVector *v){if(v==NULL)return -1;v->data=NULL;v->size=0;v->capacity=0;return 0;}
int vector_push(IntVector *v,int x){
 if(v==NULL)return -1;
 if(v->size==v->capacity){
  size_t n=v->capacity==0?VECTOR_INITIAL_CAPACITY:v->capacity*2u;
  if(n<v->capacity||vector_reserve(v,n)!=0)return -1;
 }
 v->data[v->size++]=x;return 0;
}
int vector_get(const IntVector *v,size_t i,int *out){if(v==NULL||out==NULL||i>=v->size)return -1;*out=v->data[i];return 0;}
int vector_set(IntVector *v,size_t i,int x){if(v==NULL||i>=v->size)return -1;v->data[i]=x;return 0;}
int vector_pop(IntVector *v,int *out){if(v==NULL||v->size==0)return -1;v->size--;if(out!=NULL)*out=v->data[v->size];return 0;}
void vector_free(IntVector *v){if(v==NULL)return;free(v->data);v->data=NULL;v->size=0;v->capacity=0;}
