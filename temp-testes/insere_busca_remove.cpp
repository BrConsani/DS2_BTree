#include<stdio.h>
#include<conio.h>
#include<string.h>

int main() {
    FILE *fd;
    
    //////////////////////////////   
    struct livro {
        int codigo;
        char nome[50];
        char seguradora[50];
        char tipo_seg[30];
    } vet[6] = {{1, "Nome-1", "Maritima", "Automovel"},
                {2, "Nome-2", "Maritima", "Automovel"},
                {3, "Nome-3", "Porto Seguro", "Residencia"},
                {4, "Nome-4", "Zurich", "Vida"},
                {5, "Nome-5", "Bradesco", "Previdencia"},
                {6, "Nome-6", "Zurich", "Vida"}};
       
    fd = fopen("insere.bin", "w+b");
    fwrite(vet, sizeof(vet), 1, fd);
    fclose(fd);
    
    //////////////////////////////
	struct busca {
        int codigo;
    } vet_b[4] = {3, 7, 5, 2};
       
    fd = fopen("busca.bin", "w+b");
    fwrite(vet_b, sizeof(vet_b), 1, fd);
    fclose(fd);    
    
    //////////////////////////////
    struct remove {
        int codigo;
    } vet_r[4] = {1, 6, 7, 2};
       
    fd = fopen("remove.bin", "w+b");
    fwrite(vet_r, sizeof(vet_r), 1, fd);
    fclose(fd);
}
