#include "cachelab.h"
#include<stdlib.h>
#include <stdio.h>
#include"cachelab.h"
#include <getopt.h>


#define _not_hit_ -1
#define _uint_max_ (unsigned int)-1
#define False 0
#define True 1
#define max(a,b) (a>b)?a:b

typedef struct Cache
{
        unsigned int s;
        unsigned int E;
        unsigned int b;
        unsigned int ***cache_model; 
}Cache;

typedef struct AdressInfo
{
        unsigned int in_which_s;
        unsigned int tag;
}Adressinfo;

typedef struct CacheRecord
{
        unsigned int hit;
        unsigned int miss;
        unsigned int eviction; 
}CacheRecord;

typedef _Bool bool;

void cacheInit(Cache *cache,unsigned int s,unsigned int E,unsigned int b);
void freeCache(Cache *cache);
Adressinfo getAdressInfomation(Cache *cache, long unsigned int adress);
int checakCache(Cache *cache, Adressinfo *adress_info);
unsigned int getMaxLRUPosInS(Cache *cache, Adressinfo *adress_info);
void updateCache(Cache *cache, Adressinfo *adress_info, CacheRecord *result);
void printHelp();

int main(int const argc,char **const argv)
{
        //bool verbose = False;
        bool want_help = False;
        bool no_such_arg = False;
        int s = 0,E = 0,b = 0;
        FILE *file_ptr = NULL;
        char opt = 0;
        while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != EOF) {
                        switch (opt) {
                                case 'h':
                                        want_help = True;
                                        break;
                                case 'v':
                                        //verbose = 1;
                                        break;
                                case 's':
                                        s = atoi(optarg);
                                        break;
                                case 'E':
                                        E = atoi(optarg);
                                        break;
                                case 'b':
                                        b = atoi(optarg);
                                        break;
                                case 't':
                                        file_ptr = fopen(optarg, "r");
                                        break;
                                default: /* '?' */
                                        printf("./csim-ref: Missing required command line argument");
                                        no_such_arg = True;
                                        break;
                        }
        }

          //error check
        if(want_help || s < 0 || E < 0|| (file_ptr == NULL) || no_such_arg){
                printHelp();
                exit(EXIT_FAILURE);
        }

        Cache cache;
        cacheInit(&cache,s,E,b);

        char cmd;
        unsigned long int adress = 0;
        char symbol;
        unsigned int size = 0;
        CacheRecord result = {0,0,0};
        while(fscanf(file_ptr," %c %lx %c %u",&cmd,&adress,&symbol,&size) != EOF){
                if(cmd == 'I')continue;
                else{
                        Adressinfo adress_info = getAdressInfomation(&cache,adress);
                        updateCache(&cache,&adress_info,&result);
                        (cmd == 'M')?updateCache(&cache,&adress_info,&result):(char)0;
                }
        }
        
        //fprintf(file_ptr,"%hhd%lx%hhd%d",cmd,adress,symbol,time);
        //printf("%c %lx%c%u\n",cmd,adress,symbol,time);
        
        printSummary(result.hit, result.miss, result.eviction);
        freeCache(&cache);
        return 0;
}


void cacheInit(Cache *cache,unsigned int s,unsigned int E,unsigned int b){
        cache->s = s;
        cache->E = E;
        cache->b = b;

        cache->cache_model = (unsigned int***)malloc((1<<s)*sizeof(unsigned int**));
        for(int i = 0;i < (1<<s);++i){
                cache->cache_model[i] = (unsigned int**)malloc(E*sizeof(unsigned int*));
                for(int j = 0;j < E;++j){
                //0--is valid?   1--tag   2--LRU
                        cache->cache_model[i][j] = (unsigned int*)malloc(3*sizeof(unsigned int));//calloc will do the zero initialization
                        cache->cache_model[i][j][0] = 0;
                        cache->cache_model[i][j][1] = 0;
                        cache->cache_model[i][j][2] = _uint_max_;
                }
        }

}

void freeCache(Cache *cache){
        for(int i = 0;i < cache->s;++i){
                for(int j = 0;j < cache->E;++j){
                        free(cache->cache_model[i][j]);
                }
                free(cache->cache_model[i]);
        }
        free(cache->cache_model);
}

Adressinfo getAdressInfomation(Cache *cache, long unsigned int adress){
        unsigned int tag = adress >> (cache->b + cache->s);
        unsigned int mask = 0;
        for(int i = 0; i < cache->s;++i){
                mask += (1<<i);
        }
        unsigned int in_which_s = (adress >> cache->b ) & mask;
        Adressinfo adress_info;
        adress_info.in_which_s = in_which_s;
        adress_info.tag = tag;
        return adress_info;
}

int checakCache(Cache *cache, Adressinfo *adress_info){
        for(unsigned int i = 0;i < cache->E;++i){
                if(cache->cache_model[adress_info->in_which_s][i][0] && cache->cache_model[adress_info->in_which_s][i][1] == adress_info->tag){
                return i;
                }
        }
        return _not_hit_;
}

unsigned int getMaxLRUPosInS(Cache *cache, Adressinfo *adress_info){
        unsigned int max_LRU= 0;
        unsigned int pos = 0;
        for(unsigned int i = 0; i < cache->E;++i){
                unsigned int LRU = cache->cache_model[adress_info->in_which_s][i][2];
                if(LRU > max_LRU){
                max_LRU = LRU;
                pos = i;
                }
        }
        return pos;
}

void updateCache(Cache *cache, Adressinfo *adress_info, CacheRecord *result){
        int pos = checakCache(cache,adress_info);
        if(pos == _not_hit_){
                result->miss += 1;
                pos =  getMaxLRUPosInS(cache,adress_info); //locate LRU position if miss
                if(cache->cache_model[adress_info->in_which_s][pos][0] != 0){
                        result->eviction += 1;
                }
                else{
                        cache->cache_model[adress_info->in_which_s][pos][0]  = 1;
                }
                cache->cache_model[adress_info->in_which_s][pos][1] = adress_info->tag;
         }
        else{
                result->hit +=1;
        }
        for(int i = 0;i < cache->E;++i){
                if(i == pos){
                        cache->cache_model[adress_info->in_which_s][i][2] = 1;
                }
                else{
                        cache->cache_model[adress_info->in_which_s][i][2] = max(cache->cache_model[adress_info->in_which_s][i][2]+1,cache->cache_model[adress_info->in_which_s][i][2]);
                }
        }
}

void printHelp(){
        printf("some help information");
}
