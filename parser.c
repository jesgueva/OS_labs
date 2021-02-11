//parses the input file into Process and event
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
const int SIZE = 20;



enum status {
    None,
    Ready,
    Running,
    Blocked,
    ReadySuspended,
    BlockedSuspended,
    Exit,
    New
};

struct process {
    int isChanged;
    enum status status;
    char id[3];
};


int main()
{
    int i;
    char* rch;
    char str[200];
    char LineInFile[40][300];
    int lineP, lineQ;
    char* sch;
    char* token;
    char tokenizedLine[10][10];

    static const struct process EmptyStruct;
    //Initialize queue
    struct process *processArr[SIZE];
    struct process diskQueue[SIZE];
    struct process printerQueue[SIZE];
    struct process keyboardQueue[SIZE];


    FILE* fp1;
    FILE* fp2;
    fp1 = fopen("inp2.txt", "r");			//open the original input file
    fp2 = fopen("inp2_parsed.txt", "w");	//output the Process ID and event to another file.
    //You can store in variables instead of printing to file

    lineP = 0;
    i = 0;

    printf("Started parsing...\n");

    //copy first line of the original file to the new filefile
    if (fgets(str, sizeof(str), fp1) != NULL){
        int index = 0;
        int isProcess = 1;
        fprintf(fp2, "%s\n", str);
        token = strtok(str, " ");

        while( strcmp(token, "end\n")) {

            if(isProcess){

                struct process *t1 = malloc(sizeof(struct process));

                strcpy(t1->id, token);
                t1->isChanged = 0;
                t1->status = None;
                processArr[index] = t1;

                isProcess = 0;
            } else {
                if (!strcmp(token, "Ready"))
                    processArr[index]->status = Ready;
                else if (!strcmp(token, "Running"))
                    processArr[index]->status = Running;
                else if (!strcmp(token, "Blocked"))
                    processArr[index]->status = Blocked;
                else if (!strcmp(token, "Ready/Suspended"))
                    processArr[index]->status = ReadySuspended;
                else if (!strcmp(token, "Blocked/Suspended"))
                    processArr[index]->status = BlockedSuspended;
                else if (!strcmp(token, "New"))
                    processArr[index]->status = New;
                else
                    return -1;
                isProcess = 1;
                index++;
            }

            token = strtok(NULL, " ");
        }


    }


    //parse each remaining line into Process event
    //while loop with fgets reads each line
    while (fgets(str, sizeof(str), fp1) != NULL)
    {
        fprintf(fp2,"%s\n", str);
        lineP = 0;
        rch = strtok(str, ":;.");					// use strtok to break up the line by : or . or ; This would separate each line into the different events
        while (rch != NULL)
        {
            strcpy(LineInFile[lineP], rch);			//copy the events into an array of strings
            lineP++;								//keep track of how many events are in that line
            rch = strtok(NULL, ":;.");				//needed for strtok to continue in the while loop
        }


        //for each event (e.g. Time slice for P7 expires) pull out process number and event
        for (i = 1; i < lineP - 1; i++)
        {
            lineQ = 0;
            sch = strtok(LineInFile[i], " ");
            while (sch != NULL)
            {
                strcpy(tokenizedLine[lineQ], sch);		//use strtok to break up each line into separate words and put the words in the array of strings
                lineQ++;								//count number of valid elements
                sch = strtok(NULL, " ");
            }

            //tokenizedLine has the event separated by spaces (e.g. Time slice for P7 expires)
            if (strcmp(tokenizedLine[1], "requests") == 0)						//Process requests an I/O device
            {
                //update the status of the process to blocked
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[0], processArr[i]->id) == 0){
                        processArr[i]->isChanged = 1;
                        processArr[i]->status = Blocked;
                        if((strcmp(tokenizedLine[3], "disk")) == 0){
                            diskQueue[i] = *processArr[i];
                            break;
                        }
                        else if((strcmp(tokenizedLine[3], "printer")) == 0){
                            printerQueue[i] = *processArr[i];
                            break;
                        }
                        else if((strcmp(tokenizedLine[3], "keyboard")) == 0){
                            keyboardQueue[i] = *processArr[i];
                            break;
                        }
                    }
                }
            }
            else if ((strcmp(tokenizedLine[2], "dispatched") == 0))				//Process is dispatched
            {
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[0], processArr[i]->id) == 0){
                        processArr[i]->isChanged = 1;
                        processArr[i]->status = Running;
                        break;
                    }
                }
            }
            else if (strcmp(tokenizedLine[0], "Time") == 0)						//Process has timed off
            {
                //PROCESS UPDATE BACK to ready
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[3], processArr[i]->id) == 0){
                        processArr[i]->isChanged = 1;
                        processArr[i]->status = Ready;
                        break;
                    }
                }
            }
            else if (strcmp(tokenizedLine[3], "out") == 0)						//Process is swapped out
            {
                //move blocked/suspend
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[0], processArr[i]->id) == 0){
                        if(processArr[i]->status == Ready){
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = ReadySuspended;
                        } else if (processArr[i]->status == Blocked) {
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = BlockedSuspended;
                        }
                        break;
                    }
                }

            }
            else if (strcmp(tokenizedLine[3], "in") == 0)						//Process is swapped in
            {
                // check current state and activate
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[0], processArr[i]->id) == 0){
                        if(processArr[i]->status == ReadySuspended ) {
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = Ready;
                        }
                        else if(processArr[i]->status == BlockedSuspended ) {
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = Blocked;
                        }
                        break;
                    }
                }
            }
            else if (strcmp(tokenizedLine[1], "interrupt") == 0)				//An interrupt has occured
            {   //check if state is blocked/sus or blocked
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[4], processArr[i]->id) == 0){
                        if(processArr[i]->status == Blocked){
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = Ready;
                        } else if (processArr[i]->status == BlockedSuspended) {
                            processArr[i]->isChanged = 1;
                            processArr[i]->status = ReadySuspended;
                        }

                        if (!strcmp(diskQueue[i].id, processArr[i]->id) ){
                            diskQueue[i] = EmptyStruct;
                        }
                        else if(!strcmp(printerQueue[i].id,processArr[i]->id)){
                            printerQueue[i] = EmptyStruct;
                        }
                        else if(!strcmp(keyboardQueue[i].id,processArr[i]->id)){
                            keyboardQueue[i] = EmptyStruct;
                        }
                        break;
                    }
                }
            }
            else																//Process has been terminated
            {   // turn processes to exit
                for(int i=0;i<SIZE;i++){
                    if(strcmp(tokenizedLine[0], processArr[i]->id) == 0){
                            processArr[i]->isChanged = 1;
//                            free(processArr[i]->id);
                            processArr[i]->status = Exit;

                        break;
                    }
                }
            }

        }
        int j = 0;
        while(processArr[j] != NULL) {
            char status[10];
            switch(processArr[j]->status) {

                case 0:
                    strcpy(status, "None");
                    break;

                case 1:
                    strcpy(status, "Ready");
                    break;

                case 2:
                    strcpy(status, "Running");
                    break;
                case 3:
                    strcpy(status, "Blocked");
                    break;
                case 4:
                    strcpy(status, "Ready/Suspended");
                    break;
                case 5:
                    strcpy(status, "Blocked/Suspended");
                    break;
                case 6:
                    strcpy(status, "Exit");
                    break;
                case 7:
                    strcpy(status, "New");
                    break;
            }

            fprintf(fp2,"%s %s", processArr[j]->id, status);
            if(processArr[j]->isChanged)
                fprintf(fp2,"*");
                processArr[j]->isChanged = 0;
                fprintf(fp2," ");
            j++;
        }

        fprintf(fp2,"\nDisk Queue: ");
        for(int z = 0; z < SIZE; z++) {
            if (diskQueue[z].status != None) {
                fprintf(fp2,"%s ", diskQueue[z].id);
            }
        }
        fprintf(fp2,"\nPrinter Queue: ");
        for(int i = 0; i < SIZE;i++){
            if (printerQueue[i].status != None) {
                fprintf(fp2,"%s ", printerQueue[i].id);
            }
        }
        fprintf(fp2,"\nKeyboard Queue: ");
        for(int z = 0; z < SIZE; z++) {
            if (keyboardQueue[z].status != None) {
                fprintf(fp2,"%s ", keyboardQueue[z].id);
            }
        }
        fprintf(fp2,"\n\n");
    }


    printf("\n\n");


    printf("Parsing complete\n\n");

    fclose(fp1);
    fclose(fp2);

    return 0;
}