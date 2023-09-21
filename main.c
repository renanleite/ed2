#include <stdio.h>
#include <string.h>

// Definição da estrutura de dados
struct RegistroLocacao {
    char CodCli[12];  // +1 para o caractere nulo ('\0')
    char CodVei[8];   // +1 para o caractere nulo ('\0')
    char NomeCliente[50];  // +1 para o caractere nulo ('\0')
    char NomeVeiculo[50];  // +1 para o caractere nulo ('\0')
    char NumeroDias[5];    // +1 para o caractere nulo ('\0')
};

struct RegistroLocacao registros[100];  // TODO: Mudar para dentro da função, retornando do carrega arquivo

// Função para inserção de um registro no final do arquivo TODO: verificar tamanho disponivel antes de inserir
void inserirRegistro(struct RegistroLocacao registroInserir) {

    FILE *arquivo = fopen("registro.bin", "a+b");
    char tamanho, ponteiro;

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }
    int teste = -1;
    fwrite(&teste, 1, sizeof(int), arquivo);

    tamanho = 18 + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + 4;

    fread(&ponteiro, sizeof(int), 1, arquivo);
    if(ponteiro == -1){
        arquivo = fopen("registro.bin", "a+b");
    }
    else{
        fread(&ponteiro, sizeof(char), 1, arquivo);
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
void removerRegistro() {

}

// Função para compactar o arquivo
void compactarArquivo() {

}

// Vai abrir o arquivo insere.bin e carregar todos os registros em um vetor de structs
void carregarArquivo() {
    FILE *arquivo = fopen("insere.bin", "rb");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    int numRegistros = 0;

    while (fread(&registros[numRegistros], sizeof(struct RegistroLocacao), 1, arquivo) == 1) {
        numRegistros++;
    }
    fclose(arquivo);
}

// Função para administrar o menu
void menu(FILE *file){
    int resposta;
    do{
        printf("Escolha uma opção:\n");
        printf("(1)Inserir\n(2)Remover\n(3)Compactar\n(4)Carregar Arquivo\n(5)Sair\n");
        scanf("%d", &resposta);

        switch (resposta) {
            case 1:
                system("clear");
                inserirRegistro(registros[1]);  // Usando um registro de exemplo para testar, mudar depois
                break;
            case 2:
                removerRegistro();
                break;
            case 3:
                compactarArquivo();
                break;
            case 4:
                carregarArquivo();
                break;
            default:
                printf("Valor inválido!\n");
        }
    }
    while(resposta != 5);
}

// Função main
int main() {
    FILE *file = fopen("registros.bin", "a+b"); // Abre o arquivo para leitura e escrita binária

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }
 
    menu(file);
    fclose(file);
    return 0;
}
