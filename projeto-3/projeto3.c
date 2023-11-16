#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAXKEYS 3
#define MINKEYS (MAXKEYS/2)
#define NIL (-1)
#define NOKEY '@'
#define NO 0
#define YES 1

struct BTPage{
    short keycount; //numero de chaves na página
    char key[MAXKEYS][19]; //contagem atual de chaves
    short child[MAXKEYS+1]; // RRNs dos filhos
    short rrnFile[MAXKEYS]; //RRN do arquivo
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
    int NumeroDias;
};

struct RegistroLocacao registrosInsercao[100];  // para armazenar o arquivo insere.bin
struct RegistroBusca registrosBusca[100];   // para armazenar o arquivo busca.bin

FILE *fileRegistros, *fileBTree;

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

bool existeArquivoBtree() {
    if(access("BTree.bin", F_OK) == 0){
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

// TODO: PRECISA TESTAR SE ESTA FUNCIONAL

bool btOpen() {
    if ((fileBTree = fopen("BTree.bin", "r+b")) == NULL) {
        return false;
    }
    return true;
}

void btread (short rrn, struct BTPage *page_ptr) {
    long addr;
    addr = (long)rrn * (long)PAGESIZE + 2L;
    fseek(fileBTree,addr,0);
    fread(&(*page_ptr),PAGESIZE,1,fileBTree);
    return;
}

short getRoot() {
    short root;
    rewind(fileBTree);
    if (!(fread(&root, sizeof(short), 1, fileBTree))) {
        printf("Erro ao ler raiz");
        exit(1);
    }
    return root;
}

void putRoot(short root) {

    fseek(fileBTree,0,SEEK_SET);
	fwrite(&root, sizeof(short), 1, fileBTree);

}

short getpageRRN() {
    short addr;
    fseek(fileBTree, 0, SEEK_END);
    addr = ftell(fileBTree) - sizeof(short);
    return ((short) addr / PAGESIZE);
}

void btWrite(short rrn, struct BTPage *page_ptr) { //Insere a pagina no arquivo
	long addr;
	addr = (long)rrn * (long)PAGESIZE + 2L;
	fseek(fileBTree, addr, 0);
	fwrite(&(*page_ptr), PAGESIZE, 1, fileBTree);
}

void pageInit(struct BTPage *p_page) {
    //Inicializa a página totalmente vazia

    p_page->keycount = NO;

    for (int i = 0; i < MAXKEYS; i++){
        p_page->key[i][0] = NOKEY;
        p_page->child[i] = NIL;
        p_page->rrnFile[i] = NOKEY;
    }
    p_page->child[MAXKEYS] = NIL;

}

short createRoot(char *chave, short left, short right, short rrnArquivo) { //Cria raiz
	struct BTPage page;

	short rrn;

	rrn = getpageRRN();
	pageInit(&page);
	strcpy( page.key[0], chave);    // TODO verificar essa linha
	
	page.child[0] = left;
	page.child[1] = right;
	page.keycount = (short)1;
	page.rrnFile[0] = rrnArquivo;
	
	btWrite(rrn, &page);
	putRoot(rrn);
	return(rrn);
}

int createTree(char *chave) {
    int menosum = NIL;

    if ((fileBTree = fopen("BTree.bin", "w+b")) == NULL) {
        printf("Erro ao abrir arquivo");
        return 0;
    }

    fseek(fileBTree, 0, SEEK_SET);
    fwrite(&menosum, sizeof(int), 1, fileBTree); // header = -1
    fclose(fileBTree);

    fileBTree = fopen("BTree.bin", "w+b");

    return (createRoot(chave, NIL, NIL, 0));
}

int buscaDuplicada(char *chave, short *pos, short rrn) { //Buscando chaves duplicadas no nó
	int i;
    struct BTPage tempPage;
    btread(rrn, &tempPage);
    if(tempPage.keycount > MAXKEYS){
        return (NO);
    }
        for (i = 0; i < tempPage.keycount; i++) {
            if (strncmp(chave, tempPage.key[i], 18) == 0) {
                *pos = i;
                return (YES);
            }
            if (strncmp(chave, tempPage.key[i], 18) < 0) { //Se for menor que a chave, está no filho a esquerda
                if (tempPage.child[i] < 0) {
                    *pos = i;
                    return (NO);
                } else {
                    *pos = i;
                    return (buscaDuplicada(chave, pos, tempPage.child[i]));
                }
            } else if (strncmp(chave, tempPage.key[i], 18) > 0) { //Se for maior que a chave, ou será igual a próxima ou está no filho a direita

                if (tempPage.key[i+1][0] != '@') { //Verifica se existe chave a direita

                    if (strncmp(chave, tempPage.key[i+1], 18) < 0) {//Se for menor que proxima chave ele busca nos filhos
                        if (tempPage.child[i] < 0) {
                            *pos = i+1;
                            return (NO);
                        } else {
                            *pos = i;
                            return (buscaDuplicada(chave, pos, tempPage.child[i+1]));
                        }
                    }
                }
                else {
                    if (tempPage.child[i] < 0) {
                        *pos = i;
                        return (NO);
                    } else {
                        *pos = i;
                        return (buscaDuplicada(chave, pos, tempPage.child[i+1]));
                    }
                }
            }
        }
}

void inserirPagina(char *chave, short rnn_child, struct BTPage *p_page, short rrnArquivo) {
	int i;

    //Ordena a pagina até encontrar a posição correta para a chave
	for(i = p_page->keycount; (strcmp(chave, &p_page->key[i-1]) < 0)  && i > 0; i--) {
		strcpy(&p_page->key[i],&p_page->key[i-1]);
		p_page->rrnFile[i] = p_page->rrnFile[i-1];
		p_page->child[i+1] = p_page->child[i];
	}
	
    //Inserindo a chave
	p_page->keycount++;
	strcpy(&p_page->key[i], chave);
	p_page->rrnFile[i] = rrnArquivo;
	p_page->child[i+1] = rnn_child;
}

void split(char *key, short r_child, struct BTPage *p_oldpage, char *promo_key, short *promo_r_child, struct BTPage *p_newpage, short rrnArquivo,short *promo_rrn) {
	int i;

    //Declarando variáveis para manipulação dos dados
	char workKeys[MAXKEYS+1][19];
	short workChild[MAXKEYS+2], workRrn[MAXKEYS+1];
	
	for (i = 0; i < MAXKEYS; i++) {
		strcpy(workKeys[i],&p_oldpage->key[i]);
		workChild[i] = p_oldpage->child[i];
		workRrn[i] = p_oldpage->rrnFile[i];
	}
	
	workChild[i] = p_oldpage->child[i];
	
	for (i = MAXKEYS; (strcmp(key,workKeys[i-1]) < 0)  && i > 0; i--) {
		strcpy(workKeys[i],workKeys[i-1]);
		workRrn[i] = workRrn[i-1];
		workChild[i+1] = workChild[i];
	}
	
	strcpy(workKeys[i],key);
	workRrn[i] = rrnArquivo;
	
	workChild[i+1] = r_child;
	*promo_r_child = getpageRRN();
	pageInit(p_newpage);
	
	for (i = 0; i < MINKEYS + 1; i++) {
		strcpy (&p_oldpage->key[i],workKeys[i]);
		p_oldpage->child[i] = workChild[i];
		p_oldpage->rrnFile[i] = workRrn[i];

		if (i != MINKEYS){
			strcpy(&p_newpage->key[i],workKeys[i+3]);
			p_newpage->child[i] = workChild[i+3];
			p_newpage->rrnFile[i] = workRrn[i+3];
			
			strcpy(&p_oldpage->key[i+2], "@");
			p_oldpage->child[i+3] = NIL;
			p_oldpage->rrnFile[i+2] = NIL;
		}
	}
	
	p_oldpage->child[MINKEYS] = workChild[MINKEYS];
	p_newpage->child[MINKEYS] = workChild[i+1+MINKEYS];
	
	p_newpage->keycount = MAXKEYS - MINKEYS - 1;
	p_oldpage->keycount = MINKEYS + 1;
	
	strcpy(promo_key,workKeys[MINKEYS+1]);
	*promo_rrn = workRrn[MINKEYS+1];
	
    printf("Divisao de nó\n");
	printf ("Chave promovida: %s", promo_key);
}

int inserirArvore (short rrn, char* chave, short *promo_r_child, char *promo_key, short rrnArquivo,short *promo_rrn) {
	struct BTPage page, newpage; 
	       
	int found, promoted;
	
	short pos,
	      p_b_rrn;//rrn promovido no arquivo de dados
	      
	char p_b_key[19]; //chave promovida
	
	if (rrn == NIL) {
		strcpy(promo_key, chave);
		*promo_r_child = NIL;
        *promo_rrn = rrnArquivo;
		return(YES);
	}

    //Busca se existe duplicata antes de inserir
	btread(rrn, &page);
	found = buscaDuplicada (chave, &pos, 0);
	
	if (found) {
		printf ("\nChave duplicada: %s\n", chave);
		return(0);
	}

    //Recursão para chaves promovidas
	promoted = inserirArvore(page.child[pos], chave, &p_b_rrn, p_b_key, rrnArquivo, &(*promo_rrn));
	if (!promoted) {
		return(NO);
	}
	
    //Verificação para inserção normal ou via Split
	if(page.keycount < MAXKEYS) {
		inserirPagina(p_b_key, p_b_rrn, &page, rrnArquivo);
        btWrite(rrn, &page);
		return(NO);

	} else {
		printf("Divisao de no\n");
		split(p_b_key, p_b_rrn, &page, promo_key, promo_r_child, &newpage, rrnArquivo, &(*promo_rrn));
        btWrite(rrn, &page);
        btWrite(*promo_r_child, &newpage);
		return(YES);
	}
}

void inserirRegistro(FILE *registros) {

    int posicao, promoted;
	
	short root, // rrn da raiz
	      promo_rrn,// rrn ppromovido
		  promo_rrn_arquivo,
		  rrnArquivo; //rrn no arquivo de dados 
	
	char promo_key[19], chave[19]; // chave promovida e chave a ser inserida

    printf("Qual registro deseja inserir?\n");
    scanf("%d", &posicao);
    posicao --; // para converter, comecando em zero

    // Inserindo o registro no final do arquivo principal:
    fseek(registros, 0, SEEK_END);
    rrnArquivo = ftell(registros);
    strcpy(chave ,registrosInsercao[posicao].CodCli);
    strcat(chave, registrosInsercao[posicao].CodVei);

    if(btOpen()) {
        short pos = 0;
        root = getRoot();
        //Verificar se chave já existe na árvore
        if (buscaDuplicada(chave, &pos, root)){
            printf("\nChave duplicada: %s\n", chave);
            return;
        }

        promoted = inserirArvore(root, chave, &promo_rrn, promo_key, rrnArquivo, &promo_rrn_arquivo);
		if (promoted)
			root = createRoot(promo_key, root, promo_rrn, promo_rrn_arquivo);

    }else{
        //Cria nova árvore e insere
        root = createTree(chave);
    }
    int sizeCodCliCodVei = 18;
    char tamanhoDoRegistro = sizeCodCliCodVei + strlen(registrosInsercao[posicao].NomeCliente) + strlen(registrosInsercao[posicao].NomeVeiculo) + sizeof(int) + 5;

    fwrite(&tamanhoDoRegistro, 1, sizeof(char), registros);
    fwrite(registrosInsercao[posicao].CodCli, 1, strlen(registrosInsercao[posicao].CodCli), registros);
    fwrite("|", 1, sizeof(char), registros);
    fwrite(registrosInsercao[posicao].CodVei, 1, strlen(registrosInsercao[posicao].CodVei), registros);
    fwrite("|", 1, sizeof(char), registros);
    fwrite(registrosInsercao[posicao].NomeCliente, 1, strlen(registrosInsercao[posicao].NomeCliente), registros);
    fwrite("|", 1, sizeof(char), registros);
    fwrite(registrosInsercao[posicao].NomeVeiculo, 1, strlen(registrosInsercao[posicao].NomeVeiculo), registros);
    fwrite("|", 1, sizeof(char), registros);
    fwrite(&registrosInsercao[posicao].NumeroDias, 1, sizeof(int), registros);
    fwrite("|", 1, sizeof(char), registros);
    printf ("\n\nChave inserida com sucesso: %s\n\n", chave);
}

void pesquisaChave(char *chave, short pagina){ //Função de buscar chave específica

    struct BTPage tempPage;
    int i;

        btread(pagina, &tempPage);

        for (i = 0; i < tempPage.keycount; i++) {
            if(strncmp(chave, tempPage.key[i], 18) == 0){
                printf("Chave encontrada: %s | pag: %d | pos: %d", tempPage.key[i], pagina, i);
                return;
            }
            if(strncmp(chave, tempPage.key[i], 18) < 0){ //Se for menor que a chave, está no filho a esquerda
                if(tempPage.child[i] < 0){
                    printf("Chave não encontrada: %s", chave);
                    return;
                }
                else{
                    pagina = tempPage.child[i];
                    return pesquisaChave(chave, pagina);
                }
            }
            else if (strncmp(chave, tempPage.key[i], 18) > 0) { //Se for maior que a chave, ou será igual a próxima ou está no filho a direita

                if (tempPage.key[i+1][0] != '@') { //Verifica se existe chave a direita

                    if (strncmp(chave, tempPage.key[i+1], 18) < 0) {//Se for menor que proxima chave ele busca nos filhos
                        if (tempPage.child[i] < 0) {
                            printf("Chave não encontrada: %s", chave);
                            return;
                        } else {
                            pagina = tempPage.child[i+1];
                            return pesquisaChave(chave, pagina);
                        }
                    }
                }
                else {
                    if (tempPage.child[i] < 0) {
                        printf("Chave não encontrada: %s", chave);
                        return;
                    } else {
                        pagina = tempPage.child[i+1];
                        return pesquisaChave(chave, pagina);
                    }
                }
            }
        }
}

void printarDados(short rrn){

    fseek(fileRegistros, rrn, SEEK_SET);

    char caracter = 'a', tamanho;
    struct RegistroLocacao aux;

//    for(int i=1; i<rrn; i++) {
//        fread(&tamanho, sizeof(char), 1, registros);
//        fseek(registros, tamanho, SEEK_CUR);
//    }

    //Leitura dos registros Variaveis;
    fseek(fileRegistros, 1, SEEK_CUR);
    fread(&aux.CodCli, sizeof(char), 11, fileRegistros);
    aux.CodCli[11] = '\0';
    fseek(fileRegistros, 1, SEEK_CUR);
    fread(&aux.CodVei, sizeof(char), 7, fileRegistros);
    aux.CodVei[7] = '\0';

    fseek(fileRegistros, 1, SEEK_CUR);
    strcpy(aux.NomeCliente, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, fileRegistros);
        if(caracter != '|'){
            strncat(aux.NomeCliente, &caracter, 1);
        }
    }

    caracter = 'a'; //Reiniciando caracter
    strcpy(aux.NomeVeiculo, "");
    while(caracter != '|'){
        fread(&caracter, sizeof(char), 1, fileRegistros);
        if(caracter != '|'){
            strncat(aux.NomeVeiculo, &caracter, 1);
        }
    }

    fscanf(fileRegistros, "%d", &aux.NumeroDias);

    //printa o registro
    printf("\n----------------------------------------------------------------------\n");
    printf("%s | %s | %s | %s | %d", aux.CodCli, aux.CodVei, aux.NomeCliente, aux.NomeVeiculo, aux.NumeroDias);
    printf("\n----------------------------------------------------------------------\n");

}

void listarDados(short rrn){

    struct BTPage tempPage;

    btread(rrn, &tempPage);

    for(int i = 0; i < tempPage.keycount ; i++){

        if(tempPage.child[i] >= 0){
            listarDados(tempPage.child[i]);
        }
        printarDados(tempPage.rrnFile[i]);

        if(i == (tempPage.keycount-1)) {
            if(tempPage.child[i+1] >= 0){
                listarDados(tempPage.child[i+1]);
            }
        }
    }
}

int main () {
    int resposta, posicao;
    short menosUm = -1, rrn;
    char chave[19];

    // Se o arquivo principal não existe, criar o arquivo para escrita:
    if(!existeArquivoRegistro()) {
        if ((fileRegistros = fopen("registros.bin","w+b")) == NULL) {
            printf("Erro ao criar arquivo");
            return 0;
        }
        fclose(fileRegistros);
    }
    // Caso exista, abrir para leitura:
    else {
        if((fileRegistros = fopen("registros.bin","r+b")) == NULL) {
            printf("Erro ao abrir arquivo");
            return 0;
        }
        fclose(fileRegistros);
    }

    if(!existeArquivoBtree()) {
        if ((fileBTree = fopen("BTree.bin","w+b")) == NULL) {
            printf("Erro ao criar arquivo");
            return 0;
        }
        fwrite(&menosUm, 1, sizeof (short),fileBTree );
        fclose(fileBTree);
    }
        // Caso exista, abrir para leitura:
    else {
        if((fileBTree = fopen("BTree.bin","r+b")) == NULL) {
            printf("Erro ao abrir arquivo");
            return 0;
        }
        fclose(fileBTree);
    }

    do {
        resposta = imprimirMenu();

        switch (resposta) {
            case 1:
                fileRegistros = fopen("registros.bin","r+b");
                fileBTree = fopen("BTree.bin","r+b");
                btOpen();
                inserirRegistro(fileRegistros);
                fclose(fileRegistros);
                fclose(fileBTree);
                break;
            case 2:
                fileRegistros = fopen("registros.bin","r+b");
                fileBTree = fopen("BTree.bin","r+b");
                btOpen();
                rrn = getRoot();
                listarDados(rrn);
                fclose(fileRegistros);
                fclose(fileBTree);
                break;
            case 3:
                fileRegistros = fopen("registros.bin","r+b");
                fileBTree = fopen("BTree.bin","r+b");
                btOpen();
                printf("Qual o registro?");
                scanf("%d", &posicao);
                strcpy(chave ,registrosBusca[posicao-1].cod_cli);
                strcat(chave, registrosBusca[posicao-1].cod_vei);
                rrn = getRoot();
                pesquisaChave(chave, rrn);
                fclose(fileRegistros);
                fclose(fileBTree);
                break;
            case 4:
                carregaArquivos();
                break;
            default:
                break;
        }
    } while (resposta != 5);
}
