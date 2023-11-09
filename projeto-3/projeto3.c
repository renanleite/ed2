#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXKEYS 3
#define NIL (-1)
#define NOKEY '@'
#define NO 0
#define YES 1

struct BTPage{
    short keycount; // number of keys in page
    char key[MAXKEYS]; // the actual keys
    short child[MAXKEYS+1]; // RRNs dos filhos
};

#define PAGESIZE sizeof(struct BTPage)

struct RegistroBusca {  //Proveniente do arquivo busca.bin
    char cod_cli[12];
    char cod_vei[8];
};

struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    char NumeroDias[4];
};

struct RegistroLocacao registrosInsercao[100];  // para armazenar o arquivo insere.bin
struct RegistroBusca registrosBusca[100];   // para armazenar o arquivo busca.bin

int imprimirMenu() {
    int resposta;

    printf("Escolha uma opção:\n");
    printf("(1) Inserir\n");
    printf("(2) Listar os dados de todos os clientes\n");
    printf("(3) Listar os dados de um cliente\n");
    printf("(4) Carregar arquivo\n");
    printf("(5) Sair\n");
    scanf("%d", &resposta);

    return resposta;
}

bool existeArquivoRegistro() {
    if(access("registros.bin", F_OK) == 0){
        return true;
    }
    return false;
}

void validarErroAbrirArquivo(FILE *arquivo) {
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }
}

void carregaArquivos(){
    //  Fazendo primeiro para 'insere.bin'
    FILE *arquivo = fopen("insere.bin", "rb");
    int quantidadeRegistros = 0;

    validarErroAbrirArquivo(arquivo);

    while(fread(&registrosInsercao[quantidadeRegistros], sizeof(struct RegistroLocacao), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);

    //  Fazendo para 'busca.bin'
    arquivo = fopen("busca.bin", "rb");
    quantidadeRegistros = 0;

    validarErroAbrirArquivo(arquivo);

    while(fread(&registrosBusca[quantidadeRegistros], sizeof(struct RegistroBusca), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);
}

// TODO: FUNCOES PARA A ARVORE, PRECISA TESTAR SE ESTA FUNCIONAL

bool btOpen(FILE *fileBTree) {
    if ((fileBTree = fopen("BTree.bin", "r+b")) == NULL) {
        return false;
    }
    return true;
}

short getRoot(FILE *fileBTree) {
    short root;
    fseek(fileBTree, 0, SEEK_SET);
    if(!(fread(&root, sizeof(short), 1, fileBTree))) {
        printf("Erro ao ler raiz");
        exit(1);
    }
    return root;
}

short getpageRRN(FILE *fileBTree) {
    short addr;
    fseek(fileBTree, 0, SEEK_END);
    addr = ftell(fileBTree) - sizeof(short);
    return ((short) addr / PAGESIZE);
}

void pageInit(struct BTPage *p_page) {
    // TODO
}

int createTree(FILE *fileBTree) {
    int menosum = NIL;

    if ((fileBTree = fopen("BTree.bin", "w+b")) == NULL) {
        printf("Erro ao abrir arquivo");
        return 0;
    }

    fseek(fileBTree, 0, SEEK_SET);
    fwrite(&menosum, sizeof(int), 1, fileBTree); // header = -1

    return 0;
    // TODO: fazer o retorno parar criar a raiz (create_root)
}

void inserirRegistro(FILE *registros, FILE *fileBTree) {

    // TODO: terminar funcao inserir
    int posicao, root;
    printf("Qual registro deseja inserir?\n");
    scanf("%d", &posicao);
    posicao --; // para converter, comecando em zero

    // Inserindo o registro no final do arquivo principal:
    fseek(registros, 0, SEEK_END);
    fwrite(&registrosInsercao[posicao], sizeof(struct RegistroLocacao), 1, registros);

    if(btOpen(fileBTree))
        root = getRoot(fileBTree);

    else
        root = createTree(fileBTree);
}

int main () {

    FILE *fileRegistros, fileBTree;
    int resposta;

    // Se o arquivo principal não existe, criar o arquivo para escrita:
    if(!existeArquivoRegistro()) {
        if ((fileRegistros = fopen("registros.bin","w+b")) == NULL) {
            printf("Erro ao criar arquivo");
            return 0;
        }
    }
    // Caso exista, abrir para leitura:
    else {
        if((fileRegistros = fopen("registros.bin","r+b")) == NULL) {
            printf("Erro ao abrir arquivo");
            return 0;
        }
    }

    do {
        resposta = imprimirMenu();

        switch (resposta) {
            case 1:
                inserirRegistro(fileRegistros, &fileBTree);
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                carregaArquivos();
                break;
            default:
                break;
        }
    } while (resposta != 5);
}
