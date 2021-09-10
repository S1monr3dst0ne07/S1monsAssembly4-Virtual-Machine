#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FILE_BUFFER_SiZE    256
#define MAPPER_SIZE         2048
#define MEM_SIZE            65535
#define STACK_SIZE          2048
#define ATTR_SIZE           256

#define ArraySize(x) sizeof(x) / sizeof(x[0])

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

typedef struct 
{
    char* instOrg;
    char* attrOrg;

    int inst;
    char attr[ATTR_SIZE];
    int lineNum;
    int conLine;

} LINE;


enum
{
    invaild,
    set,
    add,
    sub,
    shg,
    shs,
    lor,
    and,
    xor,
    not,
    lDA,
    lDR,
    sAD,
    sRD,
    lPA,
    lPR,
    sAP,
    sRP,
    out,
    inp_asm,
    lab,
    got,
    jm0,
    jmA,
    jmG,
    jmL,
    jmS,
    ret,
    pha,
    pla,
    brk,
    clr,
    putstr,
    ahm,
    fhm
};




int findIndex(const char* sourceArray[], const char* targetChar, const int size)
{
    for (int i = 0; i < size; i++)
    {
        const char* currentIndex = sourceArray[i];
        if (strcmp(currentIndex, targetChar) == 0)
        {
            return i;
        }
    }

    return -1;
}




int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "File Path not given\n");
        return 1;
    }

    char* runFilePath;
    runFilePath = argv[1];

    FILE* filePtr;
    char buffer[FILE_BUFFER_SiZE];
    errno_t fileOpenError;

    fileOpenError = fopen_s(&filePtr, runFilePath, "r");
    if (fileOpenError != 0)
    {
        fprintf(stderr, "Error while loading file");
        return 1;
    }


    //count number real line in file
    int lineCount = 0;
    while (fgets(buffer, FILE_BUFFER_SiZE - 1, filePtr))
    {
        buffer[strcspn(buffer, "\n")] = 0;

        //check for vaild lines, aka lines that don't begin with a terminator or a comment marker        
        if (buffer[0] != 0 && buffer[0] != '"') lineCount++;
    }
    rewind(filePtr);


    //alloc all needed memory
    LINE* lineBuffer = (LINE*)malloc(lineCount * sizeof(LINE));


    //parse file into a list of structs
    int lineIndex = 0;
    int mapperIndex = 0;
    char *inst, *attr, *attrNew;
    char* attrMapper[MAPPER_SIZE] = { 0 };
    int lineMapper[MAPPER_SIZE]   = { 0 };

    while (fgets(buffer, FILE_BUFFER_SiZE - 1, filePtr))
    {
        buffer[strcspn(buffer, "\n")] = NULL;
        if (buffer[0] != 0 && buffer[0] != '"')
        {
            lineBuffer[lineIndex].conLine = NULL;

            inst = strtok(buffer, " ");
            attr = strtok(NULL, " ");

            //DEBUG START
            /*
            char* attrOrg;
            char* instOrg = malloc(sizeof(inst) + sizeof(char));
            memset(instOrg, 0, sizeof(instOrg));
            strcpy(instOrg, inst);

            if (attr != NULL)
            {
                attrOrg = malloc(sizeof(attr) + sizeof(char));
                memset(attrOrg, 0, sizeof(attrOrg));
                strcpy(attrOrg, attr);
            }
            else
            {
                attrOrg = NULL;
            }

            lineBuffer[lineIndex].instOrg = instOrg;
            lineBuffer[lineIndex].attrOrg = attrOrg;
            */
            //DEBUG END

            if (attr != NULL)
            {
                strcpy(lineBuffer[lineIndex].attr, attr);
            }
            else
            {
                memset(lineBuffer[lineIndex].attr, 0, ATTR_SIZE);
            }



            int instNum = NULL;
            int index = 0;
            int targetLine = 0;

            /**/ if (strcmp(inst, "set") == 0)      instNum = set;
            else if (strcmp(inst, "add") == 0)      instNum = add;
            else if (strcmp(inst, "sub") == 0)      instNum = sub;
            else if (strcmp(inst, "shg") == 0)      instNum = shg;
            else if (strcmp(inst, "shs") == 0)      instNum = shs;
            else if (strcmp(inst, "lor") == 0)      instNum = lor;
            else if (strcmp(inst, "and") == 0)      instNum = and;
            else if (strcmp(inst, "xor") == 0)      instNum = xor;
            else if (strcmp(inst, "not") == 0)      instNum = not;
            else if (strcmp(inst, "lDA") == 0)      instNum = lDA;
            else if (strcmp(inst, "lDR") == 0)      instNum = lDR;
            else if (strcmp(inst, "sAD") == 0)      instNum = sAD;
            else if (strcmp(inst, "sRD") == 0)      instNum = sRD;
            else if (strcmp(inst, "lPA") == 0)      instNum = lPA;
            else if (strcmp(inst, "lPR") == 0)      instNum = lPR;
            else if (strcmp(inst, "sAP") == 0)      instNum = sAP;
            else if (strcmp(inst, "sRP") == 0)      instNum = sRP;
            else if (strcmp(inst, "out") == 0)      instNum = out;
            else if (strcmp(inst, "inp") == 0)      instNum = inp_asm; //inp is already used as a c keyword


            else if (strcmp(inst, "lab") == 0)
            {
                lineMapper[mapperIndex] = lineIndex;
                attrMapper[mapperIndex] = lineBuffer[lineIndex].attr;
                instNum = lab;

                mapperIndex++;
            }
            

            else if (strcmp(inst, "got") == 0)
            {
                instNum = got;
            }
            else if (strcmp(inst, "jm0") == 0)

            {
                instNum = jm0;
            }
            else if (strcmp(inst, "jmA") == 0)
            {
                instNum = jmA;
            }
            else if (strcmp(inst, "jmG") == 0)
            {
                instNum = jmG;
            }
            else if (strcmp(inst, "jmL") == 0)
            {
                instNum = jmL;
            }
            else if (strcmp(inst, "jmS") == 0)
            {
                instNum = jmS;
            }


            else if (strcmp(inst, "ret") == 0)      instNum = ret;            
            else if (strcmp(inst, "pha") == 0)      instNum = pha;
            else if (strcmp(inst, "pla") == 0)      instNum = pla;
            else if (strcmp(inst, "brk") == 0)      instNum = brk;
            else if (strcmp(inst, "clr") == 0)      instNum = clr;
            else if (strcmp(inst, "putstr") == 0)   instNum = putstr;
            else if (strcmp(inst, "ahm") == 0)      instNum = ahm;
            else if (strcmp(inst, "fhm") == 0)      instNum = fhm;

            else
            {
                instNum = invaild;
            }




            lineBuffer[lineIndex].inst = instNum;
            lineBuffer[lineIndex].lineNum = lineIndex;


            lineIndex++;
        }
    }




    //set conLine of all lines containing a jump

    int   runInst;
    char* runAttr;

    for (int i = 0; i < lineCount; i++)
    {
        LINE lineStruct = lineBuffer[i];
        runInst = lineStruct.inst;
        runAttr = lineStruct.attr;


        if (runInst >= got && runInst <= jmS)
        {
            int attr2lineIndex = findIndex(attrMapper, runAttr, mapperIndex);
            lineBuffer[i].conLine = lineMapper[attr2lineIndex];

        }
    }

    //now finally run with lightspeed
    unsigned short int acc = 0;
    unsigned short int reg = 0;

    unsigned short int mem[MEM_SIZE]     = { 0 };
    unsigned short int stack[STACK_SIZE] = { 0 };
    int stackPtr = 0;

    char outputBuffer[2] = "";


    for (int execPtr = 0; execPtr < lineCount; execPtr++)
    {
        LINE lineStruct = lineBuffer[execPtr];

        runInst = lineStruct.inst;
        runAttr = lineStruct.attr;


        switch (runInst)
        {
            case set:
                reg = (int)atoi(runAttr);
                break;

            case add:
                acc += reg;
                break;

            case sub:
                acc -= reg;
                break;

            case shg:
                acc <<= 1;
                break;

            case shs:
                acc >>= 1;
                break;

            case lor:
                acc |= reg;
                break;

            case and:
                acc &= reg;
                break;

            case xor:
                acc ^= reg;

            case not:
                acc = ~acc;

            case lDA:
                acc = mem[(int)atoi(runAttr)];
                break;

            case lDR:
                reg = mem[(int)atoi(runAttr)];
                break;

            case sAD:
                mem[(int)atoi(runAttr)] = acc;
                break;

            case sRD:
                mem[(int)atoi(runAttr)] = reg;
                break;


            case lPA:
                acc = mem[mem[(int)atoi(runAttr)]];
                break;

            case lPR:
                reg = mem[mem[(int)atoi(runAttr)]];
                break;

            case sAP:
                mem[mem[(int)atoi(runAttr)]] = acc;
                break;

            case sRP:
                mem[mem[(int)atoi(runAttr)]] = reg;
                break;

            case out:
                printf("%d\n", mem[atoi(runAttr)]);
                break;

            case inp_asm:
                printf(">>>");
                scanf("%hd", &mem[atoi(runAttr)]);
                break;


            case got:
                execPtr = lineStruct.conLine;
                break;

            case jm0:
                if(acc == 0) execPtr = lineStruct.conLine;
                break;

            case jmA:
                if (acc == reg) execPtr = lineStruct.conLine;
                break;

            case jmG:
                if (acc > reg) execPtr = lineStruct.conLine;
                break;

            case jmL:
                if (acc < reg) execPtr = lineStruct.conLine;
                break;

            case jmS:
                //save the execPtr on the stack
                stack[stackPtr++] = execPtr;
                execPtr = lineStruct.conLine;
                break;

            case ret:
                execPtr = stack[--stackPtr];
                break;

            case pha:
                stack[stackPtr++] = acc;
                break;

            case pla:
                acc = stack[--stackPtr];
                break;

            case brk:
                exit(0);
                break;

            case clr:
                acc = 0;
                reg = 0;
                break;

            case putstr:
                outputBuffer[0] = (char)acc;
                printf("%s", &outputBuffer);

            default:
                break;
        }



    }




    return 0;
}



