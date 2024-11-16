#include "basedados.h"

/**
 * 25215 -> Guilherme Pedrinho
 * 20888 -> Francisco Meneses
*/

struct s_msg msg;
struct s_shm *listServ;
ptrListLivro biblioteca = NULL;

//Converter o ponteiro do livro escolhido para struct
LIVRO convert(ptrNodeLivro l2){
    LIVRO l1;

    if(!l2){
        l1.cod_livro = -1;
    }else {
        strcpy(l1.autor, l2->data->autor);
        strcpy(l1.titulo, l2->data->titulo);
        l1.preco = l2->data->preco;
        l1.cod_livro = l2->data->cod_livro;
    }

    return l1;
}

//Handles Ctr C
void sig_handler(int sig_num){    
    int status;
    //Aceder à fila
    int id_msg=msgget(100, 0);
    exit_on_error(id_msg, "Erro ao aceder à fila\n");

    //Terminar todos os processos ativos no servidor
    if(fork()) {
        //Espera pelo processo filho terminar
        wait(NULL);
        //Eliminar a fila
        id_msg = msgctl(id_msg, IPC_RMID, NULL);

        //Aceder mem. partilhada
        int id_shm = shmget(CHAVE_SHM, 0, 0);
        exit_on_error(id_msg, "Erro ao aceder à mem. partilhada\n");
        
        //Eliminar mem. partilhada
        id_shm = shmctl(id_shm, IPC_RMID, NULL);
    }else {
        //Função que termina os servidores ativos no servidor   
        finishServer(listServ->onServ);

        destruirLista(biblioteca);
        exit(0);       
    }
    printf("\nServidor terminou..\n");

    exit(0);
}



int main(int argc, char *argv[]){
    int id_msg, r, id_shm;
    long para;
    char op;
    ptrNodeLivro livroPretendido = NULL;
    signal(SIGINT, sig_handler);
    

    biblioteca = criarListaLivros();
    msg.de = getpid();

    //Criar uma lista de clientes ativos no Serv partilhada por todos os processos
    id_shm = shmget(CHAVE_SHM, sizeof(struct s_shm), IPC_CREAT | PERMISSOES);
    exit_on_error(id_shm, "Erro ao tentar criar/aceder a mem. partilhada");

    listServ = (struct s_shm *) shmat(id_shm, NULL, 0);

    //Inicializar array de utilizadores a zero's
    inicServ(listServ->onServ);

    //Cria fila
    id_msg=msgget(100, IPC_CREAT | IPC_EXCL | PERMISSOES);

    //Criação da BD
    if(isEmpty() == 1){ //Se o ficheiro se encontrar vazio
        printf("BD não existente. Criar? (s/n)\n");
        scanf(" %c", &op);
        if(op == 'n'){
            id_msg = msgctl(id_msg, IPC_RMID, NULL);
            exit(1);
        }
        do{
            fflush(stdout);
            ptrNodeLivro L = criarLivro();
            lerLivro(L);
            addLivroInic(L, biblioteca);
            printf("Adicionar livro? (s/n): ");
            scanf(" %c", &op);
        }while(op != 'n' && op != 'N');
        gravarBiblioteca(biblioteca);

    }else if(isEmpty() == 2){ //Se o ficheiro tiver informação
        carregarBaseDados(biblioteca);
        showLista(biblioteca);
        printf("Editar BD existente? (s/n)\n");
        scanf(" %c", &op);
        if(op == 's'){
            do{
                fflush(stdout);
                ptrNodeLivro L = criarLivro();
                lerLivro(L);
                addLivroInic(L, biblioteca);
                printf("Adicionar livro? (s/n): ");
                scanf(" %c", &op);
            }while(op != 'n' && op != 'N');
            
            gravarBiblioteca(biblioteca);
        }
    }
    printf("Servidor inicou...\n");

    while (1) {
        //Aguarda mensagens na fila
        para=1; 
        r=msgrcv(id_msg, (struct msgbuf *) &msg, sizeof(msg)-sizeof(long),para,0);
        exit_on_error(r, "Erro ao receber mensagem\n");
    
        if(strcmp("hello", msg.texto) == 0){ //Cliente entrou no servidor
            printf("Cliente [%d] iniciou\n", msg.de);
            //showOnServ(listServ->onServ);
        }else if(strcmp("goodbye", msg.texto) == 0){ //Cliente terminou o programa
            printf("Cliente [%d] terminou\n", msg.de);

        }else{ //Cliente pediu um código
            printf("Cliente [%d] solicitou código [%s]\n", msg.de, msg.texto);
            
            //Criar um processo filho para responder ao pedido
            if(fork()==0){
                livroPretendido = procurarLivro(atoi(msg.texto), biblioteca);
                //Converter o ponteiro para uma struct a ser enviada
                msg.livroPretendido = convert(livroPretendido);
                msg.para = msg.de;
                strcpy(msg.texto, "Here is your data");
                r=msgsnd(id_msg, (struct msgbuf *)&msg,sizeof(msg)-sizeof(long), 0);
                exit_on_error(r, "Erro ao enviar resposta");
                exit(0);
            }
        }
    } 

    return 0;
}