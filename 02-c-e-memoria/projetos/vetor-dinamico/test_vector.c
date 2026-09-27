#include "vector.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 IntVector v;assert(vector_init(&v)==0);
 for(int i=0;i<1000;++i)assert(vector_push(&v,i*3)==0);
 assert(v.size==1000&&v.capacity>=v.size);
 int x=0;assert(vector_get(&v,37,&x)==0&&x==111);
 assert(vector_set(&v,37,999)==0);assert(vector_get(&v,37,&x)==0&&x==999);
 assert(vector_get(&v,1000,&x)==-1);
 assert(vector_pop(&v,&x)==0&&x==2997&&v.size==999);
 vector_free(&v);assert(v.data==NULL&&v.size==0&&v.capacity==0);
 puts("vector: ok");return 0;
}
