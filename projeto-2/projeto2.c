#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NAOENCONTRADO -1;
#define NAOPAREADO -2;

bool verificaPareamento();
void criarArquivoVerificarIndice();
void criaIndice();
int buscarRegistro(char [],char []);
void exibeRegistro(int);
void ordenaArquivos();
void carregarArquivo();
int imprimirMenu();
void menu();

struct RegistroArquivoBusca { //Proveniente do arquivo Busca_p.bin
    char cod_cli[12];
    char cod_vei[8];
};

struct RegistroIndice {
    char cod_cli[12];
    char cod_vei[8];
    int posicao;
};

struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    char NumeroDias[4];    // +1 para o caractere nulo ('\0')
};

struct RegistroLocacao registros[100];
struct RegistroArquivoBusca registrosBusca[100];
struct RegistroIndice indices[100];
int totalRegistrosCarregados;
int quantidadeIndices;

int main() {
    criarArquivoVerificarIndice();
    menu();
    return 0;
}

//Função para ordenar os indices
void ordenaArquivos(){

    int i,j;
    struct RegistroIndice aux;

    //BubbleSort para ordenar as chaves
    for(i = 0; i < quantidadeIndices; i++){
        for(j = 0; j < quantidadeIndices -i -1; j++){

            //CodCli é a chave principal da ordenação, se ambas forem iguais, é comparado então CodVei
            if(indices[j].cod_cli > indices[j+1].cod_cli){ 
                aux = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = aux;
            }
            else if((indices[j].cod_cli == indices[j+1].cod_cli) && (indices[j].cod_vei > indices[j+1].cod_vei)){
                aux = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = aux;
            }

        }
    }
}

//Copia dados do Indice
void copiaArrayIndice(){

    FILE *arquivoIndice = fopen("indice.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12], tamanhoRegistro, pareamento;
    quantidadeIndices = 0;

    fread(&pareamento, sizeof(char), 1, arquivoIndice);
    while(fread(&codCliRegistro, sizeof(char), 12, arquivoIndice)){
        fread(&codVeiRegistro, sizeof(char), 8, arquivoIndice);
        fseek(arquivoIndice, 1, SEEK_CUR);
        fread(&indices[quantidadeIndices].posicao, sizeof(int), 1, arquivoIndice);

        strcpy(indices[quantidadeIndices].cod_cli , codCliRegistro);
        strcpy(indices[quantidadeIndices].cod_vei , codVeiRegistro);
        quantidadeIndices++;
        fseek(arquivoIndice, 1, SEEK_CUR);
    }

    fclose(arquivoIndice);
}

bool isTamanhoRegistrado(char tamanhoRegistro, FILE *arquivo) {
    bool isTamanhoRegistrado = (fread(&tamanhoRegistro, sizeof(char), 1, arquivo)) != NULL;
    return isTamanhoRegistrado;
}

//Monta indices a partir dos Dados e depois ordena
void montaArrayIndice(){

    FILE *arquivo = fopen("registo.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12], tamanhoRegistro;

    //isTamanhoRegistrado(tamanhoRegistro, arquivo);

    while(fread(&tamanhoRegistro, sizeof(char), 1, arquivo)){
        indices[quantidadeIndices].posicao = ftell(arquivo);
        fread(&codCliRegistro, sizeof(char), 12, arquivo);
        fseek(arquivo, 1, SEEK_CUR);
        fread(&codVeiRegistro, sizeof(char), 8, arquivo);

        strcpy(indices[quantidadeIndices].cod_cli , codCliRegistro);
        strcpy(indices[quantidadeIndices].cod_vei , codVeiRegistro);
        quantidadeIndices++;

        fseek(arquivo, tamanhoRegistro - 19, SEEK_CUR);
    }
    ordenaArquivos();
}

void criarArquivoVerificarIndice(){
    FILE *arquivo;
    FILE *arquivoIndice;
    char pareamento;

    //TODO: Caso registro.bin esteja vazio, mesmo existindo. resolver o problema a seguir.
    if(arquivo = fopen("registro.bin", "rb")){
        if(arquivoIndice = fopen("indice.bin", "rb")){

            bool verifica = verificaPareamento();

            //Verifica pareamento para copiar os indices ou montar a partir dos dados
            if(verifica){
                copiaArrayIndice();
            }
            else{
                montaArrayIndice();
            }
        }
        else{
            montaArrayIndice();
        }

        fclose(arquivo);
        fclose(arquivoIndice);
        return;
    }
    else{
        arquivo = fopen("registro.bin", "w+b");
        fclose(arquivo);
    }
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

    fwrite("N", 1, sizeof(char), arquivoBusca); //Indica que o Arquivo de Indices não está pareado durante a inserção

    tamanhoDoRegistro = sizeCodCliCodVei + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + sizeof(int);

    indices[quantidadeIndices].posicao = ftell(arquivo);

    fwrite(&tamanhoDoRegistro, 1, sizeof(char), arquivo);
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

    fclose(arquivo);
    fclose(arquivoBusca);

    printf("\n---Registro Inserido com sucesso---\n\n");
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
                    exibeRegistro(buscarRegistro(registrosBusca[posicao].cod_cli, registrosBusca[posicao].cod_vei));
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


//Verifica se há pareamento, nesse caso retorna true
bool verificaPareamento(){
    FILE *arquivoIndice = fopen("indices.bin", "rb");
    char pareamento;
    fread(&pareamento, sizeof(char), 1, arquivoIndice);
    if(pareamento != 'P'){
        fclose(arquivoIndice);
        return false;
    }
    fclose(arquivoIndice);
    return true;
}

//Cria o Arquivo de Indice inserindo os dados do vetor
void criaIndice(){

    FILE *arquivoIndice = fopen("indice.bin", "w+b");

    fwrite("N", 1, sizeof(char), arquivoIndice); //Insere Não Pareado para caso ocorra alguma interrupção durante a inserção

    for(int i = 0; i < quantidadeIndices; i++){
        fwrite(indices[i].cod_cli, 12, sizeof(char), arquivoIndice);
        fwrite(indices[i].cod_vei, 8, sizeof(char), arquivoIndice);
        fwrite(" ", 1, sizeof(char), arquivoIndice);
        fwrite(indices[i].posicao, 1, sizeof(int), arquivoIndice);
    }

    rewind(arquivoIndice);
    fwrite("P", 1, sizeof(char), arquivoIndice); //Atualiza para Pareado ao fim da inserção
    fclose(arquivoIndice);

}

//Pesquisa o Indice e retorna a posicao dele caso o arquivo esteja
int buscarRegistro(char codCli[],char codVei[]){

    FILE *arquivoBusca = fopen("indice.bin", "rb");
    char codVeiRegistro[8], codCliRegistro[12], tamanhoRegistro, pareamento;
    int posicaoAtual, posicaoRegistro = NAOENCONTRADO;

    if(!verificaPareamento()){
        fclose(arquivoBusca);
        return NAOPAREADO;
    }
    fseek(arquivoBusca, 1, SEEK_CUR); // Pula o pareamento que ja foi 
    while(fread(&codCliRegistro, sizeof(char), 12, arquivoBusca)){
        fread(&codVeiRegistro, sizeof(char), 8, arquivoBusca);
        fseek(arquivoBusca, 1, SEEK_CUR);
        fread(&posicaoAtual, sizeof(int), 1, arquivoBusca);

        if((codVei != codVeiRegistro) && (codCli != codCliRegistro)){ 

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

//Printa o registro desejado
void exibeRegistro(int posicao){

    if(posicao == -1){
        printf("\nNão foi possível encontrar o registro\n");
        return;
    }

    char tamanhoRegistro;
    char caracter = 'a';
    struct RegistroLocacao aux;
    FILE *arquivo = fopen("registro.bin", "rb");

    fseek(arquivo, posicao, SEEK_SET);

    //Leitura dos registros Variaveis;
    while(caracter != '|'){
        fread(caracter, sizeof(char), 1, arquivo);
        if(caracter != '|'){
            strncat(aux.NomeCliente, caracter, 1);
        }
    }
    fseek(arquivo, 1, SEEK_CUR);
    caracter = 'a'; //Reiniciando caracter
    while(caracter != '|'){
        fread(caracter, sizeof(char), 1, arquivo);
        if(caracter != '|'){
            strncat(aux.NomeVeiculo, caracter, 1);
        }
    }

    fseek(arquivo, 1, SEEK_CUR);
    fread(&aux.NumeroDias, sizeof(int), 1, arquivo);

    //printa o registro
    printf("\n----------------------------------------------------------------------\n");
    printf("%s | %s | %s | %s | %d", aux.CodCli, aux.CodVei, aux.NomeCliente, aux.NomeVeiculo, aux.NumeroDias);
    printf("\n----------------------------------------------------------------------\n");
    
    fclose(arquivo);
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