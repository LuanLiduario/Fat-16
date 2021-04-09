#include "tp.h"
int init()
{
	int i = 0;
	FILE *arq = fopen("fat.part", "wb");
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		return 0;
	}
	//boot_block
	uint8_t boot_block = 0xbb;
	for (i; i < CLUSTER_SIZE; i++)
	{
		fwrite(&boot_block, sizeof(boot_block), 1, arq);
	}
	//preencher tabela fat
	fat[0] = 0xfffd;
	i = 1;
	for (i; i < 10; i++)
	{
		fat[i] = 0xfffe;
	}
	fat[9] = 0xffff;
	for (i; i < 4096; i++)
	{ //definir o restante das
		fat[i] = 0x00;
	}
	//FAT
	fwrite(fat, sizeof(uint16_t), 4096, arq); //salva o fat no arquivo de 4096 entradas de 16 bits
	//Root dir
	memset(root_dir, 0x00, sizeof(root_dir));				//1 cluster
	fwrite(root_dir, sizeof(dir_entry_t), 32, arq); //salva o root_dir no arquivo (32 entradas de diretório)
	//Data Cluesters
	uint8_t data[CLUSTER_SIZE];
	memset(data, 0x00, CLUSTER_SIZE); //cluster
	i = 0;
	for (i; i < 4086; i++)
	{																			// 4086 clusters
		fwrite(data, 1, CLUSTER_SIZE, arq); //salvar cluster no arquivo fat.part
	}
	fclose(arq);
	return 1;
}

int load()
{
	FILE *arq = fopen("fat.part", "rb");
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		return 0;
	}
	fseek(arq, CLUSTER_SIZE, SEEK_SET);	//Aponta para o FAT após o boot_block de 1024 bytes
	fread(fat, sizeof(uint16_t), 4096, arq);// Ler o FAT com 4096 entradas de 16 bits
	fread(root_dir, sizeof(dir_entry_t), 32, arq); // Ler diretorio raiz com 32 entradas
	fclose(arq);
	return 1;
}

void ls(char *diretorio){
	if(diretorio == NULL){
		printf("Caminho invalido\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho
	//char *dirAtual = (char *)malloc(sizeof(char) * 50); // Recebe nome do diretorio atual
	int index;
	index = procurarDIr(diretorio, dirAtual);
	if(index != -1){
		data_cluster data = lerCluster(index);
		printf("DIRETORIOS:\n");
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) {
					if(data.dir[i].attributes == 1) {
						printf("%s ", data.dir[i].filename); 
					}
				}
			}
			printf("\nARQUIVOS:\n");
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) {
					if(data.dir[i].attributes == 0) {
						printf("%s ", data.dir[i].filename); 
					}
				}
			}
			printf("\n");
	}
	free(dirAtual);
}

void mkdir(char *diretorio){
	if(diretorio == NULL || strcmp(diretorio,"") == 0){
		printf("Caminho invalido\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho
	//char *dirAtual = (char *)malloc(sizeof(char) * 50); // Recebe nome do diretorio atual
	int index;
	index = procurarDIr(diretorio, dirAtual);
	if(index != -1){
		data_cluster data = lerCluster(index);
		data_cluster novoDiretorio;
		int indexBloco,j;
		
		memset(novoDiretorio.dir, 0x0000, 32 * sizeof(dir_entry_t));

		for( j = 0; j < 32; j++){
			if(strcmp(data.dir[j].filename, dirAtual) == 0){ //verifica se ja existe diretorio com este nome
				printf("DIRETORIO JA EXISTE\n");
				free(dirAtual);
				return;
			}
			if(data.dir[j].first_block == 0) {
				if(strlen(dirAtual) < 18){
					strncpy(data.dir[j].filename, dirAtual, strlen(dirAtual));
				}
				else{
					strncpy(data.dir[j].filename, dirAtual, 17);
				}
				data.dir[j].attributes = 1; //1 - diretorio
				for(indexBloco = 10; indexBloco < 4096; indexBloco++){//percorre a fat
					if(fat[indexBloco] == 0x0000){//procura uma posição vazia
						data.dir[j].first_block = indexBloco;
						fat[indexBloco] = 0xffff; 
						atualizarFat();//atualiza a fat no arquivo
						salvarCluster(index, data); //atualiza o cluster
						salvarCluster(indexBloco, novoDiretorio); // salva o novo diretorio no arquivo
						free(dirAtual);
						return;
					}
				}
			}
		}

		printf("DIRETORIO CHEIO\n");
	}
	free(dirAtual);
}

void create(char *diretorio){
	if(diretorio == NULL || strcmp(diretorio,"") == 0){
		printf("Caminho invalido\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho
	int index;
	index = procurarDIr(diretorio, dirAtual);
	if(index != -1){
		data_cluster data = lerCluster(index);
		data_cluster novoDiretorio;
		int indexBloco,j;
		memset(novoDiretorio.dir, 0x0000, 32 * sizeof(dir_entry_t));
		for( j = 0; j < 32; j++){
			if(strcmp(data.dir[j].filename, dirAtual) == 0){ //verifica se ja existe arquivo com este nome
				printf("ARQUIVO JA EXISTE\n");
				free(dirAtual);
				return;
			}
			if(data.dir[j].first_block == 0) {
				if(strlen(dirAtual) < 18){
					strncpy(data.dir[j].filename, dirAtual, strlen(dirAtual));
				}
				else{
					strncpy(data.dir[j].filename, dirAtual, 17);
				}
				data.dir[j].attributes = 0; //0 - arquivo
				for(indexBloco = 10; indexBloco < 4096; indexBloco++){//percorre a fat
					if(fat[indexBloco] == 0x0000){//procura uma posição vazia
						data.dir[j].first_block = indexBloco;
						fat[indexBloco] = 0xffff; 
						atualizarFat();//atualiza a fat no arquivo
						salvarCluster(index, data); //atualiza o cluster
						salvarCluster(indexBloco, novoDiretorio); // salva o novo diretorio no arquivo
						free(dirAtual);
						return;
					}
				}
			}
		}

		printf("DIRETORIO CHEIO\n");
	}
	free(dirAtual);
}

int procurarDIr(char *diretorio, char * dirAtual){
	int j = 0, index = 9, k = 0;
	data_cluster data;
	for(int i = 0; i <= strlen(diretorio); i++){//percorre o diretorio recebido
		if(diretorio[i] != '/' && diretorio[i] != '\n' && diretorio[i] != '\0'){
			dirAtual[j] = diretorio[i];
			j++;
		}else{
			dirAtual[j] = '\0';
			data = lerCluster(index);
			for(k = 0; k < 32; k++){
				if( strcmp(data.dir[k].filename, dirAtual) == 0 && data.dir[k].first_block != 0 && data.dir[k].attributes == 1 ) {//achei dir
					index = data.dir[k].first_block;
					break;
				}
			}
			if(k == 32){// nao encontrou diretorio
				if(( diretorio[i+1] != '\0' && diretorio[i] != '\0')){//verifica se é o ultimo
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;
				}else{
					return index;
				}
			}else{
				j = 0;
				strcpy(dirAtual, "");
			}
		}
	}

}

void atualizarFat(){
	FILE *arq = fopen("fat.part", "rb+");
	if(arq == NULL) {
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, CLUSTER_SIZE, SEEK_SET);//Aponta para o FAT após o boot_block de 1024 bytes
	fwrite(fat, sizeof(uint16_t), 4096, arq);//Salva fat
	fclose(arq);
}

data_cluster lerCluster(int index)
{
	FILE *arq = fopen("fat.part", "rb");
	data_cluster cluster;
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	memset(&cluster, 0x00, CLUSTER_SIZE);
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET);
	fread(&cluster, CLUSTER_SIZE, 1, arq);
	fclose(arq);
	return cluster;
}

void salvarCluster(int index, data_cluster cluster){
	FILE *arq = fopen("fat.part", "rb+");
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET); // Vai ao índice desejado
	fwrite(&cluster, CLUSTER_SIZE, 1, arq); // Lê o union e escreve no cluster
	fclose(arq);
}

void separaString(char *string1, char *string2, char *string3,char * separador)
{
	int tam = strlen(string1);
	strcpy(string2, strtok(string1,separador));
	if(tam == strlen(string2)){
		strcpy(string3, "");
	}else{
		//a segunda parte do ponto onde parou o último uso da função strtok e vai até o final
		strcpy(string3, strtok(NULL, "\n"));
	}
}
