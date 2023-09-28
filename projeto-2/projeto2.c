#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NAOENCONTRADO -1;
#define NAOPAREADO -2;

bool estarPareado();
int buscarRegistro(char [],char []);
int imprimirMenu();
void menu();
void criarArquivoVerificarIndice();
void criaIndice();
void ordenarIndices();
void copiaDadosDoIndice();
void montaArrayIndiceOrdenaArquivo();
void carregarArquivoInsere();
void carregarArquivoBusca();
void exibeRegistro(int);
void validarErroAbrirArquivo(FILE *);

struct RegistroArquivoBusca { //Proveniente do arquivo Busca_p.bin
    char cod_cli[12];
    char cod_vei[8];
};

struct RegistroIndice {
    char chave[19];
    int posicao;
};

struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    int NumeroDias;
};

struct RegistroLocacao registros[100];
struct RegistroArquivoBusca registrosBusca[100];
struct RegistroIndice indices[100];
int totalRegistrosCarregados;
int quantidadeIndices = 0;

int main() {
    criarArquivoVerificarIndice();
    menu();
    return 0;
}

void inserirRegistro(struct RegistroLocacao registroInserir) {
    FILE *arquivo = fopen("registro.bin", "a+b");
    FILE *arquivoBusca;
    if((arquivoBusca = fopen("indice.bin", "rb+"))){
        fwrite("N", 1, sizeof(char), arquivoBusca); //Indica que o Arquivo de Indices não está pareado durante a inserção
        fclose(arquivoBusca);
    }

    char tamanhoDoRegistro;
    int sizeCodCliCodVei = 18; 

    validarErroAbrirArquivo(arquivo);

    tamanhoDoRegistro = sizeCodCliCodVei + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + sizeof(int) + 5;

    fwrite(&tamanhoDoRegistro, 1, sizeof(char), arquivo);
    fwrite(registroInserir.CodCli, 1, strlen(registroInserir.CodCli), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.CodVei, 1, strlen(registroInserir.CodVei), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.NomeCliente, 1, strlen(registroInserir.NomeCliente), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(registroInserir.NomeVeiculo, 1, strlen(registroInserir.NomeVeiculo), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);
    fwrite(&registroInserir.NumeroDias, 1, sizeof(int), arquivo);
    fwrite("|", 1, sizeof(char), arquivo);

    //Adiciona o Indice a Memória
    indices[quantidadeIndices].posicao = quantidadeIndices + 1;
    strcpy(indices[quantidadeIndices].chave , registroInserir.CodCli);
    strcat(indices[quantidadeIndices].chave , registroInserir.CodVei);
    quantidadeIndices++;
    ordenarIndices();

    fclose(arquivo);

    printf("\n---Registro Inserido com sucesso---\n\n");
}

//Pesquisa o Indice e retorna a posicao dele caso o arquivo esteja
int buscarRegistro(char codCli[],char codVei[]){

    FILE *arquivoBusca = fopen("indice.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12];
    int posicaoAtual, posicaoRegistro = NAOENCONTRADO;
    bool primeiro, segundo;

    char chaveBusca[19];
    strcpy(chaveBusca, codCli);
    strcat(chaveBusca, codVei);


    if(!estarPareado()){
        for(int i=0; i<quantidadeIndices;i++){
            primeiro = (strncmp(chaveBusca, indices[i].chave, 18) == 0);
            if(primeiro){
                posicaoRegistro = indices[i].posicao;
                break;
            }
        }
    }

    else{
        fseek(arquivoBusca, 1, SEEK_CUR); // Pula o pareamento que ja foi 
        while(fread(&codCliRegistro, sizeof(char), 11, arquivoBusca)){
            fread(&codVeiRegistro, sizeof(char), 8, arquivoBusca);
            fread(&posicaoAtual, sizeof(int), 1, arquivoBusca);

            primeiro = (strncmp(codVei,codVeiRegistro, 7) == 0);
            segundo = (strncmp(codCli,codCliRegistro, 11) == 0);

            if(primeiro && segundo) {
                posicaoRegistro = posicaoAtual;
                break;
            }
        }
    }

    fclose(arquivoBusca);
    return posicaoRegistro;
}

//Printa o registro desejado
void exibeRegistro(int posicao){

    if(posicao == -1){
        printf("\nNão foi possível encontrar o registro\n\n");
        return;
    }

    if(posicao == -2){
        printf("\nNão foi possível realizar a pesquisa em um indice não pareado\n\n");
        return;
    }

    char caracter = 'a', tamanho;
    struct RegistroLocacao aux;
    FILE *arquivo = fopen("registro.bin", "rb");

    for(int i=1; i<posicao; i++) {
        fread(&tamanho, sizeof(char), 1, arquivo);
        fseek(arquivo, tamanho, SEEK_CUR);
    }
    //Leitura dos registros Variaveis;
    fseek(arquivo, 1, SEEK_CUR);
    fread(&aux.CodCli, sizeof(char), 11, arquivo);
    aux.CodCli[11] = '\0';
    fseek(arquivo, 1, SEEK_CUR);
    fread(&aux.CodVei, sizeof(char), 7, arquivo);
    aux.CodVei[7] = '\0';

    fseek(arquivo, 1, SEEK_CUR);
    strcpy(aux.NomeCliente, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, arquivo);
        if(caracter != '|'){
            strncat(aux.NomeCliente, &caracter, 1);
        }
    }

    caracter = 'a'; //Reiniciando caracter
    strcpy(aux.NomeVeiculo, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, arquivo);
        if(caracter != '|'){
            strncat(aux.NomeVeiculo, &caracter, 1);
        }
    }

    fscanf(arquivo, "%d", &aux.NumeroDias);

    //printa o registro
    printf("\n----------------------------------------------------------------------\n");
    printf("%s | %s | %s | %s | %d", aux.CodCli, aux.CodVei, aux.NomeCliente, aux.NomeVeiculo, aux.NumeroDias);
    printf("\n----------------------------------------------------------------------\n");
    
    fclose(arquivo);
}

void criarArquivoVerificarIndice(){
    FILE *arquivo;
    FILE *arquivoIndice;
    char pareamento;

    if((arquivo = fopen("registro.bin", "rb"))){
        fseek(arquivo, 0, SEEK_END);
        int size = ftell(arquivo);
        if (size != 0) {
            if((arquivoIndice = fopen("indice.bin", "rb"))){
                bool pareado = estarPareado();

                //Verifica pareamento para copiar os indices ou montar a partir dos dados
                if(pareado){
                    copiaDadosDoIndice();
                }
                else{
                    montaArrayIndiceOrdenaArquivo();
                }
                fclose(arquivoIndice);
            }
            else{
                montaArrayIndiceOrdenaArquivo();
            }

            fclose(arquivo);
            return;
        }
    }
    else{
        arquivo = fopen("registro.bin", "w+b");
        fclose(arquivo);
    }
}

void montaArrayIndiceOrdenaArquivo(){

    FILE *arquivo = fopen("registro.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12], tamanhoRegistro;

    while(fread(&tamanhoRegistro, sizeof(char), 1, arquivo)){
        indices[quantidadeIndices].posicao = quantidadeIndices + 1;
        fread(&codCliRegistro, sizeof(char), 11, arquivo);
        fseek(arquivo, 1, SEEK_CUR);
        fread(&codVeiRegistro, sizeof(char), 7, arquivo);
        codVeiRegistro[7] = '\0';

        strcpy(indices[quantidadeIndices].chave , codCliRegistro);
        strcat(indices[quantidadeIndices].chave , codVeiRegistro);
        quantidadeIndices++;

        fseek(arquivo, tamanhoRegistro - 19, SEEK_CUR);
    }
    ordenarIndices();
}

void menu(){
    int resposta, posicao;
    do{
        resposta = imprimirMenu();
        switch (resposta) {
            case 1:
                system("clear");

                printf("Qual o registro deseja inserir?\n");
                scanf("%d", &posicao);
                posicao--;

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
                posicao--;

                if(posicao < totalRegistrosCarregados){
                    exibeRegistro(buscarRegistro(registrosBusca[posicao].cod_cli, registrosBusca[posicao].cod_vei));
                }
                else{
                    printf("\n---Valor Inválido de registro---\n\n");
                }
                break;

            case 3:
                carregarArquivoInsere();
                carregarArquivoBusca();
                break;

            case 4:
                system("clear");
                criaIndice();
                printf("\n---Finalizando o programa!!!---\n\n");
                break;

            default:
                system("clear");
                printf("\n---Valor Inválido---\n\n");
        }
    }
    while(resposta != 4);
}

int imprimirMenu() {
    int resposta;

    printf("Escolha uma opção:\n");
    printf("(1)Inserir\n(2)Buscar\n(3)Carregar Arquivos\n(4)Sair\n");
    scanf("%d", &resposta);

    return resposta;
}

void carregarArquivoInsere() {
    FILE *arquivo = fopen("insere.bin", "rb");
    int quantidadeRegistros = 0;

    validarErroAbrirArquivo(arquivo);

    while(fread(&registros[quantidadeRegistros], sizeof(struct RegistroLocacao), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);

    totalRegistrosCarregados = quantidadeRegistros;
}

void carregarArquivoBusca() {
    FILE *arquivo = fopen("busca_p.bin", "rb");
    int quantidadeRegistros = 0;

    validarErroAbrirArquivo(arquivo);

    while(fread(&registrosBusca[quantidadeRegistros], sizeof(struct RegistroArquivoBusca), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);
}

//Cria o Arquivo de Indice inserindo os dados do vetor
void criaIndice(){

    FILE *arquivoIndice = fopen("novo.bin", "w+b");

    fwrite("N", 1, sizeof(char), arquivoIndice); //Insere Não Pareado para caso ocorra alguma interrupção durante a inserção

    for(int i = 0; i < quantidadeIndices; i++){
        fwrite(indices[i].chave, 19, sizeof(char), arquivoIndice);
        fwrite(&indices[i].posicao, 1, sizeof(int), arquivoIndice);
    }

    rewind(arquivoIndice);
    fwrite("P", 1, sizeof(char), arquivoIndice); //Atualiza para Pareado ao fim da inserção
    fclose(arquivoIndice);

    remove("indice.bin");
    rename("novo.bin", "indice.bin");
}

void ordenarIndices(){
    int i,j;
    struct RegistroIndice aux;
    bool igualAoProximoCodigoCliente = false;

    //BubbleSort para ordenar as chaves
    for(i = 0; i < quantidadeIndices; i++){
        for(j = 0; j < quantidadeIndices -i -1; j++){

            igualAoProximoCodigoCliente = (strcmp(indices[j].chave, indices[j+1].chave) > 0);
            
            //CodCli é a chave principal da ordenação, se ambas forem iguais, é comparado então CodVei
            if(igualAoProximoCodigoCliente) {
                aux = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = aux;
            }
        }
    }
}

void copiaDadosDoIndice(){

    FILE *arquivoIndice = fopen("indice.bin", "rb");
    char chaveRegistro[19], tamanhoRegistro, pareamento;
    quantidadeIndices = 0;

    fread(&pareamento, sizeof(char), 1, arquivoIndice);
    while(fread(&chaveRegistro, sizeof(char), 19, arquivoIndice)){
        fread(&indices[quantidadeIndices].posicao, sizeof(int), 1, arquivoIndice);

        strcpy(indices[quantidadeIndices].chave , chaveRegistro);
        quantidadeIndices++;
    }

    fclose(arquivoIndice);
}

bool estarPareado(){
    FILE *arquivoIndice = fopen("indice.bin", "rb");
    char pareamento;
    fread(&pareamento, sizeof(char), 1, arquivoIndice);
    if(pareamento != 'P'){
        fclose(arquivoIndice);
        return false;
    }
    fclose(arquivoIndice);
    return true;
}

void validarErroAbrirArquivo(FILE *arquivo) {
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }
}