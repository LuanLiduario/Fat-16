#include "tp.h"

void init(){
	FILE *arq = fopen("fat.part","wb");
	if(arq == NULL){
		printf("ERRO ao abrir fat\n");
		return ;
	}
	//boot_block
	uint8_t boot_block = 0xbb;
	for(int i = 0; i < 1024; i++){
		fwrite(&boot_block,sizeof(boot_block),1, arq);
	}
	//preencher tabela fat
	fat[0] = 0xfffd;
	fat[9] = 0xffff;
	int i = 1; 
	for(i ; i < 10; i++){
		fat[i] = 0xfffe;
	}
	printf("%d\n",i);
	for(i ; i < 4096; i++){
		fat[i] = 0x00;//tirar o 00
	}
	//FAT
	fwrite(fat, sizeof(uint16_t), 4096, arq);//salva o fat no arquivo
	memset(root_dir, 0, 1024);//1 cluster (32 entradas de diretório))
	fwrite(root_dir, sizeof(dir_entry_t), 32, arq);//salva o root_dir no arquivo

	/* inicializa o restando do disco com o valor 0 */
	uint8_t t[CLUSTER_SIZE];
	memset(t, 0, CLUSTER_SIZE);
	for(int i = 0; i < 4086; i++)
		fwrite(t, 1, CLUSTER_SIZE, arq);

	fclose(arq);
}

void load(){
	FILE *arq = fopen("fat.part", "rb");
	if(arq == NULL) {
		printf("ERRO ao abrir fat\n");
		return;
	}
	fseek(arq, 1024, SEEK_SET); //Aponta para o FAT
	fread(fat, sizeof(uint16_t), 4096, arq);// Ler o FAT com 4096 entradas de 16 bits
	fread(root_dir, sizeof(dir_entry_t), 32, arq);// Ler FAT com 32 entradas
	fclose(arq);
	printf("FAT carregada com sucesso\n");
}