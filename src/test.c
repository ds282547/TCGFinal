#include "stdio.h"
#include "stdlib.h"
#include "string.h"
int counter = 0;
int compose(int n,int kk,int k,int s,int cb[]){
    for(int i=s;i<n;++i){
        cb[k-1] = i;
        if(k>1){
            compose(n,kk,k-1,i+1,cb);
        } else {
            for(int j=0;j<kk;++j){
                printf("%d ",cb[j]);
            }
            printf("\n");
            counter++;
        }
    }
}

int main(int argc, char const *argv[])
{
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    int n1,n2,n3;
    char str[]="7F+4";
    sscanf(str,"%1x%1x+%1x",&n1,&n2,&n3);
    printf("%d %d %d\n",n1,n2,n3);

    return 0;
    char currentGameID[50] = {0};

    enum Role{
        Play,
        Evil
    };



    int cb[3];
    compose(6,3,3,0,cb);

    printf("%d\n", counter);
    return;

    
    while(1){
        int i,j;
        char buf[500] = {0};
        char cmd[50] = {0};
        char cmdAtr[50] = {0};
        char gameid[50] = {0};

        char agent1[50] = {0};
        char agent2[50] = {0};

        scanf ("%[^\n]%*c", buf);
        switch(buf[0]){
            case '@':
            //For Command
                //skip space
                i=1;
                while(buf[i]==' ') {
                    if(buf[i]=='\0') break;
                    i++;
                }
                j=i;
                while(buf[j]!=' ' && buf[j]!='\0')
                    j++;
                
                memset(cmd, '\0', sizeof(cmd));
                strncpy(cmd,buf+i,j-i);

                if(strcmp(cmd,"login") == 0){
                    printf("@ login 0756078|ds282547 AAA(player) BBB(environment)\n");
                } else if(strcmp(cmd,"exit") == 0){
                    exit(1);
                }
            break;
            case '#':
            // For Comment
                i=1;
                while(buf[i]==' ') {
                    if(buf[i]=='\0') break;
                    i++;
                }
                j=i;
                while(buf[j]!=' ' && buf[j]!='\0')
                    j++;
                
                memset(gameid, '\0', sizeof(gameid));
                strncpy(gameid,buf+i,j-i);
                
                //printf("GameID = %s\n",gameid);
                i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                j=i;while(buf[j]!=' ' && buf[j]!='\0')j++;

                memset(cmd, '\0', sizeof(cmd));
                strncpy(cmd,buf+i,j-i);
                //printf("cmd = %s\n",cmd);

                if(strcmp(cmd,"?") == 0){
                    printf("Ask For Action\n",agent1,agent2);

                } else if(strcmp(cmd,"open") == 0){
                    i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                    j=i;while(buf[j]!=':' && buf[j]!='\0')j++;

                    memset(agent1, '\0', sizeof(agent1));
                    strncpy(agent1,buf+i,j-i);

                    //Skip :
                    i=++j;while(buf[i]!=' ' && buf[i]!='\0')i++;

                    memset(agent2, '\0', sizeof(agent2));
                    strncpy(agent2,buf+j,i-j);

                    printf("Hikechen : %s vs %s\n",agent1,agent2);

                    if(currentGameID[0] == '\0'){
                        strcpy(currentGameID,gameid);
                        printf("#%s open accpet\n",gameid);
                    } else {
                        printf("#%s open reject\n",gameid);
                    }
                
                } else if(strcmp(cmd,"close") == 0){
                    i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                    j=i;while(buf[j]!=' ' && buf[j]!='\0')j++;


                    memset(cmdAtr, '\0', sizeof(cmdAtr));
                    strncpy(cmdAtr,buf+i,j-i);
                
                    printf("Close Flag = %s\n",cmdAtr);
                } else {
                    //Action
                    printf("Action = %s\n",cmd);
                }

                

                


                

            break;
            // case '%':
            // System Info!

        }

        


       

    }
    return 0;
}
