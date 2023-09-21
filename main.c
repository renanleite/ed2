#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Definição da estrutura de dados

struct RegistroArquivoRemove { //Proveniente do arquivo Remove.bin
    char cod_cli[12];
    char cod_vei[8];
};

struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    char NumeroDias[5];    // +1 para o caractere nulo ('\0')
};

struct RegistroLocacao registros[100];  // TODO: Mudar para dentro da função, retornando do carrega arquivo
struct RegistroArquivoRemove remover[100]; //Verificar se vai fazer Array tamanho fixo ou Malloc


//Função para verificar se o aqruivo existe e caso não, ele irá criar e inicializar com -1, indicando que não há espaços vazios no arquivo
void criaArquivo(){
    FILE *arquivo;
    if(arquivo = fopen("registro.bin", "rb")){
        fclose(arquivo);
        return;
    }
    else{
        arquivo = fopen("registro.bin", "w+b");
        fwrite("-1", 1, sizeof(char), arquivo);
        fclose(arquivo);
    }
}

// Função para inserção de um registro no final do arquivo TODO: verificar tamanho disponivel antes de inserir
void inserirRegistro(struct RegistroLocacao registroInserir) {

    FILE *arquivo = fopen("registro.bin", "rb+");
    char tamanho, offset;

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    tamanho = 18 + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + 4;

    fread(&offset, sizeof(char), 1, arquivo);
    if(offset == -1){
        arquivo = fopen("registro.bin", "a+b");
    }
    else{
        offset = (tamanho);
        if(offset == -1){
            arquivo = fopen("registro.bin", "a+b");
        }
        else{
            fseek(arquivo, offset, SEEK_SET);
        }
    }

    fwrite(&tamanho, 1, sizeof(char), arquivo);
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

    printf("\n---Registro Inserido com sucesso---\n\n");
}

// Função para remover os registros TODO: precisamos remover o registro baseado na chave primaria (CodCli + CodVei)
void removerRegistro(char CodVei[], char CodCli[]) {
    FILE *arquivo = fopen("registro.bin", "rb+");

    char CodVeiRegistro[8], CodCliRegistro[12], tamanhoRegistro;
    bool RegistroEncontrado = false;
    int offset;

    fread(&offset, sizeof(int), 1, arquivo); //Lendo o offset inicial
    while(fread(&tamanhoRegistro, sizeof(char), 1, arquivo)){
        
        fread(&CodCliRegistro, sizeof(char), 8, arquivo);
        fseek(arquivo, 1, SEEK_CUR);
        fread(&CodVeiRegistro, sizeof(char), 12, arquivo);

        if((CodVei != CodVeiRegistro) && (CodCli != CodCliRegistro)){
            fseek(arquivo, tamanhoRegistro - 21, SEEK_CUR);
        }
        else{
            RegistroEncontrado = true;
            break;
        }
    }

    if(RegistroEncontrado){

        fseek(arquivo, -21, SEEK_CUR);
        offset = ftell(arquivo) - 1;
        offset = adicionaOffset(offset);
        
        fwrite("*", sizeof(char), 1, arquivo);
        fwrite(offset, sizeof(int), 1, arquivo);
        printf("\n---Registro Removido com sucesso---\n\n");
        
    }
    else{
        printf("\n---Registro Não Encontrado---\n\n");
    }

}

// Função para compactar o arquivo
void compactarArquivo() {

}

// Vai abrir o arquivo insere.bin e carregar todos os registros em um vetor de structs
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

    //Fazendo para Remove.bin
    arquivo = fopen("remove.bin", "rb");
    quantidadeRegistros = 0;

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    while(fread(&remover[quantidadeRegistros], sizeof(struct RegistroArquivoRemove), 1, arquivo)){
        quantidadeRegistros++;
    }
    fclose(arquivo);
}

void lerArquivo() {
    FILE *arquivo = fopen("registro.bin", "rb");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    fclose(arquivo);
}

//Adiciona um novo Offset a "pilha"
int adicionaOffset(int posicaoRemovido){
    FILE *arquivo = fopen("registro.bin", "rb+");
    int offset = 0, offsetAnterior;

    while(offset != -1){
        offsetAnterior = offset;
        fseek(arquivo, offset + 2, SEEK_SET); //Anda o tamanho +1 do tamanho +1 do *
        fread(&offset, sizeof(int), 1, arquivo);
    }

    rewind(arquivo);
    fwrite(posicaoRemovido, sizeof(int), 1, arquivo);
    return offsetAnterior;

}
 
//Busca um espaço para inserção de registro e atualiza a "pilha" de Offsets
int buscaEspaço(char tamanho){
    
    FILE *arquivo = fopen("registro.bin", "rb+");
    int offset = 0, offsetAnterior;
    char espaçoLivre = 0;

    while((offset != -1) && (espaçoLivre < tamanho)){
        offsetAnterior = offset;

        fseek(arquivo, offset, SEEK_SET);
        fread(espaçoLivre, sizeof(char), 1, arquivo);
        fseek(arquivo, 1, SEEK_CUR);
        fread(&offset, sizeof(int), 1, arquivo);

    }

    if(offset == -1){
        return -1;
    }
    else{
        //Atualiza o offset do começo
        rewind(arquivo);
        fwrite(offsetAnterior, sizeof(int), 1, arquivo);
        return offsetAnterior;
    }


}
// Função para administrar o menu
void menu(){
    int resposta, posicao;

    do{
        printf("Escolha uma opção:\n");
        printf("(1)Inserir\n(2)Remover\n(3)Compactar\n(4)Carregar Arquivos\n(5)Ler Arquivo\n(6)Sair\n");
        scanf("%d", &resposta);

        switch (resposta) {
            case 1:
                //system("clear");
                printf("Qual o registro deseja inserir?\n");
                scanf("%d", &posicao);
                inserirRegistro(registros[posicao]);
            case 2:
                printf("Qual o registro deseja remover?\n");
                scanf("%d", &posicao);
                removerRegistro(remover[posicao].cod_vei, remover[posicao].cod_cli);
                break;
            case 3:
                compactarArquivo();
                break;
            case 4:
                carregarArquivo();
                break;
            case 5:
                lerArquivo();
                break;
            case 6:
            break;
            default:
                printf("Valor inválido!\n");
        }
    }
    while(resposta != 6);
}

// Função main
int main() {
 
    criaArquivo();
    menu();
    return 0;
}
