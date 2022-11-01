#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */


#define MAX_LINE_LENGTH 255
#define InstBit 16


static int length = 0;

FILE* infile = NULL;
FILE* outfile = NULL;

int toNum( char * pStr )
{
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if( *pStr == '#' )              /* decimal */
    {
        pStr++;
        if( *pStr == '-' )              /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if( *pStr == 'x' ) /* hex     */
    {
        pStr++;
        if( *pStr == '-' )              /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if( lNeg )
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

enum code{
    ADD,AND,BR,BRZ,BRN,BRP,BRNZP,BRZP,BRNP,HALT,JMP,JSR,JSRR,LDB,LDW,LEA,NOP,NOT,RET,LSHF,RSHFL,RSHFA,RTI,STB,STW,TRAP,XOR,ORIG,FILL,END
};

int isOpcode(char *ptr){
    if(strcmp(ptr, "add") == 0){
        return ADD;
    }
    else if(strcmp(ptr,"and") == 0){
        return AND;
    }
    else if(strcmp(ptr,"br") == 0){
        return BR;
    }
    else if(strcmp(ptr,"brz") == 0){
        return BRZ;
    }
    else if(strcmp(ptr,"brn") == 0){
        return BRN;
    }
    else if(strcmp(ptr,"brp") == 0){
        return BRP;
    }
    else if(strcmp(ptr,"brnzp") == 0){
        return BRNZP;
    }
    else if(strcmp(ptr,"brzp") == 0){
        return BRZP;
    }
    else if(strcmp(ptr,"brnp") == 0){
        return BRNP;
    }
    else if(strcmp(ptr,"halt") == 0){
        return HALT;
    }
    else if(strcmp(ptr,"jmp") == 0){
        return JMP;
    }
    else if(strcmp(ptr,"jsr") == 0){
        return JSR;
    }
    else if(strcmp(ptr,"jsrr") == 0){
        return JSRR;
    }
    else if(strcmp(ptr,"ldb") == 0){
        return LDB;
    }
    else if(strcmp(ptr,"ldw") == 0){
        return LDW;
    }
    else if(strcmp(ptr,"lea") == 0){
        return LEA;
    }
    else if(strcmp(ptr,"nop") == 0){
        return NOP;
    }
    else if(strcmp(ptr,"not") == 0){
        return NOT;
    }
    else if(strcmp(ptr,"ret") == 0){
        return RET;
    }
    else if(strcmp(ptr,"lshf") == 0){
        return LSHF;
    }
    else if(strcmp(ptr,"rshfl") == 0){
        return RSHFL;
    }
    else if(strcmp(ptr,"rshfa") == 0){
        return RSHFA;
    }
    else if(strcmp(ptr,"rti") == 0){
        return RTI;
    }
    else if(strcmp(ptr,"stb") == 0){
        return STB;
    }
    else if(strcmp(ptr,"stw") == 0){
        return STW;
    }
    else if(strcmp(ptr,"trap") == 0){
        return TRAP;
    }
    else if(strcmp(ptr,"xor") == 0){
        return XOR;
    }
    else if(strcmp(ptr,".orig") == 0){
        return ORIG;
    }
    else if(strcmp(ptr,".fill") == 0){
        return FILL;
    }
    else if(strcmp(ptr,".end") == 0){
        return END;
    }
    else{
        return -1;
    }
}

typedef struct data {
    int idx;
    char* name;
    struct data *nextdata;
}data;


void newData(data* dat, char* name, int idx){
    int i = 0;
    while(i < length){
        dat = dat -> nextdata;
        i++;
    }

    data* newdata = (data*) malloc((sizeof(data)));
    dat -> nextdata = newdata;
    int names = strlen(name);
    newdata -> idx = idx;
    newdata -> name = strncpy((char*)malloc(names + 1), name, names);
    newdata -> nextdata = NULL;
    length++;
}

int findLabel(data* dat, char* label){
    int i = 0;
    while(i <= length){
        if(strcmp(dat->name, label) == 0){
            return dat->idx;
        }
        else{
            dat = dat -> nextdata;
        }
    }
    return NULL;
}

data* Hash(){
    data* hash = (data*)malloc(sizeof(data));

    hash->idx = -1;
    hash->name = "in";
    hash = (hash->nextdata = (data*)malloc(sizeof(data)));

    hash->idx = -1;
    hash->name = "out";
    hash = (hash->nextdata = (data*)malloc(sizeof(data)));

    hash->idx = -1;
    hash->name = "getc";
    hash = (hash->nextdata = (data*)malloc(sizeof(data)));

    hash->idx = -1;
    hash->name = "puts";
    hash = NULL;

    return hash;
}

enum
{
    DONE, OK, EMPTY_LINE
};

int
readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
)
{
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while( *lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n' )
        lPtr++;

    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
        return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    }

    *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg1 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg4 = lPtr;

    return( OK );
}

char hex(int num){
    if(num == 0){
        return '0';
    }
    else if(num == 1){
        return '1';
    }
    else if(num == 2){
        return '2';
    }
    else if(num == 3){
        return '3';
    }
    else if(num == 4){
        return '4';
    }
    else if(num == 5){
        return '5';
    }
    else if(num == 6){
        return '6';
    }
    else if(num == 7){
        return '7';
    }
    else if(num == 8){
        return '8';
    }
    else if(num == 9){
        return '9';
    }
    else if(num == 10){
        return 'A';
    }
    else if(num == 11){
        return 'B';
    }
    else if(num == 12){
        return 'C';
    }
    else if(num == 13){
        return 'D';
    }
    else if(num == 14){
        return 'E';
    }
    else if(num == 15){
        return 'F';
    }
}

void binOut(int *x, FILE *pOutfile) {
    int i;
    int num;
    fprintf(pOutfile, "0x");
    for (i = 0; i < InstBit; i += 4) {
        num = *(x + i + 3)
              + (*(x + i + 2) * 2)
              + (*(x + i + 1) * 4)
              + (*(x + i + 0) * 8);
        fprintf(pOutfile, "%c", hex(num));
    }

    fprintf(pOutfile, "\n");
}


void int_to_bin(int* x, int num, int bit) {
    int a[InstBit] = { 0 };
    int b = bit - 1;
    int c = 0;
    int d = 0;
    int i;
    int carry = 0;
}

void Add(FILE *pOutfile, char *A1, char *A2, char *A3, char *A4) {
    int binary[16] = { 0,0,0,1 };
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);

    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        binary[10] = 1;
        if (a3 >= -16 && a3 < 16) {
            int_to_bin(&binary[11], a3, 5);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }
    else {
        binary[10] = 0;
        binary[11] = 0;
        binary[12] = 0;
        if (a3 >= 0 && a3 <= 7) {
            int_to_bin(&binary[13], a3, 3);
        }
        else {
            printf("Invalid register: %s", A3);
            exit(4);
        }
    }
    binOut(binary, pOutfile);
}

void And(FILE *pOutfile, char *A1, char *A2, char *A3, char *A4) {
    int binary[16] = { 0,1,0,1 };
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);


    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        binary[10] = 1;
        if (a3>= -16 && a3 < 16) {
            int_to_bin(&binary[11], a3, 5);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }
    else {
        binary[10] = 0;
        binary[11] = 0;
        binary[12] = 0;
        if (a3 >= 0 && a3 <= 7) {
            int_to_bin(&binary[13], a3, 3);
        }
        else {
            printf("Invalid register: %s", A3);
            exit(4);
        }
    }
    binOut(binary, pOutfile);
}

void Br(FILE *pOutfile, char *A1, data *ptr, int n, int z, int p, int line) {
    int binary[16] = { 0,0,0,0,n,z,p };
    int offset;


    if (*A1 == '#') {
        offset = toNum(A1);

        if (offset >= -256 && offset < 256) {
            int_to_bin(&binary[7], offset, 9);
        }
        else {
            printf("Error: Invalid Constant: %s", A1);
            exit(3);
        }
        binOut(binary, pOutfile);
    }

    else if (findLabel(ptr, A1) == -1) {
        printf("Error: %s is not define", A1);
        exit(1);
    }

    else {
        offset = findLabel(ptr, A1) - (line+1);
        if (offset >= -256 && offset < 256) {
            int_to_bin(&binary[7], offset, 9);
        }
        else {
            printf("Error: Out of Bound: %s", A1);
            exit(4);
        }
        binOut(binary, pOutfile);
    }

}

void Xor(FILE *pOutfile, char *A1, char *A2, char *A3, char *A4) {
    int binary[16] = { 1,0,0,1 };
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);


    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        binary[10] = 1;
        if (a3 >= -16 && a3 < 16) {
            int_to_bin(&binary[11], a3, 5);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }
    else {
        binary[10] = 0;
        binary[11] = 0;
        binary[12] = 0;
        if (a3 >= 0 && a3 <= 7) {
            int_to_bin(&binary[13], a3, 3);
        }
        else {
            printf("Invalid register: %s", A3);
            exit(4);
        }
    }
    binOut(binary, pOutfile);
}

void Ld(FILE *pOutfile, char *A1, char *A2, char *A3, int BW) {
    int binary[16] = { 0,BW,1,0 };
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);


    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        if (a3 >= -32 && a3 < 131) {
            int_to_bin(&binary[10], a3, 6);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }
    else {
        printf("Invalid operand: %s", A3);
        exit(2);
    }

    binOut(binary, pOutfile);
}

void Return(FILE *pOutfile, int n){
    int binary[16] = {1,n,0,0,0,0,0,n,n,n,0,0,0,0,0,0};
    binOut(binary, pOutfile);

}

void St(FILE *pOutfile, char *A1, char *A2, char *A3, int BW) {
    int binary[16] = { 0,BW,1,1 };
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);


    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        if (a3 >= -32 && a3 < 131) {
            int_to_bin(&binary[10], a3, 6);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }
    else {
        printf("Invalid operand: %s", A3);
        exit(2);
    }

    binOut(binary, pOutfile);
}

void Trap(FILE *pOutfile, char *A1) {
    int binary[16] = {1,1,1,1,0,0,0,0};
    int vector = toNum(A1);

    if(vector >= -128 && vector <=127) {
        int_to_bin(&binary[8], vector, 8);
        binOut(binary, pOutfile);
    }

    else {
        printf("Error: Invalid Constant: %s", A1);
        exit(4);
    }
}

void Shift(FILE *pOutfile, char *A1, char *A2, char *A3, int n, int m) {
    int binary[16] = {1,1,0,1};
    int a1 = toNum(A1);
    int a2 = toNum(A2);
    int a3 = toNum(A3);


    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (a2 >= 0 && a2 <= 7) {
        int_to_bin(&binary[7], a2, 3);
    }
    else {
        printf("Invalid register: %s", A2);
        exit(4);
    }

    if (*A3 == '#') {
        binary[10] = n;
        binary[11] = m;
        if (a3 >= 0 && a3 < 16) {
            int_to_bin(&binary[12], a3, 4);
        }
        else {
            printf("Error: Invalid Constant: %s", A3);
            exit(3);
        }
    }

    binOut(binary, pOutfile);
}

void Jmp(FILE *pOutfile, char *A1) {
    int binary[16] = {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int base = toNum(A1);

    if (base >= 0 && base <= 7) {
        int_to_bin(&binary[7], base, 3);
    }
    binOut(binary,pOutfile);

}

void Jsrr(FILE *pOutfile, char *A1, int n, data *ptr, int line) {
    int binary[16] = {0,1,0,0,n,0,0,0,0,0,0,0,0,0,0,0};
    int base;
    int offset;

    if(n == 0) {
        base = toNum(A1);
        if (base >= 0 && base <= 7) {
            int_to_bin(&binary[7], base, 3);
        }
    }
    else if (*A1 == '#') {
        offset = toNum(A1);

        if (offset >= -((2<<11)/2) && offset < ((2<<11)/2)) {
            int_to_bin(&binary[5], offset, 11);
        }
        else {
            printf("Error: Invalid Constant: %s", A1);
            exit(3);
        }
    }
    else if (findLabel(ptr, A1) == -1) {
        printf("Error: %s is not define", A1);
        exit(1);
    }
    else {
        offset = findLabel(ptr, A1) - (line+1);
        if (offset >= -((2<<11)/2) && offset < ((2<<11)/2)) {
            int_to_bin(&binary[5], offset, 9);
        }
        else {
            printf("Error: Out of Bound: %s", A1);
            exit(4);
        }
    }

    binOut(binary,pOutfile);

}

void Lea(FILE *pOutfile, char *A1, char *A2, data *ptr, int line) {
    int binary[16] = { 1,1,1,0};
    int offset;
    int a1=toNum(A1);;

    if (a1 >= 0 && a1 <= 7) {
        int_to_bin(&binary[4], a1, 3);
    }
    else {
        printf("Invalid register: %s", A1);
        exit(4);
    }

    if (*A2 == '#') {
        offset = toNum(A2);
        if (offset >= -256 && offset < 256) {
            int_to_bin(&binary[7], offset, 9);
        }
        else {
            printf("Error: Invalid Constant: %s", A2);
            exit(3);
        }
        binOut(binary, pOutfile);
    }

    else if (findLabel(ptr, A2) == -1) {
        printf("Error: %s is not define", A2);
        exit(1);
    }

    else {
        offset = findLabel(ptr, A2) - (line+1);
        if (offset >= -256 && offset < 256) {
            int_to_bin(&binary[7], offset, 9);
        }
        else {
            printf("Error: Out of Bound: %s", A2);
            exit(4);
        }
        binOut(binary, pOutfile);
    }

}



void setLabel(FILE *pInfile, data* ptr, int* len){
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;
    int line;

    FILE * lInfile;

    lInfile = fopen( "data.in", "r" );  /* open the input file */

    do
    {
        lRet = readAndParse( lInfile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            if(findLabel(ptr, lLabel) == NULL){
                newData(ptr,lLabel, line -1);
                *lLabel = NULL;
            }
            else{
                exit(4);
            }
        }
        if(*lOpcode != NULL){
            line++;
        }
    } while( lRet != DONE );


}

void assemble(FILE *pInfile, FILE *pOutfile, data* ptr, int len) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;

    int lRet;
    int line = -2;

    FILE * lInfile;

    lInfile = fopen( "data.in", "r" );

    do
    {
        lRet = readAndParse( lInfile, lLine, &lLabel,
                             &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            if(*lOpcode != NULL){
                   line++;
                   if(isOpcode(lOpcode)==0){
                       Add(pOutfile, lArg1, lArg2, lArg3, lArg4); break;
                   }
                   else if(isOpcode(lOpcode)==1){
                       And(pOutfile, lArg1, lArg2, lArg3, lArg4); break;
                   }
                   else if(isOpcode(lOpcode)==2){
                       Br(pOutfile,  lArg1, ptr, 1, 1, 1, line); break;
                   }
                   else if(isOpcode(lOpcode)==3){
                       Br(pOutfile,  lArg1, ptr, 1, 0, 0, line); break;
                   }
                   else if(isOpcode(lOpcode)==4){
                       Br(pOutfile,  lArg1, ptr, 0, 0, 1, line); break;
                   }
                   else if(isOpcode(lOpcode)==5){
                       Br(pOutfile,  lArg1, ptr, 0, 1, 0, line); break;
                   }
                   else if(isOpcode(lOpcode)==6){
                       Br(pOutfile,  lArg1, ptr, 1, 0, 1, line); break;
                   }
                   else if(isOpcode(lOpcode)==7){
                       Br(pOutfile, lArg1, ptr, 0, 1, 1, line); break;
                   }
                   else if(isOpcode(lOpcode)==8){
                       Br(pOutfile, lArg1, ptr, 1, 1, 0, line); break;
                   }
                   else if(isOpcode(lOpcode)==9){
                       Br(pOutfile, lArg1, ptr, 1, 1, 1, line); break;
                   }
                   else if(isOpcode(lOpcode)==10){
                       Xor(pOutfile, lArg1, lArg2, lArg3, lArg4); break;
                   }
                   else if(isOpcode(lOpcode)==11){
                       Ld(pOutfile, lArg1, lArg2, lArg3, 0); break;
                   }
                   else if(isOpcode(lOpcode)==12){
                       Ld(pOutfile, lArg1, lArg2, lArg3, 1); break;
                   }
                   else if(isOpcode(lOpcode)==13){
                       Xor(pOutfile, lArg1, lArg2, "#-1", lArg4); break;
                   }
                   else if(isOpcode(lOpcode)==14){
                       Return(pOutfile,1); break;
                   }
                   else if(isOpcode(lOpcode)==15){
                       Return(pOutfile,0); break;
                   }
                   else if(isOpcode(lOpcode)==16){
                       St(pOutfile, lArg1, lArg2, lArg3, 0); break;
                   }
                   else if(isOpcode(lOpcode)==17){
                       St(pOutfile, lArg1, lArg2, lArg3, 1); break;
                   }
                   else if(isOpcode(lOpcode)==18){
                       Trap(pOutfile, lArg1); break;
                   }
                   else if(isOpcode(lOpcode)==19){
                       Trap(pOutfile, "x25"); break;
                   }
                   else if(isOpcode(lOpcode)==20){
                       Shift(pOutfile, lArg1, lArg2, lArg3,0,0); break;
                   }
                   else if(isOpcode(lOpcode)==21){
                       Shift(pOutfile, lArg1, lArg2, lArg3,1,1); break;
                   }
                   else if(isOpcode(lOpcode)==22){
                       Shift(pOutfile, lArg1, lArg2, lArg3,1,1); break;
                   }
                   else if(isOpcode(lOpcode)==23){
                       Jmp(pOutfile,lArg1); break;
                   }
                   else if(isOpcode(lOpcode)==24){
                       Jsrr(pOutfile,lArg1,1,ptr, line); break;
                   }
                   else if(isOpcode(lOpcode)==25){
                       Jsrr(pOutfile,lArg1,0,ptr, line); break;
                   }
                   else if(isOpcode(lOpcode)==26){
                       Lea(pOutfile, lArg1, lArg2, ptr, line); break;
                   }
                   else if(isOpcode(lOpcode)==27){
                       if(toNum(lArg1)%2 != 0) {
                           printf("Bad .orig\n");
                           exit(3);
                       }
                       break;
                   }
                   else if(isOpcode(lOpcode)==28){
                       fprintf(pOutfile, "0x%x\n", toNum(lArg1)); break;
                   }
                   else if(isOpcode(lOpcode)==29){

                   }
                   else {
                       printf("not an opcode");
                       exit(2);
                   }

            }
        }
    } while( lRet != DONE );

}

data* hash;

int main(int argc, char* argv[]) {

    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    char  *lLabel, *lOpcode, *lArg1,
            *lArg2, *lArg3, *lArg4;
    int len;


    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    hash = Hash();
    setLabel(infile, hash, &len);
    fclose(infile);
    infile = fopen(iFileName, "r");
    assemble(infile, outfile, hash, len);

    fclose(infile);
    fclose(outfile);
}


