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
    short rnnFile[MAXKEYS];
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

void putRoot(FILE *fileBTree, short root) {

    fseek(fileBTree,0,SEEK_SET);
	fwrite(&root, sizeof(short), 1, fileBTree);

}

short getpageRRN(FILE *fileBTree) {
    short addr;
    fseek(fileBTree, 0, SEEK_END);
    addr = ftell(fileBTree) - sizeof(short);
    return ((short) addr / PAGESIZE);
}

void btWrite(short rrn, struct BTPage *page_ptr, FILE *fileBTree) { //Insere a pagina no arquivo  
	long addr;
	addr = (long)rrn * (long)PAGESIZE + 2L;
	fseek(fileBTree, addr, 0);	
	fwrite(&(*page_ptr), PAGESIZE, 1, fileBTree);

	return;
}

void pageInit(struct BTPage *p_page) {
    //Inicializa a página totalmente vazia

    p_page->keycount = NO;

    for (int i = 0; i < MAXKEYS; i++){
        p_page->key[i] = NOKEY;
        p_page->child[i] = NIL;
        p_page->rnnFile[i] = NOKEY;
    }
    p_page->child[MAXKEYS] = NIL;

}

int createTree(FILE *fileBTree, char *chave) {
    int menosum = NIL;

    if ((fileBTree = fopen("BTree.bin", "w+b")) == NULL) {
        printf("Erro ao abrir arquivo");
        return 0;
    }

    fseek(fileBTree, 0, SEEK_SET);
    fwrite(&menosum, sizeof(int), 1, fileBTree); // header = -1
    fclose(fileBTree);

    fileBTree = fopen("BTree.bin", "w+b");

    return (createRoot(chave, NIL, NIL, 0, fileBTree));
}

short create_root(char *chave, short left, short right, short rrnArquivo, FILE *fileBTree) {
	struct BTPage page;

	short rrn;

	rrn = getpageRRN(fileBTree);
	pageinit(&page);
	strcpy(page.key[0], chave);
	
	page.child[0] = left;
	page.child[1] = right;
	page.keycount = (short)1;
	page.rnnFile[0] = rrnArquivo;
	
	btwrite(rrn, &page);
	putroot(rrn);

    printf("Chave inserida com sucesso: %s", chave);
	
	return(rrn);
}

int buscaDuplicada(char *chave, FILE *fileBTree) { //Buscando chaves duplicadas no nó
	int i;
    struct BTPage tempPage;

	fread(&tempPage, sizeof(PAGESIZE), 1, fileBTree);

        for(i = 0; i < tempPage.keycount; i++){
            if(chave == tempPage.key[i]){
                return (YES);
            }
            if(chave < tempPage.key[i]){ //Se for menor que a chave, está no filho a esquerda
                if(tempPage.child[i] < 0){
                    return(NO);
                }
                else{
                    fseek(fileBTree, tempPage.child[i] * sizeof(PAGESIZE), SEEK_SET);
                    return(buscaDuplicada(chave, fileBTree));
                }
            }
            else if(chave > tempPage.key[i]){ //Se for maior que a chave, ou será igual a próxima ou está no filho a direita

                if(i < (tempPage.keycount - 1)){ //Verifica se existe chave a direita

                    if(chave < tempPage.key[i+1]){//Se for menor que proxima chave ele busca nos filhos

                        if(tempPage.child[i] < 0){
                            return(NO);
                        }
                        else{
                            fseek(fileBTree, tempPage.child[i+1] * sizeof(PAGESIZE), SEEK_SET);
                            return(buscaDuplicada(chave, fileBTree));
                        }

                    }
                }else {
                    return(NO);
                }
            }
        }
	

}

int inserirArvore(FILE *fileBTree, char *chave, short rnnArquivo){



}

void inserirRegistro(FILE *registros, FILE *fileBTree) {

    // TODO: terminar funcao inserir
    int posicao, root;
    printf("Qual registro deseja inserir?\n");
    scanf("%d", &posicao);
    posicao --; // para converter, comecando em zero

    // Inserindo o registro no final do arquivo principal:
    short rnn;
    fseek(registros, 0, SEEK_END);
    rnn = ftell(registros);
    char chave[18];
    strcpy(chave ,registrosInsercao->CodCli[posicao]);
    strcat(chave, registrosInsercao->CodVei[posicao]);

    if(btOpen(fileBTree)){

        root = getRoot(fileBTree);
        //Verificar se chave já existe na árvore
        if (buscaDuplicada(chave, fileBTree)){
            printf("Chave duplicada: %s", chave);
            return;
        }

        inserirArvore(fileBTree, chave, rnn);

    }else{
        //Cria nova árvore e insere
        root = createTree(fileBTree, chave);
    }

    fwrite(&registrosInsercao[posicao], sizeof(struct RegistroLocacao), 1, registros);
    printf("Chave inserida com sucesso: %s", chave);
}


void pesquisaChave(FILE *registros, FILE *fileBTree, char chave[]){ //Função de buscar chave específica

    bool naoEncontrado = true;
    struct BTPage tempPage;
    int i;
    short pagina = 0;

    while(naoEncontrado){

        fread(&tempPage, sizeof(PAGESIZE), 1, fileBTree);

        for(i = 0; i < tempPage.keycount; i++){
            if(chave == tempPage.key[i]){
                printf("Chave encontrada: %s | pag: %d | pos: %d", tempPage.key[i], pagina, i);
                naoEncontrado = false;
                break;
            }
            if(chave < tempPage.key[i]){ //Se for menor que a chave, está no filho a esquerda
                if(tempPage.child[i] < 0){
                    printf("Chave não encontrada: %s", chave);
                    break;
                }
                else{
                    fseek(fileBTree, tempPage.child[i] * sizeof(PAGESIZE), SEEK_SET);
                    pagina = tempPage.child[i];
                }
            }
            else if(chave > tempPage.key[i]){ //Se for maior que a chave, ou será igual a próxima ou está no filho a direita

                if(i < (tempPage.keycount - 1)){ //Verifica se existe chave a direita

                    if(chave < tempPage.key[i+1]){//Se for menor que proxima chave ele busca nos filhos

                        if(tempPage.child[i] < 0){
                            printf("Chave não encontrada: %s", chave);
                            break;
                        }
                        else{
                            fseek(fileBTree, tempPage.child[i+1] * sizeof(PAGESIZE), SEEK_SET);
                            pagina = tempPage.child[i+1];
                        }

                    }
                }else {
                    printf("Chave não encontrada: %s", chave);
                    break;
                }
            }
        }

    }

}

void listarDados(FILE *registros, FILE *fileBTree){

    struct BTPage tempPage;
    fread(&tempPage, sizeof(PAGESIZE), 1, fileBTree);
    for(int i = 0; i < tempPage.keycount + 1; i++){

        if(tempPage.child >= 0){
            fseek(fileBTree, tempPage.child[i] * sizeof(PAGESIZE), SEEK_SET);
            listarDados(registros, fileBTree);
        }
        if (i < tempPage.keycount){
            printarDados(registros, tempPage.rnnFile[i]);
        }
    }

}

void printarDados(FILE *registros, short rnn){

    rewind(registros);
    char caracter = 'a', tamanho;
    struct RegistroLocacao aux;

    for(int i = 1; i < rnn; i++) {
        fread(&tamanho, sizeof(char), 1, registros);
        fseek(registros, tamanho, SEEK_CUR);
    }
    //Leitura dos registros Variaveis;
    fseek(registros, 1, SEEK_CUR);
    fread(&aux.CodCli, sizeof(char), 11, registros);
    aux.CodCli[11] = '\0';
    fseek(registros, 1, SEEK_CUR);
    fread(&aux.CodVei, sizeof(char), 7, registros);
    aux.CodVei[7] = '\0';

    fseek(registros, 1, SEEK_CUR);
    strcpy(aux.NomeCliente, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, registros);
        if(caracter != '|'){
            strncat(aux.NomeCliente, &caracter, 1);
        }
    }

    caracter = 'a'; //Reiniciando caracter
    strcpy(aux.NomeVeiculo, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, registros);
        if(caracter != '|'){
            strncat(aux.NomeVeiculo, &caracter, 1);
        }
    }

    fscanf(registros, "%d", &aux.NumeroDias);

    //printa o registro
    printf("\n----------------------------------------------------------------------\n");
    printf("%s | %s | %s | %s | %d", aux.CodCli, aux.CodVei, aux.NomeCliente, aux.NomeVeiculo, aux.NumeroDias);
    printf("\n----------------------------------------------------------------------\n");

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
