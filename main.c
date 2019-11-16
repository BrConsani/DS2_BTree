#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define NULO -1
#define VAGO -2

#define INICIO 0
#define ATUAL 1
#define FINAL 2
#define TAM 256

#define MAXCHAVE 3
#define MINCHAVE MAXCHAVE/2

typedef struct reg{
	int cod;
	char nome[50];
	char seg[50];
	char tipo[30];
} Registro;

typedef struct no{
    int contagem;
    int chaves[MAXCHAVE];
    int ponteiros[MAXCHAVE+1]
} No;

Registro tempInsere[6];
int tempRemove[4];
int tempBusca[4];
int tempIndex[3] = {0, 0, 0};
int raiz;

int inserirCodigo(int rnn, int codigo, int* filhoPromovido, int* codigoPromovido);
int buscarNo(int codigo, No* no, int* posicao);
int inserirNaPagina(int codigo, int rrn, No *no);
void dividirNo(int codigo, int rnn, No* noAntigo, int* codigoPromovido, int* filhoPromovido, No* novoNo);

void listarTodos();
void buscarCodigo(int codigo);

void carregarArquivos();
void obterCache();
void salvarCache();
void dumpArquivo();

int main(void){

    DIR *temp = opendir("temp");

	if (temp)
		closedir(temp);
	else
		mkdir("temp", 0777);

    obterCache();
    //Verificar se arvore existe, se nao, criala
    
    printf("///////////////  SISTEMA DE REGISTRO DE SEGURADORAS  ///////////////\n");
	printf("///////////////                MENU                  ///////////////\n");
	printf("///////////////  1. INSERIR                          ///////////////\n");
	printf("///////////////  2. LISTAR TODOS OS SEGURADOS        ///////////////\n");
	printf("///////////////  3. BUSCAR CODIGO                    ///////////////\n");
	printf("///////////////  4. CARREGAR ARQUIVOS                ///////////////\n");
    printf("///////////////  5. DUMP DE ARQUIVOS                 ///////////////\n");
	printf("/////////////// -1. SAIR                             ///////////////\n");

    int escolha;

    do{
        printf("\n$ ");
        scanf("%d", &escolha);
        switch(escolha){
            case 1:
                //inserir
                //tempIndex[0]++;
                break;
            case 2:
                //listar todos os segurados
                break;
            case 3:
                //buscar
                //tempIndex[2]++;
                break;
            case 4:
                carregarArquivos();
                break;
            case 5:
                dumpArquivo();
                break;
            case -1:
                break;
            default:
                printf("Opcao invalida.\n");
        }
        if(tempIndex[0] >= 6){
            tempIndex[0] = 0;
            printf("Todos os registros foram adicionados, insercao reiniciada.\n");
        }
        if(tempIndex[1] >= 4){
            tempIndex[1] = 0;
            printf("Todos os codigos foram removidos, remocao reiniciada.\n");
        }
        if(tempIndex[2] >= 4){
            tempIndex[2] = 0;
            printf("Todos os codigos foram buscados, busca reiniciada.\n");
        }
        salvarCache();         
    }while(escolha != -1);
}

int inserirCodigo(int rnn, int codigo, int* filhoPromovido, int* codigoPromovido){
    No no;
    No novoNo;
    int encontrado, promovido;
    int posicao, auxRaiz, auxCodigo;

    if(raiz == NULO){
        *codigoPromovido = codigo;
        filhoPromovido = NULO;
        return(TRUE);
    }

    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, rnn * sizeof(No) * 2, INICIO);
    fread(&no, sizeof(No), 1, arvoreB);
    fclose(arvoreB);

    encontrado = buscarNo(codigo, &no, &posicao);    
    if(encontrado){
        printf("Codigo %d duplicado!/n", codigo);
        return FALSE;
    }

    promovido = inserirCodigo(no.ponteiros[posicao], codigo, &auxRaiz, &auxCodigo);
    if(promovido == FALSE){
        return FALSE;
    }
    if(no.contagem < MAXCHAVE){
        inserirNaPagina(auxCodigo, auxRaiz, &no);
        FILE* arvoreB;

        arvoreB = fopen("./temp/arvoreB.bin", "r+b");
        fseek(arvoreB, rnn * sizeof(No) * 2, INICIO);
        fwrite(&no, sizeof(No),1 ,arvoreB);
        fclose(arvoreB);
        return FALSE;
    }else{
        dividirNo(auxCodigo, auxRaiz, &no, codigoPromovido, filhoPromovido, &novoNo);
        FILE* arvoreB;

        arvoreB = fopen("./temp/arvoreB.bin", "r+b");
        fseek(arvoreB, rnn * sizeof(No) * 2, INICIO);
        fwrite(&no, sizeof(No),1 ,arvoreB);
        fseek(arvoreB, *filhoPromovido * sizeof(No) * 2, INICIO);
        fwrite(&novoNo, sizeof(No),1 ,arvoreB);
        fclose(arvoreB);
    }
}

int buscarNo(int codigo, No* no, int* posicao){
    int i;
    for(i=0; i<no->contagem && codigo > no->chaves[i]; i++);
        *posicao = i;
    if(*posicao < no->contagem && codigo == no->chaves[*posicao]){
        return TRUE;
    }else{
        return FALSE;
    }
}

int inserirNaPagina(int codigo, int rrn, No *no){
    int i;
    for(i=no->contagem; codigo < no->chaves[i-1] && i > 0; i--){
        no->chaves[i] = no->chaves[i-1];
        no->ponteiros[i+1] = no->ponteiros[i];
    }
    no->contagem++;
    no->chaves[i] = codigo;
    no->ponteiros[i+1] = rrn;
}

void dividirNo(int codigo, int rnn, No* noAntigo, int* codigoPromovido, int* filhoPromovido, No* novoNo){
    int i;
    int meio;
    int tempCodigos[MAXCHAVE+1];
    int tempPonteiro[MAXCHAVE+2];

    for(i=0; i<MAXCHAVE; i++){
        tempCodigos[i] = noAntigo->chaves[i];
        tempPonteiro[i] = noAntigo->ponteiros[i];
    }
    tempPonteiro[i] = noAntigo->ponteiros;
    for(i=MAXCHAVE; codigo < tempCodigos[i-1] && i > 0; i--){
        tempCodigos[i] = tempCodigos[i-1];
        tempPonteiro[i+1] = tempPonteiro[i];
    }
    tempCodigos[i] = codigo;
    tempPonteiro[i+1] = rnn;
    
    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, )



    filhoPromovido = 

}


void listarTodos(){

}

void buscarCodigo(int codigo){

}

void carregarArquivos(){
    FILE *insere;

	insere = fopen("./temp-testes/insere.bin", "rb");
	fread(&tempInsere, sizeof(struct reg), 6, insere);
	fclose(insere);

    FILE *remove;

	remove = fopen("./temp-testes/remove.bin", "rb");
	fread(&tempRemove, sizeof(int), 4, remove);
	fclose(remove);

    FILE *busca;

	busca = fopen("./temp-testes/busca.bin", "rb");
	fread(&tempBusca, sizeof(int), 4, busca);
	fclose(busca);

	printf("Dados carregados com sucesso!\n");
}

void obterCache(){
    FILE *cache;

    cache = fopen("./temp/cache.bin", "r+b");

    if(cache == NULL){
        printf("Cache criado!\n");
        salvarCache();
    }else{
        fread(&tempIndex, sizeof(int), 3, cache);
        fread(&raiz, sizeof(int), 1, cache);
        fclose(cache);
        printf("Cache obtido [%d, %d, %d, %d]\n", tempIndex[0], tempIndex[1], tempIndex[2], raiz);
    }
}

void salvarCache(){
    FILE *cache;

    cache = fopen("./temp/cache.bin", "w+b");
    fwrite(&tempIndex, sizeof(int), 3, cache);
    fwrite(&raiz, sizeof(int), 1, cache);
    fclose(cache);
}

void hexDump(size_t offset, void *addr, int len){

	int i;
	unsigned char bufferLine[17];
	unsigned char *pc = (unsigned char *)addr;

	for (i = 0; i < len; i++){
		if ((i % 16) == 0){
			if (i != 0)
				printf(" %s\n", bufferLine);
			printf("%08zx: ", offset);
			offset += (i % 16 == 0) ? 16 : i % 16;
		}
		printf("%02x ", pc[i]);
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)){
			bufferLine[i % 16] = '.';
		}else{
			bufferLine[i % 16] = pc[i];
		}
		bufferLine[(i % 16) + 1] = '\0';
	}

	while ((i % 16) != 0){
		printf("  ");
		if (i % 2 == 1)
			putchar(' ');
		i++;
	}
	printf("  %s\n", bufferLine);
}

void dumpArquivo(){
    FILE *myfile;
    int escolha;

    printf("Escolha o arquivo para fazer o dump:\n\n");

    if (fopen("./temp/data.bin", "rb") != NULL)
        printf(" 1. data.bin\n");
    if (fopen("./temp/treeb.bin", "rb") != NULL)
        printf(" 2. treeb.bin\n");

    printf("-1. RETORNAR\n\n");

    printf("$ ");

    scanf("%d", &escolha);

    if(escolha == -1)
        return;

    switch (escolha){
        case 1:
            myfile = fopen("./temp/data.bin", "rb");
            break;
        case 2:
            myfile = fopen("./temp/treeb.bin", "rb");
            break;
        default:
            printf("Escolha invalida, abortando dump.\n");
            return;
    }

    printf("\n");

    unsigned char buffer[TAM];
    size_t n;
    size_t offset = 0;

    while ((n = fread(buffer, 1, TAM, myfile)) > 0){
        hexDump(offset, buffer, n);
        if (n < TAM)
            break;
        offset += n;
    }
    fclose(myfile);
}