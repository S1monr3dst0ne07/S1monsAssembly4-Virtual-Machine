#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FILE_BUFFER_SiZE 256
#define ATTR_BUFFER_SIZE 2048

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

typedef struct 
{
    int inst;
    char* attr;
    int lineNum;

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
    int attrBufferIndex = 0;
    char *inst, *attr, *attrNew;
    char* attrBuffer[ATTR_BUFFER_SIZE];

    while (fgets(buffer, FILE_BUFFER_SiZE - 1, filePtr))
    {
        buffer[strcspn(buffer, "\n")] = 0;
        if (buffer[0] != 0 && buffer[0] != '"')
        {
            inst = strtok(buffer, " ");
            attr = strtok(NULL, " ");

            if (attr != NULL)
            {
                attrNew = malloc(sizeof(attr));
                strcpy(attrNew, attr, sizeof(attr));
            }
            else
            {
                attrNew = NULL;
            }




            int instNum = NULL;

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
                attrBuffer[attrBufferIndex++] = attrNew;
            
                instNum = lab;
            }
            

            else if (strcmp(inst, "got") == 0)      instNum = got;
            else if (strcmp(inst, "jm0") == 0)      instNum = jm0;
            else if (strcmp(inst, "jmA") == 0)      instNum = jmA;
            else if (strcmp(inst, "jmG") == 0)      instNum = jmG;
            else if (strcmp(inst, "jmL") == 0)      instNum = jmL;
            else if (strcmp(inst, "jmS") == 0)      instNum = jmS;

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
            lineBuffer[lineIndex].attr = attrNew;
            lineBuffer[lineIndex].lineNum = lineIndex;


            lineIndex++;
        }
    }

    for (int i = 0; i < lineCount; i++)
    {
        
        //printf("%d\n", lineBuffer[i].inst);
        printf("%s\n", lineBuffer[i].attr);
    }


    for (int i = 0; i < attrBufferIndex; i++)
    {

        printf("Array: %s\n", attrBuffer[i]);
    }



    printf("Done");
    return 0;
}