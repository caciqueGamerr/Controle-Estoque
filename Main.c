#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define TAM_BUFFER 100

typedef struct {
    char **nomes;
    int *qtds;
    int n;
    int cap;
} Estoque;

void limpar_buffer(){
    int ch;
    while((ch = getchar()) != '\n' && ch != EOF);
}

void ler_linha(char texto[], int tamanho){
    if(fgets(texto, tamanho, stdin) == NULL){
        texto[0] = '\0';
        return;
    }
    if(strchr(texto, '\n') == NULL){
        limpar_buffer();
    }
    texto[strcspn(texto, "\r\n")] = '\0';
}

int ler_inteiro(){
    int n, ok;
    char next;

    ok = scanf("%d", &n);
    if(ok == EOF){
        exit(0);
    }
    next = getchar();
    if(ok != 1){
        limpar_buffer();
        return 0;
    }
    if(next != '\n'){
        limpar_buffer();
        return 0;
    }
    return n;
}

// Copia um texto para um espaço novo da heap
char *copiar_texto(char texto[]){
    char *copia = malloc(strlen(texto) + 1);    // +1 para o '\0'
    if(copia == NULL){
        return NULL;
    }
    strcpy(copia, texto);
    return copia;
}

// Reserva a memória inicial do estoque. calloc deixa tudo zerado (NULL / 0)
int iniciar(Estoque *e){
    e->n = 0;
    e->cap = 4;
    e->nomes = calloc(e->cap, sizeof(char *));
    e->qtds = calloc(e->cap, sizeof(int));

    if(e->nomes == NULL || e->qtds == NULL){
        free(e->nomes);                         // free(NULL) é permitido e não faz nada
        free(e->qtds);
        return 0;
    }
    return 1;
}

// Devolve para o sistema tudo que foi alocado
void liberar(Estoque *e){
    for(int i = 0; i < e->n; i++){
        free(e->nomes[i]);                     
    }
    free(e->nomes);                             
    free(e->qtds);
}

// Coloca um item na memória. Se o espaço acabou, aumenta com realloc
int adicionar_memoria(Estoque *e, char nome[], int qtd){
    if(e->n == e->cap){
        int novo = e->cap * 2;

        // Guarda o resultado do realloc em outra variável: se falhar ele devolve NULL
        char **novos_nomes = realloc(e->nomes, novo * sizeof(char *));
        if(novos_nomes == NULL){
            return 0;
        }
        e->nomes = novos_nomes;

        int *novas_qtds = realloc(e->qtds, novo * sizeof(int));
        if(novas_qtds == NULL){
            return 0;
        }
        e->qtds = novas_qtds;
        e->cap = novo;
    }

    char *copia = copiar_texto(nome);
    if(copia == NULL){
        return 0;
    }
    e->nomes[e->n] = copia;
    e->qtds[e->n] = qtd;
    e->n++;
    return 1;
}

// Tira o item da posição pos: libera o nome e puxa os de baixo uma posição para cima
void remover_posicao(Estoque *e, int pos){
    free(e->nomes[pos]);
    for(int i = pos; i < e->n - 1; i++){
        e->nomes[i] = e->nomes[i + 1];          
        e->qtds[i] = e->qtds[i + 1];
    }
    e->n--;
    e->nomes[e->n] = NULL;                      
}

// Lê o inventory.txt e coloca os itens na memória (uma vez, no começo do programa)
void carregar(Estoque *e){
    FILE *arq = fopen("inventory.txt", "r");
    char nome[TAM_BUFFER];
    char linha[TAM_BUFFER];

    if(arq == NULL){                            // arquivo não existe (estoque vazio)
        return;
    }
    while(fgets(nome, sizeof nome, arq) != NULL){
        nome[strcspn(nome, "\r\n")] = '\0';
        if(fgets(linha, sizeof linha, arq) == NULL){
            break;
        }
        if(!adicionar_memoria(e, nome, atoi(linha))){
            printf("Erro: memória insuficiente ao ler o estoque.\n");
            break;
        }
    }
    fclose(arq);
}

// Atualiza o arquivo com os dados que estão na memória.
int salvar(Estoque *e){
    FILE *arq = fopen("inventory.txt", "w");
    if(arq == NULL){
        printf("Erro ao abrir o arquivo inventory.txt\n");
        return 0;
    }
    for(int i = 0; i < e->n; i++){
        fprintf(arq, "%s\n%d\n", e->nomes[i], e->qtds[i]);
    }
    fclose(arq);
    return 1;
}

int main(){
    setlocale(LC_ALL, "Portuguese");

    char accept = 's';
    int op, qtd, pos;
    char nome[TAM_BUFFER];
    char resposta[20];
    Estoque estoque;

    if(!iniciar(&estoque)){
        printf("Erro: memória insuficiente.\n");
        return 1;
    }
    carregar(&estoque);

    do{
        printf("=================================\n");
        printf("   Controle de Estoque\n");
        printf("=================================\n");
        printf("Selecione uma opção:\n");
        printf("1. Adicionar Item\n");
        printf("2. Remover Item\n");
        printf("3. Listar Estoque\n");
        printf("4. Sair\n");
        printf("Opção: ");

        op = ler_inteiro();
        while(op < 1 || op > 4){
            printf("Opção inválida! Tente novamente.\n");
            printf("Opção: ");
            op = ler_inteiro();
        }

        if(op == 4){
            break;
        }

        switch(op){
            case 1:
                printf("Digite o nome do item: ");
                ler_linha(nome, TAM_BUFFER);

                printf("Digite a quantidade: ");
                qtd = ler_inteiro();
                while(qtd < 1){
                    printf("Quantidade inválida. Digite um número inteiro positivo.\n");
                    printf("Digite a quantidade: ");
                    qtd = ler_inteiro();
                }

                if(adicionar_memoria(&estoque, nome, qtd)){
                    if(salvar(&estoque)){
                        printf("Item adicionado com sucesso!\n");
                    }
                }
                else{
                    printf("Erro: memória insuficiente.\n");
                }
            break;

            case 2:
                printf("Digite o nome do item: ");
                ler_linha(nome, TAM_BUFFER);

                printf("Digite a quantidade a ser removida: ");
                qtd = ler_inteiro();
                while(qtd < 1){
                    printf("Quantidade inválida. Digite um número inteiro positivo.\n");
                    printf("Digite a quantidade a ser removida: ");
                    qtd = ler_inteiro();
                }

                // procura o item pelo nome
                pos = -1;
                for(int i = 0; i < estoque.n; i++){
                    if(strcmp(estoque.nomes[i], nome) == 0){
                        pos = i;
                        break;
                    }
                }

                if(pos == -1){
                    printf("Item não encontrado.\n");
                }
                else if(qtd > estoque.qtds[pos]){
                    printf("Estoque insuficiente. Quantidade disponível: %d\n", estoque.qtds[pos]);
                }
                else{
                    estoque.qtds[pos] = estoque.qtds[pos] - qtd;

                    if(estoque.qtds[pos] == 0){
                        remover_posicao(&estoque, pos);
                        if(salvar(&estoque)){
                            printf("Item removido do estoque!\n");
                        }
                    }
                    else if(salvar(&estoque)){
                        printf("Quantidade atualizada com sucesso!\n");
                    }
                }
            break;

            case 3:
                if(estoque.n == 0){
                    printf("O estoque está vazio.\n");
                }
                else{
                    printf("==============================\n");
                    printf("        Estoque Atual\n");
                    printf("==============================\n");
                    for(int i = 0; i < estoque.n; i++){
                        if(i > 0){
                            printf("\n");
                        }
                        printf("Nome: %s\n", estoque.nomes[i]);
                        printf("Quantidade: %d\n", estoque.qtds[i]);
                    }
                }
            break;
        }

        printf("Deseja realizar outra operação? (s/n): ");
        ler_linha(resposta, sizeof resposta);
        if(strcmp(resposta, "s") == 0){
            accept = 's';
        }
        else{
            accept = 'n';
        }

    } while(accept == 's');

    liberar(&estoque);
    printf("Obrigado por usar o Controle de Estoques! Até a próxima.\n");
    return 0;
}