#include "basedados.h"

//Dado pelo docente
#define STRING char *
#define MAX_LINHA_FICHEIRO 150

typedef char LinhaTexto[MAX_LINHA_FICHEIRO];

LinhaTexto LT;
//Termina aqui


//Função dada pelo docente para ler "strings"
int mygetline(char line[], int max){
    int nch = 0, c;
    max = max - 1; /* espaço for '\0'*/
    while((c = getchar()) != EOF){

        if(c == '\n')break;

        if(nch < max){
            line[nch] = c;
            nch = nch + 1;}
        }
    if(c == EOF && nch == 0) return EOF;

    line[nch] = '\0';
    return nch;
}

// Função dada pelo docente para ler ficheiros
STRING *Read_Split_Line_File(FILE *f, int n_campos_max, int *n_campos_lidos, char *separadores){
    *n_campos_lidos = 0;
    if (!f) return NULL;
    if (fgets(LT , MAX_LINHA_FICHEIRO , f) != NULL)
    {
        STRING *Res = (STRING *)malloc(n_campos_max*sizeof(STRING));
        char *pch = strtok (LT, separadores);
        int cont = 0;
        while (pch != NULL)
        {
            Res[cont] = (char *)malloc((strlen(pch)+1)*sizeof(char));
            strcpy(Res[cont++], pch);
            pch = strtok (NULL, separadores);
        }
        *n_campos_lidos = cont;
        return Res;
    }
    return NULL;
};

//Função para saber se a BD está vazia
int isEmpty(){
    int value = 0;

    /** return values:
     * 0 -> file does not exist
     * 1 -> file is empty
     * 2 -> file is not empty
    */
    FILE *f = fopen("livros.txt", "r");

    if(!f){
        fclose(f);
        return 0;
    } 

    fseek(f, 0L, SEEK_END);

    if(ftell(f) == 0)
        value = 1;
    else
        value = 2;

    fclose(f);
    return value;
}


ptrNodeLivro criarLivro(){
    ptrNodeLivro livro = (ptrNodeLivro)malloc(sizeof (struct nodeLivro));
    livro->data = malloc(sizeof(struct livro));
    livro->next = NULL;
    return livro;
}

ptrListLivro criarListaLivros(){
    ptrListLivro lista = (ptrListLivro)malloc(sizeof(struct listaLivro));
    lista->inic = NULL;
    lista->NEL = 0;
    return lista;
}

void lerLivro(ptrNodeLivro livro){
    if(!livro) return;
    printf("Insira o código: ");
    scanf("%d", &livro->data->cod_livro);
    printf("Insira o preço: ");
    scanf("%f", &livro->data->preco);
    getchar();
    printf("Insira o título do livro: ");
    mygetline(livro->data->titulo, MAXMSG);
    printf("Insira o nome autor do livro: ");
    mygetline(livro->data->autor, MAXMSG);
}

int carregarBaseDados(ptrListLivro lista){
    if(!lista) return 0;

    int n_campos_max = 20;
    int n_campos_lidos;
    int n_linhas_lidas = 0;

    FILE *F1 = fopen("livros.txt","r");

    if (!F1) {
        printf("\n\n\tImpossivel abrir Ficheiro \n\n");
        return 0;
    }

    while(!feof(F1)){
        ptrNodeLivro L = criarLivro();
        STRING *V = Read_Split_Line_File(F1, n_campos_max, &n_campos_lidos, "\t\n");

        for (int i = 0; i < n_campos_lidos; i++){
            L->data->cod_livro = atoi(V[0]);
            L->data->preco = atof(V[3]);
            strcpy(L->data->autor, V[1]);
            strcpy(L->data->titulo, V[2]);
            break;
        }
        for (int i = 0; i < n_campos_lidos; i++)
            free (V[i]);

        addLivroInic(L, lista);
        free (V);
    }
    fclose(F1);
    return 1;
}

int gravarBiblioteca(ptrListLivro lista){
    if(!lista) return 0;

    FILE *f = fopen("livros.txt", "w");

    ptrNodeLivro atual = lista->inic;

    while(atual != NULL){
        if(atual->next != NULL)
            fprintf(f, "%d\t%s\t%s\t%.2f\n", atual->data->cod_livro, atual->data->autor, atual->data->titulo, atual->data->preco);
        else
            fprintf(f, "%d\t%s\t%s\t%.2f", atual->data->cod_livro, atual->data->autor, atual->data->titulo, atual->data->preco);
        atual = atual->next;
    }

    fclose(f);
    return 1;
}

//Incializa a lista do servidor
void inicServ(CLIENTE arr[]){
    for(int i = 0; i < MAXUSERS; i ++) arr[i].pid_cliente = 0;
}

//Adiciona um cliente ao servidor
int addToServ(CLIENTE arr[], CLIENTE cliente){
    for(int i = 0; i < MAXUSERS; i ++){
        if(arr[i].pid_cliente == 0){
            arr[i].pid_cliente = cliente.pid_cliente;
            return 1;
        }
    }
    //Se não for possível adicionar cliente 
    return 0;
}

//Remove um cliente da lista do servidor
int remFromServ(CLIENTE arr[], long pid){
    for(int i = 0; i < MAXUSERS; i ++){
        if(arr[i].pid_cliente == pid){
            arr[i].pid_cliente = 0;
            return 1;
        }
    }
    return 0;
}

//Termina todos os processos ativos no servidor
void finishServer(CLIENTE arr[]){
    for(int i = 0; i < MAXUSERS; i ++){
        if(arr[i].pid_cliente != 0){
            printf("Cliente [%ld] terminou\n", arr[i].pid_cliente);
            kill(arr[i].pid_cliente, SIGTERM);
        }
    }
}

void showOnServ(CLIENTE arr[]){
    printf("----LIST OF CLIENTS-----\n");
    for(int i = 0; i < MAXUSERS; i ++){
        printf("Cliente [%d] -> [%ld]\n", i, arr[i].pid_cliente);
    }
}

int addLivroInic(ptrNodeLivro livro, ptrListLivro lista){
    if(!livro || !lista) return 0;

    if(lista->inic == NULL){
        lista->NEL ++;
        lista->inic = livro;
        livro->next = NULL;
        return 1;
    }

    livro->next = lista->inic;
    lista->inic = livro;
    lista->NEL ++;
    return 1;
}

void destruirLivro(ptrNodeLivro livro){
    free(livro->data);
    free(livro);
}

void destruirLista(ptrListLivro lista){
    if(!lista) return;

    ptrNodeLivro atual = lista->inic;

    while(atual->next){
        destruirLivro(atual->next);
        atual = atual->next;
    }

    destruirLivro(lista->inic);
    free(lista);
}

void showPtrLivro(ptrNodeLivro livro){
    printf("\tCódigo: [%d]\n", livro->data->cod_livro);
    printf("\tAutor: [%s]\n", livro->data->autor);
    printf("\tTitulo: [%s]\n", livro->data->titulo);
    printf("\tPreço: [%.2f]€\n", livro->data->preco);
}

void showLivro(LIVRO livro){
    printf("\tCódigo: [%d]\n", livro.cod_livro);
    printf("\tAutor: [%s]\n", livro.autor);
    printf("\tTitulo: [%s]\n", livro.titulo);
    printf("\tPreço: [%.2f]€\n", livro.preco);
}

ptrNodeLivro procurarLivro(int cod, ptrListLivro lista){
    if(!lista) return NULL;

    ptrNodeLivro atual = lista->inic;

    while(atual != NULL && atual->data->cod_livro != cod){
        atual = atual->next;
    }

    return atual;
}

void showLista(ptrListLivro lista){
    if(!lista) return;
    int i = 1;

    ptrNodeLivro atual = lista->inic;

    while(atual != NULL){
        printf("Livro [%d]\n", i);
        showPtrLivro(atual);
        atual = atual->next;
        ++i;
    }
}

