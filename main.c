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
    char NumeroDias[4];    // +1 para o caractere nulo ('\0')
};

struct RegistroLocacao registros[100];  // TODO: Mudar para dentro da função, retornando do carrega arquivo
struct RegistroArquivoRemove registrosRemover[100]; //Verificar se vai fazer Array tamanho fixo ou Malloc


//Função para verificar se o arquivo existe e caso não, ele irá criar e inicializar com -1, indicando que não há espaços vazios no arquivo
void criaArquivo(){
    FILE *arquivo;
    int offsetVazio[] = {-1}; 

    if(arquivo = fopen("registro.bin", "rb")){
        fclose(arquivo);
        return;
    }
    else{
        arquivo = fopen("registro.bin", "w+b");
        fwrite(offsetVazio, 1, sizeof(int), arquivo);
        fclose(arquivo);
    }
}

// Função para inserção de um registro no final do arquivo TODO: verificar tamanho disponivel antes de inserir
void inserirRegistro(struct RegistroLocacao registroInserir) {

    //TODO: TRATAR CASO NÃO EXISTA O REGISTRO INSERIDO DA POSIÇÃO
    
    FILE *arquivo = fopen("registro.bin", "a+b");
    char tamanhoDoRegistro;
    int offset;
    int sizeCodCliCodVei = 18; 

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    tamanhoDoRegistro = 
        sizeCodCliCodVei + strlen(registroInserir.NomeCliente) + strlen(registroInserir.NomeVeiculo) + sizeof(int);

    fread(&offset, sizeof(char), 1, arquivo);

    //Verifica se irá inserir no final ou no meio, caso seja no final ele insere o tamanho do registro
    if(offset == -1){
        arquivo = fopen("registro.bin", "a+b");
        fwrite(&tamanhoDoRegistro, 1, sizeof(char), arquivo);
    }
    else{
        offset = (tamanhoDoRegistro);
        if(offset == -1){
            arquivo = fopen("registro.bin", "a+b");
            fwrite(&tamanhoDoRegistro, 1, sizeof(char), arquivo);
        }
        else{
            fseek(arquivo, offset, SEEK_SET);
        }
    }

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

        if((CodVei != CodVeiRegistro) && (CodCli != CodCliRegistro)){ //Verifica se encontrou o registro a ser removido

            fseek(arquivo, tamanhoRegistro - 21, SEEK_CUR); //Pula para o proximo registro

        }
        else{
            RegistroEncontrado = true;
            fseek(arquivo, -21, SEEK_CUR);
            break;
        }
    }

    if(RegistroEncontrado){  //Remove o registro

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

    FILE *arquivo = fopen("registro.bin", "rb");
    FILE *arquivoCompactado = fopen("novo.bin", "w+b");

    int offset, contadorPartes, quantidadeDados = 5;
    char tamanho, buffer[sizeof(struct RegistroLocacao)];

    fread(&offset, sizeof(char), 1, arquivo);

    while(fread(&tamanho, sizeof(char), 1, arquivo)){
        fread(buffer, sizeof(char), tamanho, arquivo);
        
        if(buffer[0] != '*'){ //Se o registro não foi removido entra no laço

            for (int i = 0; i < tamanho; i++){

                if(buffer[i] == '|'){
                    contadorPartes++;
                }

                if(contadorPartes == quantidadeDados){ //Verifica se o registro terminou para poder inserir, independente do tamanho descrito no original
                    tamanho = i;
                    fwrite(tamanho, sizeof(char), 1, arquivoCompactado);
                    fwrite(buffer, sizeof(char), tamanho, arquivoCompactado);
                    contadorPartes = 0;
                    break;
                }

            }

        }

    }

    //Deleta o arquivo fragmentado e substitui pelo novo
    fclose(arquivo);
    fclose(arquivoCompactado);
    remove("registro.bin");
    rename("novo.bin", "registro.bin");

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

    while(fread(&registrosRemover[quantidadeRegistros], sizeof(struct RegistroArquivoRemove), 1, arquivo)){
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
int buscaEspaco(char tamanho){
    
    FILE *arquivo = fopen("registro.bin", "rb+");
    int offset = 0, offsetAnterior;
    char espacoLivre = 0;

    while((offset != -1) && (espacoLivre < tamanho)){
        offsetAnterior = offset;

        fseek(arquivo, offset, SEEK_SET);
        fread(espacoLivre, sizeof(char), 1, arquivo);
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

int imprimirMenu() {
    int resposta;

    printf("Escolha uma opção:\n");
    printf("(1)Inserir\n(2)Remover\n(3)Compactar\n(4)Carregar Arquivos\n(5)Ler Arquivo\n(6)Sair\n");
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

                inserirRegistro(registros[posicao]);
                break;
            case 2:
                system("clear");

                printf("Qual o registro deseja remover?\n");
                scanf("%d", &posicao);

                removerRegistro(registrosRemover[posicao].cod_vei, registrosRemover[posicao].cod_cli);
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
                system("clear");

                printf("\n---Finalizando o programa!!!---\n\n");
                break;
            default:
                system("clear");

                printf("\n---Valor Inválido---\n\n");
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
