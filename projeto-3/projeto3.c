#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

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

int main () {

    FILE *fileRegistros;
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
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            default:
                break;
        }
    } while (resposta != 5);
}
