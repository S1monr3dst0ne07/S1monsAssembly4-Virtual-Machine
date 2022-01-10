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


#define ArraySize(x) sizeof(x) / sizeof(x[0])

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS




typedef struct 
{
    //original command
    char instOrg[128];
    char attrOrg[128];

    //parsed command
    int inst;
    char attr[ATTR_SIZE];

    //parsed attr, if attr can be stored as int
    unsigned short int attrInt;

    //error feedback
    int lineNum;
    int conLine;

} LINE;




typedef struct chunklist
{
    unsigned int ptr;
    unsigned int size;
    struct chunklist* next;

} CHUNKLIST;


CHUNKLIST* findSpace(unsigned short int neededSpace, CHUNKLIST* startChunk)
{

    CHUNKLIST* iter = startChunk;

    while (true)
    {
        //check for end of list
        if (iter->next == NULL) break;
        
        //check for space in between two chunks
        unsigned int iterEndPtr = iter->ptr + iter->size;
        unsigned int spaceIterEnd2Next = (iter->next->ptr) - iterEndPtr;
        
        if (spaceIterEnd2Next > neededSpace) break;

        iter = iter->next;
    }

    return iter;


}

//finds the chunk that fits ptr and size, and returns the previouse in the list
CHUNKLIST* findPrevChunkByPtrAndSize(unsigned int ptr, unsigned int size, CHUNKLIST* startChunk)
{
    CHUNKLIST* iter = startChunk;
    CHUNKLIST* last = NULL;

    while (iter)
    {
        if (iter->ptr == ptr && iter->size == size) return last; 
        last = iter;
        iter = iter->next;
    }
    
    return NULL;
}

//recursive because lazy, bruh
void printChunkList(CHUNKLIST* startChunk)
{
    if (startChunk)
    {
        printf("    Ptr:    %d\n", startChunk->ptr);
        printf("    Size:   %d\n", startChunk->size);
        printf("\n");

        printChunkList(startChunk->next);

    }
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

            strcpy(lineBuffer[lineIndex].instOrg, inst);

            lineBuffer[lineIndex].attrInt = 0;
            if (attr != NULL)
            {
                if (isOnlyDigits(attr))
                {
                    lineBuffer[lineIndex].attrInt = atoi(attr);
                }
                strcpy(lineBuffer[lineIndex].attr, attr);
                strcpy(lineBuffer[lineIndex].attrOrg, attr);
            }
            else
            {
                memset(lineBuffer[lineIndex].attr, 0, ATTR_SIZE);
                lineBuffer[lineIndex].attrOrg[0] = NULL;
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

    CHUNKLIST* startChunk = (CHUNKLIST*)malloc(sizeof(CHUNKLIST));
    startChunk->ptr = 0;
    startChunk->size = 0;
    startChunk->next = NULL;

    char outputBuffer[2] = "";
    FILE* filePluginPtr = NULL;

    bool isRunning = 1;
    unsigned short int intAttr;

    for (int execPtr = 0; execPtr < lineCount && isRunning; execPtr++)
    {
        LINE lineStruct = lineBuffer[execPtr];

        runInst = lineStruct.inst;
        runAttr = lineStruct.attr;
        intAttr = lineStruct.attrInt;

        //printf(": %s %s\n", lineStruct.instOrg, lineStruct.attrOrg);

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
                break;

            case not:
                acc = ~acc;
                break;

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

            //case statements are cursed (in c at least) 
            case ahm: ;
                unsigned short int allocSize = reg;
                
                CHUNKLIST *insertBase = findSpace(allocSize, startChunk);
                CHUNKLIST *newChunk = (CHUNKLIST*)malloc(sizeof(CHUNKLIST));

                //insert the new chunk into the link list
                unsigned int newChunkBasePtr = insertBase->ptr + insertBase->size;
                newChunk->ptr =  newChunkBasePtr;
                newChunk->size = allocSize;
                newChunk->next = insertBase->next;

                //link the new chunk as the next for the insertBase, to fully insert the new chunk and override the last next (which now is the next->next)
                insertBase->next = newChunk;

                acc = newChunkBasePtr | (1 << 15);

                //printChunkList(startChunk);
                //printf("end\n");

                break;

            case fhm: ;
                unsigned short int freeSize = reg;
                unsigned short int freeBaseRaw = acc;
                unsigned short int freeBase = freeBaseRaw & ~(1 << 15);

                //the previouse is needed to delete the next chunk from the list, because the next pointer of the previouse need to be overritten to exclude the chunk for the list
                CHUNKLIST* foundPreviouseChunk = findPrevChunkByPtrAndSize(freeBase, freeSize, startChunk);
                CHUNKLIST* foundChunk = NULL;

                if (foundPreviouseChunk != NULL)
                {
                    foundChunk = foundPreviouseChunk->next;

                    //exclude the foundChunk for the list, so it can be freed
                    foundPreviouseChunk->next = foundChunk->next;
                    free(foundChunk);

                    //clear memory
                    for (int i = 0; i < freeSize; i++) mem[freeBaseRaw + i] = 0;


                }
                else
                {
                    printf("Error: Chunk (ptr: %d, size: %d) could not be found", freeBase, freeSize);
                }

                //printChunkList(startChunk);
                //printf("end\n");

                break;

            case plugin:
                //im mot parsing the plugins, because they should not be called as often as normal commands

                    if (strcmp(runAttr, "File::Read") == 0)
                {
                    unsigned short int ptr = _PopStack(stack, &stackPtr);
                    char* path = _GetString(mem, ptr);

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


