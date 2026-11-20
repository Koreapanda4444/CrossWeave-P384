#include "crossweave.h"
#include "check.h"
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif
static void work(unsigned id){
    uint8_t key[16]={0},nonce[16]={0},p[257],c[257],out[257],tag[16];key[0]=(uint8_t)id;
    for(size_t i=0;i<257;++i)p[i]=(uint8_t)(i+id);
    for(unsigned j=0;j<200;++j){nonce[0]=(uint8_t)j;nonce[1]=(uint8_t)id;
        CHECK(crossweave_encrypt(c,257,tag,16,p,257,p,33,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(crossweave_decrypt(out,257,c,257,tag,16,p,33,nonce,16,key,16)==CROSSWEAVE_OK);
        CHECK(memcmp(p,out,257)==0);
    }
}
#ifdef _WIN32
static unsigned __stdcall run(void *p){work(*(unsigned *)p);return 0;}
#else
static void *run(void *p){work(*(unsigned *)p);return NULL;}
#endif
int main(void){unsigned ids[4]={1,2,3,4};
#ifdef _WIN32
    HANDLE threads[4];
    for(unsigned i=0;i<4;++i){threads[i]=(HANDLE)_beginthreadex(NULL,0,run,&ids[i],0,NULL);CHECK(threads[i]!=NULL);}
    for(unsigned i=0;i<4;++i){CHECK(WaitForSingleObject(threads[i],INFINITE)==WAIT_OBJECT_0);CHECK(CloseHandle(threads[i]));}
#else
    pthread_t threads[4];
    for(unsigned i=0;i<4;++i)CHECK(pthread_create(&threads[i],NULL,run,&ids[i])==0);
    for(unsigned i=0;i<4;++i)CHECK(pthread_join(threads[i],NULL)==0);
#endif
    puts("Four concurrent callers, 800 authenticated round trips with disjoint writable buffers");return 0;
}
