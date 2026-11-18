#include "crossweave_internal.h"
#include "permutation_test.h"
#include "test_io.h"
int main(void) {
    char line[160],mode[16],hex[97],extra; unsigned rounds;
    while(fgets(line,sizeof line,stdin)!=NULL) {
        uint8_t state[48];
        CHECK(sscanf(line,"%15s %u %96s %c",mode,&rounds,hex,&extra)==3);
        CHECK(rounds<=16 && unhex(hex,state,48)==48);
        if(strcmp(mode,"inverse")==0) cw_test_inverse(state,rounds);
        else {
            CHECK(strcmp(mode,"p")==0);
            if(rounds==12) CHECK(crossweave_p384_12(state,48,state,48)==CROSSWEAVE_OK);
            else if(rounds==16) CHECK(crossweave_p384_16(state,48,state,48)==CROSSWEAVE_OK);
            else cw_permute(state,rounds);
        }
        printhex(state,48);
    }
    CHECK(!ferror(stdin)); return 0;
}
