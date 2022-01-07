#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include <windows.h>
#include <SDL.h>

#define FILE_BUFFER_SiZE    256
#define MAPPER_SIZE         2048
#define MEM_SIZE            65535
#define STACK_SIZE          2048
#define ATTR_SIZE           256
#define STRING_BUFFER       1024
#define CHUNK_LIST_SIZE     255


#define ArraySize(x) sizeof(x) / sizeof(x[0])

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS




typedef struct 
{
    //original command
    char* instOrg;
    char* attrOrg;

    //parsed command
    int inst;
    char attr[ATTR_SIZE];

    //parsed attr, if attr can be stored as int
    int attrInt;

    //error feedback
    int lineNum;
    int conLine;

} LINE;




typedef struct
{
    unsigned int ptr;
    unsigned int size;
    bool isAlloced;


} CHUNK;

void printChunkList(CHUNK chunkList[])
{
    printf("START CHUNK LIST\n");
    for (int i = 0; i < CHUNK_LIST_SIZE; i++)
    {
        CHUNK iter = chunkList[i];
        if (iter.isAlloced)
        {
            printf("Index: %d\n", i);
            printf("    Ptr      : %d\n", iter.ptr);
            printf("    Size     : %d\n", iter.size);
        }

    }
    printf("END   CHUNK LIST\n");


}



enum
{
    invalid,
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
    fhm,
    plugin,
};




int isOnlyDigits(const char* s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }

    return 1;
}

//this function will pop from the stack, just for convenience
unsigned short int _PopStack(short int* stack, int* stackPtr)
{
    return stack[--*stackPtr];
}


char* _GetString(short int* mem, unsigned short int ptr)
{
    unsigned short int scanPtr = ptr;
    int size = 0;

    while (mem[scanPtr++]) size++;
    
    char* stringBuffer = malloc((size + 1) * sizeof(char));
    stringBuffer[size] = 0;

    int index = 0;
    while (mem[ptr])
    {
        stringBuffer[index++] = mem[ptr];

        ptr++;
    }

    return stringBuffer;
}


//function to find index of element in array
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
    clock_t beginTime = clock();

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
                if (isOnlyDigits(attr)) lineBuffer[lineIndex].attrInt = atoi(attr);
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
            

            else if (strcmp(inst, "got") == 0) instNum = got;
            else if (strcmp(inst, "jm0") == 0) instNum = jm0;
            else if (strcmp(inst, "jmA") == 0) instNum = jmA;
            else if (strcmp(inst, "jmG") == 0) instNum = jmG;
            else if (strcmp(inst, "jmL") == 0) instNum = jmL;
            else if (strcmp(inst, "jmS") == 0) instNum = jmS;


            else if (strcmp(inst, "ret") == 0)      instNum = ret;            
            else if (strcmp(inst, "pha") == 0)      instNum = pha;
            else if (strcmp(inst, "pla") == 0)      instNum = pla;
            else if (strcmp(inst, "brk") == 0)      instNum = brk;
            else if (strcmp(inst, "clr") == 0)      instNum = clr;
            else if (strcmp(inst, "putstr") == 0)   instNum = putstr;
            else if (strcmp(inst, "ahm") == 0)      instNum = ahm;
            else if (strcmp(inst, "fhm") == 0)      instNum = fhm;
            else if (strcmp(inst, "plugin") == 0)   instNum = plugin;

            else
            {
                printf("Error, invalid command: %s", inst);
                return 0;


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




    //SDL stuff
    SDL_Event sdlEvent;
    SDL_Window* sdlWindow       = NULL;
    SDL_Renderer* sdlRenderer   = NULL;
    SDL_Init(SDL_INIT_VIDEO);




    //now finally run with lightspeed
    unsigned short int acc = 0;
    unsigned short int reg = 0;

    unsigned short int mem[MEM_SIZE]     = { 0 };
    unsigned short int stack[STACK_SIZE] = { 0 };
    int stackPtr = 0;

    CHUNK chunkList[CHUNK_LIST_SIZE] = { 0 };

    char outputBuffer[2] = "";
    FILE* filePluginPtr = NULL;

    bool isRunning = 1;
    int intAttr;

    for (int execPtr = 0; execPtr < lineCount && isRunning; execPtr++)
    {
        LINE lineStruct = lineBuffer[execPtr];

        runInst = lineStruct.inst;
        runAttr = lineStruct.attr;
        intAttr = lineStruct.attrInt;

        switch (runInst)
        {
            case set:
                reg = intAttr;
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
                acc = mem[intAttr];
                break;

            case lDR:
                reg = mem[intAttr];
                break;

            case sAD:
                mem[intAttr] = acc;
                break;

            case sRD:
                mem[intAttr] = reg;
                break;


            case lPA:
                acc = mem[mem[intAttr]];
                break;

            case lPR:
                reg = mem[mem[intAttr]];
                break;

            case sAP:
                mem[mem[intAttr]] = acc;
                break;

            case sRP:
                mem[mem[intAttr]] = reg;
                break;

            case out:
                printf("%d\n", mem[intAttr]);
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
                isRunning = 0;
                continue;

            case clr:
                acc = 0;
                reg = 0;
                break;

            case putstr:
                

                outputBuffer[0] = (char)acc;
                printf("%s", &outputBuffer);
                break;

            case ahm:
                //iterate the chunks to find place to fit new chunk

                printf("START AHM\n");
                printChunkList(chunkList);

                for (int i = 0; i < ArraySize(chunkList) - 1; i++)
                {
                    CHUNK iter = chunkList[i + 0];
                    CHUNK next = chunkList[i + 1];

                    unsigned int allocSize = reg;
                    unsigned int endIter = (iter.ptr + iter.size);
                    unsigned int iter2NextIntervalSize = next.ptr - endIter;
                    //printf("iter2NextIntervalSize: %d\n", iter2NextIntervalSize);

                    //check if fitting interval has been found
                    bool foundAlloc = 0;

                    //special case for index 0, becasue that index is never test to unalloced
                    if (!iter.isAlloced && !i)
                    {
                        iter.isAlloced = 1;
                        iter.ptr       = 0;
                        iter.size      = allocSize;

                        //override acc with base ptr
                        acc = (1 << 15);

                        foundAlloc = 1;
                    }
                    //check if i == 0 and the current chunk is alloced and the current pointer is not 0, meaning there is space infront of the first chunk
                    else if (!i && iter.isAlloced && iter.ptr)
                    {
                        printf("Found space at 0 infornt of the first chunk\n");

                        foundAlloc = 1;
                    }

                    //normal case for if the next element is unalloced
                    else if (!next.isAlloced)
                    {
                        printf("found space at %d\n", i);
                        next.isAlloced  = 1;
                        next.ptr        = endIter;
                        next.size       = allocSize;

                        //override acc with base ptr, add offset 
                        acc = endIter | (1 << 15);
                        
                        foundAlloc = 1;
                    }
                    else if (iter2NextIntervalSize > allocSize)
                    {
                        printf("deez nuts\n");
                        foundAlloc = 1;
                    }

                    chunkList[i + 0] = iter;
                    chunkList[i + 1] = next;

                    if (foundAlloc) break;


                }

                printf("END AHM\n");
                printChunkList(chunkList);
                printf("\n");

                break;

            //case statements are cursed (in c at least) 
            case fhm: ;
                unsigned int freeSize = reg;
                unsigned int freeBase = acc & ~(1 << 15);

                printf("START FHM\n");
                printChunkList(chunkList);


                //find the give chunk in the chunckList and reset it to be unallocated
                bool foundMatching = 0;
                for (int findMatchingIndex = 0; findMatchingIndex < ArraySize(chunkList); findMatchingIndex++)
                {
                    if (chunkList[findMatchingIndex].isAlloced &&
                        chunkList[findMatchingIndex].ptr == freeBase &&
                        chunkList[findMatchingIndex].size == freeSize)
                    {
                        //the list needs to stay in sorted order, all elements with a higher index need to be moved one back
                        //untill an unallocated element is reached, to keep the array in sorted order and with not spaces

                        //this is done be copying the next element to the current element's cell and than stepping forward
                        //this also has the effect of clearing the current cell

                        for (int moveIndex = findMatchingIndex; moveIndex < ArraySize(chunkList) - 1; moveIndex++)
                        {
                            chunkList[moveIndex].ptr = chunkList[moveIndex + 1].ptr;
                            chunkList[moveIndex].size = chunkList[moveIndex + 1].size;
                            chunkList[moveIndex].isAlloced = chunkList[moveIndex + 1].isAlloced;

                            //if the end of alloced chunks is found, it's not need to continue, that would just move empty cell around
                            if (!chunkList[moveIndex].isAlloced) break;
                        }

                        foundMatching = 1;
                        break;
                    }
                }
                

                //throw error if no result has been found
                if (!foundMatching) printf("Error: no matching chunk was found, while trying to free\n");

                printf("END FHM\n");
                printChunkList(chunkList);
                printf("\n");

                break;

            case plugin:
                //im mot parsing the plugins, because they should not be called as often as normal commands

                    if (strcmp(runAttr, "File::Read") == 0)
                {
                    unsigned short int ptr = _PopStack(stack, &stackPtr);
                    printf("ptr: %d\n", ptr);

                    char* path = _GetString(mem, ptr);
                    printf("path: %s\n", path);

                    filePluginPtr = fopen(path, "rb");
                    if (filePluginPtr == NULL)
                    {
                        fprintf(stderr, "Error while loading file via plugin\n");
                        return 1;
                    }
                }
                else if (strcmp(runAttr, "File::Buffer2Stack") == 0)
                {
                    if (filePluginPtr != NULL)
                    {
                        stack[stackPtr++] = (unsigned int)fgetc(filePluginPtr);
                    }

                }

                else if (strcmp(runAttr, "Screen::WinInit") == 0)
                {
                    unsigned int windowHeight = _PopStack(stack, &stackPtr);
                    unsigned int windowWidth = _PopStack(stack, &stackPtr);

                    SDL_CreateWindowAndRenderer(windowWidth, windowHeight, 0, &sdlWindow, &sdlRenderer);
                }
                else if (strcmp(runAttr, "Screen::WinExit") == 0)
                {
                    while (1) {
                        if (SDL_PollEvent(&sdlEvent) && sdlEvent.type == SDL_QUIT) break;
                        Sleep(1);
                    }
                }
                else if (strcmp(runAttr, "Screen::Draw") == 0)
                {
                    unsigned int windowIndexY = _PopStack(stack, &stackPtr);
                    unsigned int windowIndexX = _PopStack(stack, &stackPtr);
                    unsigned int windowB      = _PopStack(stack, &stackPtr);
                    unsigned int windowG      = _PopStack(stack, &stackPtr);
                    unsigned int windowR      = _PopStack(stack, &stackPtr);

                    SDL_SetRenderDrawColor(sdlRenderer, windowR, windowG, windowB, 255);
                    SDL_RenderDrawPoint(sdlRenderer, windowIndexX, windowIndexY);
                }
                else if (strcmp(runAttr, "Screen::Update") == 0)
                {
                    SDL_RenderPresent(sdlRenderer);
                }



                break;

            default:
                break;
        }



    }


    clock_t endTime = clock();
    double timeSpent = (double)(endTime - beginTime) / CLOCKS_PER_SEC;
    
    printf("Time spent: %lf", timeSpent);

    return 0;
}


