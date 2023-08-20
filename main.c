#include <stdio.h>
#include <string.h>

// Definição da estrutura de dados
struct RegistroLocacao {
    char CodCli[12];
    char CodVei[8];
    char NomeCliente[51];
    char NomeVeiculo[51];
    int NumeroDias;
};

// Função para inserção de um registro no final do arquivo TODO: verificar tamanho disponivel antes de inserir
void inserirRegistro() {

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
    }

    struct RegistroLocacao registros[100];
    int numRegistros = 0;

    while (!feof(arquivo)) {
        char tamanhoRegistro;
        if (fread(&tamanhoRegistro, sizeof(unsigned char), 1, arquivo) != 1) {
            break; // Fim do arquivo
        }

        char linha[256];
        if (fgets(linha, tamanhoRegistro + 1, arquivo) == NULL) {
            break; // Fim do arquivo ou erro
        }

        linha[strcspn(linha, "\n")] = '\0'; // TODO: verificar se pode tirar essa linha

        if (strlen(linha) != tamanhoRegistro) {
            // TODO: Dados inconsistentes, talvez tratar erro
            continue;
        }

        // Armazena o valor do registro no vetor de structs
        sscanf(linha, "%11[^|]|%7[^|]|%49[^|]|%49[^|]|%d|",
               registros[numRegistros].CodCli,
               registros[numRegistros].CodVei,
               registros[numRegistros].NomeCliente,
               registros[numRegistros].NomeVeiculo,
               &registros[numRegistros].NumeroDias);

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
                inserirRegistro();
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
