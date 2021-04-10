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
	index = procurarDIr(diretorio, dirAtual,2);
	if(index != -1){
		data_cluster data = lerCluster(index);
		printf("DIRETORIOS:\n");
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0 && data.dir[i].attributes == 1) {
					printf("%s ", data.dir[i].filename); 
				}
			}
			printf("\nARQUIVOS:\n");
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0 && data.dir[i].attributes == 0) {
						printf("%s ", data.dir[i].filename); 
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
	if(diretorio == "/"){
		printf("NAO E POSSIVEL CRIAR O DIRETORIO RAIZ\n");
		return;
	}
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho
	//char *dirAtual = (char *)malloc(sizeof(char) * 50); // Recebe nome do diretorio atual

	int index;
	index = procurarDIr(diretorio, dirAtual,1);
	// if(getNumDiretorios(diretorio)==0){
	// 	index == 9;
	// }
	if(index != -1 && strcmp(dirAtual,"") != 0){
		data_cluster data = lerCluster(index);
		data_cluster novoDiretorio;
		int indexBloco,j;
		
		memset(novoDiretorio.dir, 0x0000, 32 * sizeof(dir_entry_t));

		for( j = 0; j < 32; j++){
			printf("%s-%s\n",dirAtual,data.dir[j].filename);
			if(strcmp(data.dir[j].filename, dirAtual) == 0 && data.dir[j].attributes == 1){ //verifica se ja existe diretorio com este nome
				printf("DIRETORIO JA EXISTE\n");
				free(dirAtual);
				return;
			}
			if(data.dir[j].first_block == 0) {
				break;
			}
		}


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
	index = procurarDIr(diretorio, dirAtual,1);
	if(index != -1){
		data_cluster data = lerCluster(index);
		data_cluster novoDiretorio;
		int indexBloco,j;
		memset(novoDiretorio.dir, 0x0000, 32 * sizeof(dir_entry_t));
		for( j = 0; j < 32; j++){
			if(strcmp(data.dir[j].filename, dirAtual) == 0 && data.dir[j].attributes == 0){ //verifica se ja existe arquivo com este nome
				printf("ARQUIVO JA EXISTE\n");
				free(dirAtual);
				return;
			}
			if(data.dir[j].first_block == 0) {
				break;
			}
		}

		if(j == 32){
			printf("DIRETORIO CHEIO\n");
			free(dirAtual);
			return;
		}

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
	free(dirAtual);
}

void unlink(char *diretorio){
	if(diretorio == NULL || strcmp(diretorio,"") == 0){
		printf("Caminho invalido\n");
		return;
	}
	if(diretorio == "/"){
		printf("NAO E POSSIVEL EXCLUIR O DIRETORIO RAIZ\n");
		return;
	}
	printf("Caminho :::%s\n",diretorio);
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho que devera ser apagado
	int index;
	index = procurarDIr(diretorio, dirAtual,1);
	if(index != -1){
		data_cluster aux;
		data_cluster data = lerCluster(index);// ler cluster que deve ser apagado
		dir_entry_t dirVazio;
		int j;
		memset(&dirVazio, 0x00, 32);
		printf("PASTA A SER EXCLUIDA :::%s:::\n",dirAtual);
		for(int i = 0; i < 32; i++){
			if(data.dir[i].first_block != 0 && strcmp(dirAtual,data.dir[i].filename) == 0){
				if(data.dir[i].attributes == 1){
					aux = lerCluster(data.dir[i].first_block);
					for(j = 0; j < 32 ; j++){
						if(aux.dir[j].first_block != 0){
							break;
						}
					}
					if(j == 32){
						fat[data.dir[i].first_block] = 0;
						data.dir[i] = dirVazio;
						salvarCluster(index, data);
						atualizarFat();
						printf("DIRETORIO APAGADO\n");
						free(dirAtual);
						return;
					}else{
						printf("ESVAZIE O DIRETORIO ANTES DE FAZER UNLINK\n");
						free(dirAtual);
						return;
					}
				}
				if(data.dir[i].attributes == 0){
					int indexaux;
					j = data.dir[i].first_block;
					while(fat[j] != 0xffff){ 
						index = j;
						j = fat[indexaux];
						fat[indexaux] = 0x0000;
					}
					data.dir[i] = dirVazio; 
					salvarCluster(index, data);
					atualizarFat();
					printf("ARQUIVO APAGADO\n");
					free(dirAtual);
					return;
				}
				printf("ERRO DE attributes\n");
			}
		}
	}
	free(dirAtual);
}

void write(char * paramentros){
	char *diretorio = (char *)malloc(sizeof(char) * 50);
	char *string = (char *)malloc(sizeof(char) * 50);
	char *dirAtual = (char *)malloc(sizeof(char) * 50);
	separaString(paramentros,string,diretorio," ");

	if(strcmp(string,"") == 0){
		printf("string vazia\n");
	}

	int index;
	index = procurarDIr(diretorio,dirAtual,3);

	if(index != -1){
			data_cluster data = lerCluster(index);
			int i;
			for( i = 0; i < 32; i++){
				if(strcmp(data.dir[i].filename,dirAtual) == 0 && data.dir[i].first_block != 0 && data.dir[i].attributes == 0){
					index = data.dir[i].first_block;
					break;
				}
			}
			if(i == 32){
				printf("ARQUIVO NAO ENCONTRADO NO DIRETORIO\n");
				return;
			}
			int numClusters;
			data_cluster *clusters;
			clusters = quebrarStringClusters(string, &numClusters); 
			int index_final = index, indexBloco;
			while(fat[index_final] != 0xffff){ 
				indexBloco = index_final;
				index_final = fat[index_final]; 
				fat[indexBloco] = 0x0000;
			}
			fat[index_final] = 0x0000;
			fat[index] = 0xffff;
			salvarCluster(index, clusters[0]);
			if(numClusters > 1){
				for(i = 1; i < numClusters; i++){
					indexBloco;
					for(indexBloco = 10; indexBloco < 4096 ; indexBloco++){
						if(fat[indexBloco] != 0x0000){
							break;
						}
					}
					fat[index] = indexBloco;
					index = indexBloco;
					fat[index] = 0xffff;
					salvarCluster(index, clusters[i]);
				}
			}
			atualizarFat();
			printf("String salva\n");
	}else{
		printf("ARQUIVO NAO ENCONTRADO\n");
	}

}




