#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct RegistroArquivoBusca { //Proveniente do arquivo Busca_p.bin
    char cod_cli[12];
    char cod_vei[8];
};

struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    char NumeroDias[4];    // +1 para o caractere nulo ('\0')
};

#define Não_Encontrado -1;
#define Não_Pareado -2;

struct RegistroLocacao registros[100];
struct RegistroArquivoBusca registrosBusca[100];
struct RegistroArquivoBusca indices[100];
int totalRegistrosCarregados;
int quantidadeIndices;

//Cria os arquivos e verifica se o Indice está pareado
void criaArquivo(){
    FILE *arquivoBusca;
    FILE *arquivo = fopen("registro.bin", "a+b");

    if ((arquivo == NULL)) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    if(arquivoBusca = fopen("indice.bin", "rb")){
        fclose(arquivoBusca);
        return;
    }
    else{
        arquivoBusca = fopen("indice.bin", "w+b");
        fwrite('N', 1, sizeof(char), arquivoBusca);
        fclose(arquivoBusca);
    }
}

//Verifica se há pareamento, nesse caso retorna true
bool verificaPareamento(){
    FILE *arquivoBusca = fopen("indices.bin", "rb");
    char pareamento;
    fread(&pareamento, sizeof(char), 1, arquivoBusca);
    if(pareamento != 'P'){
        return false;
    }
    else{
        return true;
    }
}

//Pesquisa o Indice e retorna a posicao dele caso o arquivo esteja
int buscarRegistro(char codCli[],char codVei[]){

    FILE *arquivoBusca = fopen("indice.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12], tamanhoRegistro, pareamento;
    int posicaoAtual, posicaoRegistro = Não_Encontrado;

    fread(&pareamento, sizeof(char), 1, arquivoBusca);
    if(pareamento != 'P'){
        return Não_Pareado;
    }
    while(fread(&codCliRegistro, sizeof(char), 12, arquivoBusca)){
        fseek(arquivoBusca, 1, SEEK_CUR);
        fread(&codVeiRegistro, sizeof(char), 8, arquivoBusca);

        if((codVei != codVeiRegistro) && (codCli != codCliRegistro)){ 

            posicaoAtual++;
            fseek(arquivoBusca, 1, SEEK_CUR);

        }
        else{
            posicaoRegistro = posicaoAtual;
            break;
        }
    }

    fclose(arquivoBusca);
    return posicaoRegistro;

}

//Função para ordenar os indices e em seguida o arquivo de Dados
void ordenaArquivos(){

}

void inserirRegistro(struct RegistroLocacao registroInserir) {
    
    FILE *arquivo = fopen("registro.bin", "a+b");
    FILE *arquivoBusca = fopen("indice.bin", "rb+");
    char tamanhoDoRegistro;
    int offset;
    int sizeCodCliCodVei = 18; 

    if ((arquivo == NULL) || (arquivoBusca == NULL)) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fwrite('N', 1, sizeof(char), arquivoBusca); //Indica que o Arquivo de Indices não está pareado durante a inserção

    tamanhoDoRegistro = sizeCodCliCodVei + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + sizeof(int);

    fwrite(registroInserir.CodCli, 1, strlen(registroInserir.CodCli), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.CodVei, 1, strlen(registroInserir.CodVei), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.NomeCliente, 1, strlen(registroInserir.NomeCliente), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.NomeVeiculo, 1, strlen(registroInserir.NomeVeiculo), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.NumeroDias, 1, sizeof (int), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);

    fclose(arquivo);

    //Adiciona o Indice a Memória
    quantidadeIndices++;
    strcpy(indices[quantidadeIndices].cod_cli, registroInserir.CodCli);
    strcpy(indices[quantidadeIndices].cod_vei, registroInserir.CodVei);
    ordenaArquivos();

    printf("\n---Registro Inserido com sucesso---\n\n");
}

// Vai abrir os arquivos e carregar todos os registros em um vetor de structs
void carregarArquivo() {
    //Fazendo primeiro para Insere.bin
    FILE *arquivo = fopen("insere.bin", "rb");
    int quantidadeRegistros = 0;

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    while(fread(&registros[quantidadeRegistros], sizeof(struct RegistroLocacao), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);

    totalRegistrosCarregados = quantidadeRegistros;

    //Fazendo para Busca.bin
    arquivo = fopen("busca_p.bin", "rb");
    quantidadeRegistros = 0;

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    while(fread(&registrosBusca[quantidadeRegistros], sizeof(struct RegistroArquivoBusca), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);
}


// Função para exibir o menu
int imprimirMenu() {
    int resposta;

    printf("Escolha uma opção:\n");
    printf("(1)Inserir\n(2)Buscar\n(3)Carregar Arquivos\n(4)Sair\n");
    scanf("%d", &resposta);

    return resposta;
}

// Função para administrar o menu
void menu(){
    int resposta, posicao;

    do{
        resposta = imprimirMenu();

        switch (resposta) {
            case 1:
                system("clear");

                printf("Qual o registro deseja inserir?\n");
                scanf("%d", &posicao);

                if(posicao < totalRegistrosCarregados){
                    inserirRegistro(registros[posicao]);
                }
                else{
                    printf("\n---Valor Inválido de registro---\n\n");
                }
                break;
            case 2:
                system("clear");

                printf("Qual registro deseja buscar?\n");
                scanf("%d", &posicao);

                if(posicao < totalRegistrosCarregados){
                    buscarRegistro(registrosBusca[posicao].cod_cli, registrosBusca[posicao].cod_vei);
                }
                else{
                    printf("\n---Valor Inválido de registro---\n\n");
                }
                break;
            case 3:
                carregarArquivo();
                break;
            case 4:
                system("clear");

                printf("\n---Finalizando o programa!!!---\n\n");
                break;
            default:
                system("clear");

                printf("\n---Valor Inválido---\n\n");
        }
    }
    while(resposta != 4);
}

// Função main
int main() {
    criaArquivo();
    menu();
    return 0;
}