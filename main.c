#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0
#define NULO -1
#define VAGO 0xfefefefe

#define INICIO 0
#define ATUAL 1
#define FINAL 2
#define TAM 256

#define MAXCHAVE 3
#define MINCHAVE 2

#define QUANTIDADEINSERIR 16
#define QUANTIDADEBUSCA 5
#define QUANTIDADEREMOVER 4

typedef struct reg{
	int cod;
	char nome[50];
	char seg[50];
	char tipo[30];
} Registro;

typedef struct no{
    int contagem;
    int chaves[MAXCHAVE];
    int offset[MAXCHAVE];
    int ponteiros[MAXCHAVE+1];
} No;

Registro tempInsere[QUANTIDADEINSERIR];
int tempRemove[QUANTIDADEREMOVER];
int tempBusca[QUANTIDADEBUSCA];
int tempIndex[3] = {0, 0, 0};
int raiz;

int inserirNoPrincipal(Registro registro);
int inserirCodigo(int rnn, int codigo, int* filhoPromovido, int* codigoPromovido, int* offsetPromovido);
int criarRaiz(int codigo, int offset, int esq, int dir);
int buscarNo(int codigo, No* no, int* posicao);
int inserirNaPagina(int codigo, int offset, int rrn, No *no);
void dividirNo(int codigo, int offset, int rnn, No* noAntigo, int* codigoPromovido, int* offsetPromovido, int* filhoPromovido, No* novoNo);

void listarTodos(int rnn);
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
    
    FILE *data;

    data = fopen("./temp/data.bin", "rb");

    if(data == NULL)
        data = fopen("./temp/data.bin", "w+b");

    fclose(data);

    FILE *arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    if(arvoreB == NULL){
        arvoreB = fopen("./temp/arvoreB.bin", "w+b");
        int aux = NULO;
        fwrite(&aux, sizeof(int), 1, arvoreB);
        raiz = NULO;
    }else{
        fread(&raiz, sizeof(int), 1, arvoreB);
    }
    fclose(arvoreB);
    
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
            int promovido, promovidoPont, promovidoCod, promovidoOffset;
            case 1:
                promovido = inserirCodigo(raiz, tempInsere[tempIndex[0]].cod , &promovidoPont, &promovidoCod, &promovidoOffset);
                if(promovido){
                    if(raiz == NULO)
                        promovidoOffset = inserirNoPrincipal(tempInsere[tempIndex[0]]);
                    raiz = criarRaiz(promovidoCod, promovidoOffset, raiz, promovidoPont);
                    FILE *arvoreB;
                    arvoreB = fopen("./temp/arvoreB.bin", "r+b");
                    fwrite(&raiz, sizeof(int), 1, arvoreB);
                    fclose(arvoreB);
                }
                tempIndex[0]++;
                break;
            case 2:
                listarTodos(raiz);
                break;
            case 3:
                buscarCodigo(tempBusca[tempIndex[2]]);
                tempIndex[2]++;
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
        if(tempIndex[0] >= QUANTIDADEINSERIR){
            tempIndex[0] = 0;
            printf("Todos os registros foram adicionados, insercao reiniciada.\n");
        }
        if(tempIndex[1] >= QUANTIDADEREMOVER){
            tempIndex[1] = 0;
            printf("Todos os codigos foram removidos, remocao reiniciada.\n");
        }
        if(tempIndex[2] >= QUANTIDADEBUSCA){
            tempIndex[2] = 0;
            printf("Todos os codigos foram buscados, busca reiniciada.\n");
        }
        salvarCache();         
    }while(escolha != -1);
}

int inserirNoPrincipal(Registro registro){
    int offset;

    FILE* data;

    data = fopen("./temp/data.bin", "r+b");
    fseek(data, 0, FINAL);
    offset = ftell(data);
    fwrite(&registro, sizeof(Registro), 1, data);
    fclose(data);
    return offset;
}

int criarRaiz(int codigo, int offset, int esq, int dir){
    No no;
    int rnn;
    FILE* arvoreB;
    
    arvoreB = fopen("./temp/arvoreB.bin", "r+b");
    fseek(arvoreB, -sizeof(int), FINAL);
    rnn = ftell(arvoreB) / sizeof(No);

    int j;
    for(j=0; j < MAXCHAVE; j++){
        no.chaves[j] = VAGO;
        no.offset[j] = VAGO;
        no.ponteiros[j] = NULO;
    }
    no.ponteiros[MAXCHAVE] = NULO;

    no.chaves[0] = codigo;
    no.offset[0] = offset;
    no.ponteiros[0] = esq;
    no.ponteiros[1] = dir;
    no.contagem = 1;

    arvoreB = fopen("./temp/arvoreB.bin", "r+b");
    fwrite(&rnn, sizeof(int), 1, arvoreB);
    fseek(arvoreB, rnn * sizeof(No) + 4, INICIO);
    fwrite(&no, sizeof(No), 1, arvoreB);
    fclose(arvoreB);
    if(rnn == 0)
        printf("Codigo %d inserido com sucesso!\n", codigo);
    return rnn;
}

int inserirCodigo(int rnn, int codigo, int* filhoPromovido, int* codigoPromovido, int* offsetPromovido){
    No no;
    No novoNo;
    int encontrado, promovido;
    int posicao, auxRaiz, auxCodigo, auxOffset = 0;

    if(rnn == NULO){
        *codigoPromovido = codigo;
        *filhoPromovido = NULO;
        return(TRUE);
    }

    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, rnn * sizeof(No) + 4, INICIO);
    fread(&no, sizeof(No), 1, arvoreB);
    fclose(arvoreB);

    encontrado = buscarNo(codigo, &no, &posicao);    
    if(encontrado){
        printf("Codigo %d duplicado!\n", codigo);
        return FALSE;
    }
    
    promovido = inserirCodigo(no.ponteiros[posicao], codigo, &auxRaiz, &auxCodigo, &auxOffset);
    if(promovido == FALSE){
        return FALSE;
    }
    if(no.contagem < MAXCHAVE){
        int offset;
        if(auxOffset != 0)
            offset = auxOffset;
        else
            offset = inserirNoPrincipal(tempInsere[tempIndex[0]]);
        inserirNaPagina(auxCodigo, offset, auxRaiz, &no);
        FILE* arvoreB;

        arvoreB = fopen("./temp/arvoreB.bin", "r+b");
        fseek(arvoreB, rnn * sizeof(No) + 4, INICIO);
        fwrite(&no, sizeof(No),1 ,arvoreB);
        fclose(arvoreB);
        printf("Codigo %d inserido com sucesso!\n", codigo);
        return FALSE;
    }else{
        int offset;
        if(auxOffset != 0)
            offset = auxOffset;
        else
            offset = inserirNoPrincipal(tempInsere[tempIndex[0]]);
        dividirNo(auxCodigo, offset, auxRaiz, &no, codigoPromovido, offsetPromovido, filhoPromovido, &novoNo);
        FILE* arvoreB;

        arvoreB = fopen("./temp/arvoreB.bin", "r+b");
        fseek(arvoreB, rnn * sizeof(No) + 4, INICIO);
        fwrite(&no, sizeof(No), 1 , arvoreB);
        fseek(arvoreB, *filhoPromovido * sizeof(No) + 4, INICIO);
        fwrite(&novoNo, sizeof(No),1 ,arvoreB);
        fclose(arvoreB);
        if(auxOffset != 0 || raiz == 0)
            printf("Codigo %d inserido com sucesso!\n", codigo);
        return TRUE;
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

int inserirNaPagina(int codigo, int offset, int rrn, No *no){
    int i;
    for(i=no->contagem; codigo < no->chaves[i-1] && i > 0; i--){
        no->chaves[i] = no->chaves[i-1];
        no->offset[i] = no->offset[i-1];
        no->ponteiros[i+1] = no->ponteiros[i];
    }
    no->contagem++;
    no->chaves[i] = codigo;
    no->offset[i] = offset;
    no->ponteiros[i+1] = rrn;
}

void dividirNo(int codigo, int offset, int rnn, No* noAntigo, int* codigoPromovido, int* offsetPromovido, int* filhoPromovido, No* novoNo){
    int i;
    int meio;
    int tempCodigos[MAXCHAVE+1];
    int tempOffset[MAXCHAVE+1];
    int tempPonteiro[MAXCHAVE+2];
    
    for(i=0; i<MAXCHAVE; i++){
        tempCodigos[i] = noAntigo->chaves[i];
        tempOffset[i] = noAntigo->offset[i];
        tempPonteiro[i] = noAntigo->ponteiros[i];
    }
    tempPonteiro[i] = noAntigo->ponteiros[i];
    for(i=MAXCHAVE; codigo < tempCodigos[i-1] && i > 0; i--){
        tempCodigos[i] = tempCodigos[i-1];
        tempOffset[i] = tempOffset[i-1];
        tempPonteiro[i+1] = tempPonteiro[i];
    }
    tempCodigos[i] = codigo;
    tempOffset[i] = offset;
    tempPonteiro[i+1] = rnn;
    
    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, -sizeof(int), FINAL);
    *filhoPromovido = ftell(arvoreB) / sizeof(No);
    fclose(arvoreB);

    int j;
    for(j=0; j < MAXCHAVE; j++){
        novoNo->chaves[j] = VAGO;
        novoNo->offset[j] = VAGO;
        novoNo->ponteiros[j] = NULO;
    }
    novoNo->ponteiros[MAXCHAVE] = NULO;

    for(i=0; i < MINCHAVE; i++){
        noAntigo->chaves[i] = tempCodigos[i];
        noAntigo->offset[i] = tempOffset[i];
        noAntigo->ponteiros[i] = tempPonteiro[i];
        noAntigo->chaves[i+1] = VAGO;
        noAntigo->offset[i+1] = VAGO;
        noAntigo->ponteiros[i+2] = NULO;
    }
    novoNo->chaves[0] = tempCodigos[3];
    novoNo->offset[0] = tempOffset[3];
    novoNo->ponteiros[0] = tempPonteiro[3];
    novoNo->ponteiros[1] = tempPonteiro[4];
    novoNo->contagem = MAXCHAVE - MINCHAVE;
    noAntigo->ponteiros[2] = tempPonteiro[2];
    noAntigo->contagem = MINCHAVE;
    *codigoPromovido = tempCodigos[MINCHAVE];
    *offsetPromovido = tempOffset[MINCHAVE];
    printf("Divisao de no!\n");
    printf("Codigo %d promovido!\n", *codigoPromovido);
}

void listarTodos(int rnn){
    No no;
    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, rnn * sizeof(No) + 4, INICIO);
    fread(&no, sizeof(No), 1, arvoreB);
    fclose(arvoreB);

    if (no.ponteiros[0] == NULO && no.ponteiros[1] == NULO &&
        no.ponteiros[2] == NULO && no.ponteiros[3] == NULO){
        int i;
        FILE* data;
        Registro registro;

        data = fopen("./temp/data.bin", "rb");
        for(i=0; i<no.contagem; i++){
            if(no.chaves[i] != VAGO){
                fseek(data, no.offset[i], INICIO);
                fread(&registro, sizeof(Registro), 1, data);
                printf("%d %s %s %s\n", registro.cod, registro.nome, registro.seg, registro.tipo);
            }
        }
        return;
    }
    int i;
    for(i=0; i<4; i++){
        if(no.ponteiros[i] != NULO){
            listarTodos(no.ponteiros[i]);
            if(i < 3 && no.chaves[i] != VAGO){
                FILE* data;
                Registro registro;

                data = fopen("./temp/data.bin", "rb");
                fseek(data, no.offset[i], INICIO);
                fread(&registro, sizeof(Registro), 1, data);
                printf("%d %s %s %s\n", registro.cod, registro.nome, registro.seg, registro.tipo);
            }
        }
    }
}

void buscarCodigo(int codigo){
    FILE* arvoreB;

    arvoreB = fopen("./temp/arvoreB.bin", "rb");
    fseek(arvoreB, 4, INICIO);
    int encontrado;
    int posicao;
    int pagina = -1;
    No no;

    do{
        fread(&no, sizeof(No), 1, arvoreB);
        encontrado = buscarNo(codigo, &no, &posicao);
        pagina++;
        if(feof(arvoreB))
            break;
    }while(!encontrado);
    if(encontrado){
        printf("Codigo %d encontrado, pagina %d, posicao %d!\n",
        codigo, pagina, posicao);
    }else{
        printf("Codigo %d nao encontrado!\n", codigo);
    }
}

void carregarArquivos(){
    FILE *insere;

	insere = fopen("./temp-testes/insere.bin", "rb");
	fread(&tempInsere, sizeof(struct reg), QUANTIDADEINSERIR, insere);
	fclose(insere);

    FILE *remove;

	remove = fopen("./temp-testes/remove.bin", "rb");
	fread(&tempRemove, sizeof(int), QUANTIDADEREMOVER, remove);
	fclose(remove);

    FILE *busca;

	busca = fopen("./temp-testes/busca.bin", "rb");
	fread(&tempBusca, sizeof(int), QUANTIDADEBUSCA, busca);
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
    if (fopen("./temp/arvoreB.bin", "rb") != NULL)
        printf(" 2. arvoreB.bin\n");

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
            myfile = fopen("./temp/arvoreB.bin", "rb");
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