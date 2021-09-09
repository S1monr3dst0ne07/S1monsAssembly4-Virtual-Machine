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
    lDA,


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


    //scan lines into array
    int lineIndex = 0;
    char *inst, *attr, *attrNew;
    char attrBuffer[ATTR_BUFFER_SIZE];
    while (fgets(buffer, FILE_BUFFER_SiZE - 1, filePtr))
    {
        buffer[strcspn(buffer, "\n")] = 0;
        if (buffer[0] != 0 && buffer[0] != '"')
        {
            inst = strtok(buffer, " ");
            attr = strtok(NULL, " ");

            if (attr != NULL)
            {
                attrNew = malloc(sizeof(attr) + sizeof(char));
                memset(attrBuffer, 0, sizeof(attrNew));
                strcpy(attrNew, attr, sizeof(attr));

                lineBuffer[lineIndex].attr = attrNew;
            }
            else
            {
                lineBuffer[lineIndex].attr = NULL;
            }



            int instNum = NULL;

            if (strcmp(inst, "lDA") == 0)
            {
                instNum = lDA;
            }

            else
            {
                instNum = invaild;
            }



            lineBuffer[lineIndex].inst = instNum;
            lineBuffer[lineIndex].lineNum = lineIndex;


            lineIndex++;
        }
    }

    for (int i = 0; i < lineCount; i++)
    {
        
        printf("%d\n", lineBuffer[i].inst);
        printf("%s\n", lineBuffer[i].attr);
    }




    printf("Done");
    return 0;
}