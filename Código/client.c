#include "basedados.h"

struct s_msg msg;
struct s_shm *listServ;

//Variável para correr o programa
int run = 1;

long pid_child = 0;


//Lidar com Ctr+C
void sig_handlerKill(int sig){
    printf("\nCliente [%d] vai terminar e informou o servidor...\n", msg.de);
    remFromServ(listServ->onServ, msg.de);
    int id_msg = msgget(100, 0);
    strcpy(msg.texto, "goodbye");
    msgsnd(id_msg, (struct msgbuf *) &msg,sizeof(msg)-sizeof(long), 0);
    exit(0);
}

//O servidor manda o sinal SIGTERM para o cliente terminar
void sig_handlerTerm(int sig){
    printf("\n<-------------------------------------------------------------->\n");
    printf("\n-----> Servidor vai terminar. Terminar cliente [%d] <------\n", msg.de);
    printf("\n<-------------------------------------------------------------->\n");
    kill(msg.de, SIGKILL);
    kill(pid_child, SIGKILL);
    exit(0);
}

//Lidar com Ctrl+Z
void sig_handlerSuspend(int sig_num){
    char command[15];
    printf("\nInsira o comando a executar ");
    mygetline(command, 15);

    if(fork()){ //Processo pai
        wait(NULL);
        return;
    }else{ //Processo filho
        execlp(command, command, NULL);
    }
}


int main(int argc, char *argv[]){
    int chave, id_msg, r, codigo, id_shm;
    long para = 1;
    char *line=NULL;
    //Inicializa informação necessária
    msg.para = 1;
    msg.de = getpid();
    //Inicializa cliente para adicionar à lista no servidor
    CLIENTE client;
    client.pid_cliente = getpid();
    //Lidar com os sinais 
    signal(SIGTERM, sig_handlerTerm);
    signal(SIGTSTP, sig_handlerSuspend);

    // Obtem id_msg da fila
    id_msg=msgget(100, 0); 
	
    //Se o servidor não tiver sido iniciado
    if(id_msg == -1){
        printf("\n----->O servidor que tentou contactar não se encontra ativo<-----\n\n");
        exit(1);
    }
    
    //Aceder à lista de clientes ativos no serv
    id_shm = shmget(CHAVE_SHM, 0, 0);
    exit_on_error(id_shm, "Erro ao tentar aceder mem. partilhada\n");
    listServ = (struct s_shm *) shmat(id_shm, NULL, 0);

    //Adicionar o cliente à lista do servidor
    if(addToServ(listServ->onServ, client) == 0){
        printf("Servidor cheio! \n");
        exit(0);
    }

    //Mensagem de auxilio para indicar quando o processo acede pela 1a vez ao servidor
    strcpy(msg.texto, "hello");
    r=msgsnd(id_msg, (struct msgbuf *) &msg,sizeof(msg)-sizeof(long), 0);
    exit_on_error(r, "Erro ao mandar mensagem inicial");

    printf("\n=================\n");
    printf("Para terminar prima as teclas Ctr + C\n");
    printf("Para executar comandos primas as teclas Ctr + Z\n");
    printf("=================\n\n");

    if(fork()){
        signal(SIGINT, sig_handlerKill);
        msg.para = 1;
        msg.de = getpid();
        printf("Insira os códigos dos produtos e prima ENTER\n");
        while(run){
            r=mygetline(msg.texto, MAXMSG);
            r=msgsnd(id_msg, (struct msgbuf *) &msg,sizeof(msg)-sizeof(long), 0);
            exit_on_error(r, "Impossivel mandar mensagem");
        }
    }else{  //Processo para receber as mensagens
        signal(SIGTSTP, SIG_IGN);
        para = getppid();
        pid_child = getpid();
        while(1){
            r=msgrcv(id_msg, (struct msgbuf *) &msg, sizeof(msg)-sizeof(long),para,0);
            exit_on_error(r,"Erro na leitura de mensagem");
            printf("DATA: ");
            if(msg.livroPretendido.cod_livro == -1){
    		    printf("NO DATA AVAILABLE\n");
	        }else{
                showLivro(msg.livroPretendido);
            }
            printf("Cliente [%d]\n", getppid());
        } 
    }
    return 0;
}