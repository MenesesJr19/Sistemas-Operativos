#ifndef BASEDADOS_H
#define BASEDADOS_H

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define exit_on_error(s,m) if (s < 0) { perror(m); exit(1); }
#define PERMISSOES 0666 /* permissões para outros utilizadores */
#define MAXMSG 250 /* assumir que chega */
#define MAXUSERS 15
#define CHAVE_SHM 23

typedef struct livro{
    int cod_livro;
    char autor[MAXMSG];
    char titulo[MAXMSG];
    float preco;
}LIVRO, *ptrLivro;

typedef struct nodeLivro{
    ptrLivro data;
    struct nodeLivro *next;
}NODELIVRO, *ptrNodeLivro;

typedef struct listaLivro{
    int NEL; //número de elementos na lista;
    ptrNodeLivro inic;
}LISTALIVRO, *ptrListLivro;

typedef struct cliente{
    long pid_cliente;
}CLIENTE;

typedef struct s_msg{
    long para; /* 1º campo: long obrigatório = PID destinatário*/
    int de; /* PID remetente*/
    char texto[MAXMSG];
    LIVRO livroPretendido;
}S_MSG;

typedef struct s_shm{
    CLIENTE onServ[MAXUSERS];
}S_SHM;

ptrListLivro criarListaLivros();
ptrNodeLivro criarLivro();
int addLivroInic(ptrNodeLivro livro, ptrListLivro lista);
void showLista(ptrListLivro lista);
int carregarBaseDados(ptrListLivro lista);
void lerLivro(ptrNodeLivro livro);
int mygetline(char line[], int max);
int gravarBiblioteca(ptrListLivro lista);
ptrNodeLivro procurarLivro(int cod, ptrListLivro lista);
void showLivro(LIVRO livro);
void showPtrLivro(ptrNodeLivro livro);
void showOnServ(CLIENTE arr[]);
void inicServ(CLIENTE arr[]);
int addToServ(CLIENTE arr[], CLIENTE cliente);
int remFromServ(CLIENTE arr[], long pid);
void finishServer(CLIENTE arr[]);
int isEmpty();
void destruirLivro(ptrNodeLivro livro);
void destruirLista(ptrListLivro lista);
#endif