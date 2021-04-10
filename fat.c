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
	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho
	//char *dirAtual = (char *)malloc(sizeof(char) * 50); // Recebe nome do diretorio atual

	int index;
	index = procurarDIr(diretorio, dirAtual,1);
	// if(getNumDiretorios(diretorio)==0){
	// 	index == 9;
	// }
	if(index != -1 && strcmp(dirAtual,"") != 0){
		printf("OLOKO:::%s>\n",dirAtual);
		data_cluster data = lerCluster(index);
		data_cluster novoDiretorio;
		int indexBloco,j;
		
		memset(novoDiretorio.dir, 0x0000, 32 * sizeof(dir_entry_t));

		for( j = 0; j < 32; j++){
			printf("%s-%s\n",dirAtual,data.dir[j].filename);
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
	}else{
		printf("DIR NAO ENCONTRADO\n");
		printf("(%s)\n",dirAtual);
		printf("%d\n",index);
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

// void unlink(char *diretorio){
// 	if(diretorio == NULL || strcmp(diretorio,"") == 0){
// 		printf("Caminho invalido\n");
// 		return;
// 	}
// 	printf("Caminho :::%s\n",diretorio);
// 	char *dirAtual = (char *)malloc(sizeof(char) * 50);// Recebe o nome do diretorio no caminho que devera ser apagado
// 	int index;
// 	index = procurarDirPai(diretorio, dirAtual);
// 	// if(index != -1){
// 	// 	data_cluster aux;
// 	// 	data_cluster data = lerCluster(index);// ler cluster que deve ser apagado
// 	// 	dir_entry_t dirVazio;
// 	// 	int j;
// 	// 	memset(&dirVazio, 0x00, 32);
// 	// 	printf("PASTA A SER EXCLUIDA :::%s:::\n",dirAtual);
// 	// 	for(int i = 0; i < 32; i++){
// 	// 		if(data.dir[i].first_block != 0) { 
// 	// 			if(data.dir[i].attributes == 1){ //diretorio
// 	// 				aux = lerCluster(data.dir[i].first_block);
// 	// 				for(j = 0; j < 32 ; j++){
// 	// 					if(aux.dir[i].first_block != 0){
// 	// 						break;
// 	// 					}
// 	// 				}
// 	// 				if(j == 32){
// 	// 					printf("ESVAZIE O DIRETORIO ANTES DE FAZER UNLINK\n");
// 	// 					return;
// 	// 				}
// 	// 				else{
// 	// 					fat[data.dir[i].first_block] = 0;
// 	// 					data.dir[i] = dirVazio;
// 	// 					salvarCluster(index, data);
// 	// 					atualizarFat();
// 	// 					printf("SUCESSO\n");
// 	// 					return;
// 	// 				}
// 	// 			} 
// 	// 		}
// 	// 	}
// 	// }
// 	free(dirAtual);
// }
int procurarDIr(char *diretorio, char *dirAtual, int procura){// 1 - atual nao existe, 2 - atual existe, 3 arquivo
	int index = 9, j =0, k = 0;
	if(strcmp(diretorio, "/") == 0){
		return index;//index
	}
	if(diretorio[0] != '/'){
		printf("PARAMETRO INVALIDO\n");
		return -1;
	}
	int numDiretorios = getNumDiretorios(diretorio);
	data_cluster data = lerCluster(index);
	for(int i = 1; i <= strlen(diretorio); i++){
		if(diretorio[i] == '/' || diretorio[i] == '\0' || diretorio[i] == '\n'){//fim do nome de um diretorio
			dirAtual[j] = '\0';
			

			if(numDiretorios == 0){
				printf("ESSE DEVE SER recebido <%s>\n",dirAtual);
			}

		for(k = 0; k< 32; k++){

			if(strcmp(data.dir[k].filename,dirAtual) == 0 && data.dir[k].first_block != 0) {// verifica se acha um diretorio no cluster com o nome do dirAtual
				if(data.dir[k].attributes == 1){//existe diretorio
					// if(numDiretorios != 0){
						index = data.dir[k].first_block;
						// if(numDiretorios == 0){
						// 	return index;
						// }

					// }
					data = lerCluster(index);
					break;
				} 
				else{//arquivo
					if(numDiretorios == 0){
						printf("E UM ARQUIVO\n");
						return index;
					}
				}
			}
		}

		if(procura == 1){//se for uma procura por um diretorio que será criado não deverá encontrar
			// if(k == 32 && numDiretorios == 0){// percorreu todo cluster e não encontrou nenhum diretorio com nome de dirAtual
			// 	return index;
			// }else if((numDiretorios != 0 && k == 32)){
			// 	printf("DIRETORIO NAO ENCONTRADO\n");
			// 	return -1;
			// }
			if(k == 32 || numDiretorios == 0){
				if(numDiretorios != 0){
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;
				}
				if(k < 32){
					printf("DIRETORIO JA EXISTE\n");
					return -1;
				}
				return index;
			}
		}else if(procura == 2){// se for uma procura por um diretorio que ja existe deverá encontrar
			if(k == 32 && numDiretorios == 0){
				printf("DIRETORIO NAO ENCONTRADO\n");
				return -1;
			}else if(k < 32 && numDiretorios == 0){// percorreu todo cluster e encontrou um diretorio com nome de dirAtual
				return index;
			}
		}
			//printf("J>>%d<<%d>>>DIR: %s\n",k,numDiretorios,dirAtual);
			numDiretorios--;
			j = 0;
		}else{
			dirAtual[j] = diretorio [i];
			j++;
		}
	}
	return index;
}

// int procurarDIr(char *diretorio, char * dirAtual){
// 	int j = 0, index = 9, k = 0;
// 	data_cluster data;
// 	int numDiretorios = getNumDiretorios(diretorio);//tenho o numero de diretorios
// 	if(diretorio[0] == '/'){//verifica se é um paramentro valido
// 		if(strcmp(diretorio, "/") == 0){//diretorio raiz
// 			return index;
// 		}
// 		if(numDiretorios == 0 && strlen(diretorio) == 1){
// 			for(int i = 1; i <=strlen(diretorio);i++){
// 				if(diretorio[i] == '/' || diretorio[i] == '\n' || diretorio[i] == '\0'){
// 					dirAtual[j] = '\0';
// 					return index;
// 				}
// 				dirAtual[j] = diretorio[i];
// 				j++;
// 			}
// 			return index;
// 		}
// 		data = lerCluster(index);//ler diretorio raiz
// 		for(int i = 1; i <= strlen(diretorio); i++){//percorre o diretorio recebido
// 			if(diretorio[i] != '/' && diretorio[i] != '\n' && diretorio[i] != '\0'){
// 				dirAtual[j] = diretorio[i];
// 				j++;
// 			}else{
// 				dirAtual[j] = '\0';
// 				printf("%d TEM Q TER UMA MERDA DE STRING :::%s\n",numDiretorios,dirAtual);
// 				for(k = 0; k < 32; k++){
// 					if( strcmp(data.dir[k].filename, dirAtual) == 0 && data.dir[k].first_block != 0 && data.dir[k].attributes == 1 ) {//achei dir
// 						index = data.dir[k].first_block;
// 						data = lerCluster(index);
// 						break;
// 					}
// 				}
// 				if(k == 32){// nao encontrou diretorio
// 					printf("%s,%d\n",dirAtual,numDiretorios );
// 					if(numDiretorios != 0){//verifica se é o ultimo
// 						printf("DIRETORIO NAO ENCONTRADO\n");
// 						return -1;
// 					}else{
// 						return index;
// 					}
// 				}else{
// 					j = 0;
// 					strcpy(dirAtual, "");
// 				}
// 				numDiretorios--;
// 			}
// 		}

// 	}else{
// 		printf("PARAMETRO INVALIDO\n");
// 		return -1;
// 	}


// }

// int procurarDIr(char *diretorio, char * dirAtual){
// 	int j = 0, index = 9, k = 0,n;
// 	n = getNumDiretorios(diretorio);
// 	data_cluster data;
// 	if(diretorio[0] == '/'){
// 		if(diretorio == NULL || strcmp(diretorio, " ") == 0 || strcmp(diretorio, "/") == 0){
// 			for(int i = 1; i <=strlen(diretorio);i++){
// 				if(diretorio[i] == '/' || diretorio[i] == '\n' || diretorio[i] == '\0'){
// 					dirAtual[j] = '\0';
// 					return index;
// 				}
// 				dirAtual[j] = diretorio[i];
// 				j++;
// 			}
// 		}
// 		int i =0;
// 		data = lerCluster(index);
// 		n--;
// 		do{
// 			printf("DO\n");
// 			if(diretorio[i] == '/' || diretorio[i] == '\n' || diretorio[i] == '\0'){// /teste/n
// 				dirAtual[j] = '\0';
// 				printf("DIR ATUAL ::: %s\n",dirAtual);
// 				for(k = 0; k < 32; k++){
// 					if( strcmp(data.dir[k].filename, dirAtual) == 0 && data.dir[k].first_block != 0 && data.dir[k].attributes == 1 ) {//achei dir
// 						index = data.dir[k].first_block;
// 						data = lerCluster(index);
// 						break;
// 					}
// 				}
// 				if(k == 32){// nao encontrou diretorio
// 					if( n == 1 )
// 					{//verifica se é o ultimo
// 						printf("2DIR ATUAL ::: %s\n",dirAtual);
// 						return index;
// 					}else{
// 						printf("DIRETORIO NAO ENCONTRADO\n");
// 						return -1;
// 					}
// 				}else{
// 					n--;
// 					j = 0;
// 					strcpy(dirAtual, "");
// 				}
// 			}else{
// 				dirAtual[j] = diretorio[i];
// 				j++;
// 			}
// 			i++;
// 		}while(i <= strlen(diretorio)+1);

// 		return -1;

// 	}else{
// 		printf("PARAMETRO INVALIDO\n");
// 		return -1;
// 	}

	

	
// 	// if(diretorio == NULL || strcmp(diretorio, " ") == 0 ||strcmp(diretorio, "/") == 0){
// 	// 	return index;
// 	// }else{
// 	// 	if(diretorio[0] == '/'){
// 	// 		int numDiretorios,k,numDiretoriosFalta;
// 	// 		numDiretorios = getNumDiretorios(diretorio);
// 	// 		printf("NUM %d\n",numDiretorios);
// 	// 		dirAtual = strtok(diretorio, "/");
// 	// 		numDiretoriosFalta = numDiretorios;
// 	// 		for(int i = 1; i < numDiretorios; i++){
// 	// 			numDiretoriosFalta--;
// 	// 			data = lerCluster(index);
// 	// 			for(k = 0; k < 32; k++){
// 	// 				if( strcmp(data.dir[k].filename, dirAtual) == 0 && data.dir[k].first_block != 0 && data.dir[k].attributes == 1 ) {//achei dir
// 	// 					index = data.dir[k].first_block;
// 	// 					data = lerCluster(data.dir[k].first_block);
// 	// 					break;
// 	// 				}
// 	// 			}
// 	// 			if(k == 32){
// 	// 				if(numDiretoriosFalta == 0){
// 	// 					return index;
// 	// 				}else{
// 	// 					printf("DIRETORIO NAO ENCONTRADO\n");
// 	// 					return -1;
// 	// 				}
// 	// 			}
// 	// 			dirAtual = strtok(NULL, "/");
// 	// 		}
// 	// 	}else{
// 	// 		printf("DIRETORIO NAO ENCONTRADO\n");
// 	// 		return -1;
// 	// 	}
// 	// }
// 	// printf("PASSOU TD\n");
// 	// return -1;
// }

int getNumDiretorios(char *caminho){
	int numDiretorios = 0;
	int i = 0;
	do{
		if(caminho[i] == '/'){
			numDiretorios++;
		}
		i++;
	}while(caminho[i] != '\0' && caminho[i] != '\n');
	return numDiretorios-1;
}

int procurarDirPai(char *diretorio, char * dirPai){
	int j = 0, index = 9, k = 0,indexAnterior = 9;
	char *aux = (char *)malloc(sizeof(char) * 50);
	data_cluster data;
	for(int i = 0; i <= strlen(diretorio); i++){//percorre o diretorio recebido
		if(diretorio[i] != '/' && diretorio[i] != '\n' && diretorio[i] != '\0'){
			aux[j] = diretorio[i];
			j++;
		}else{
			aux[j] = '\0';
			data = lerCluster(index);
			indexAnterior = index;
			for(k = 0; k < 32; k++){
				if( strcmp(data.dir[k].filename, aux) == 0 && data.dir[k].first_block != 0 && data.dir[k].attributes == 1 ) {//achei dir
					index = data.dir[k].first_block;
					break;
				}
			}
			if(k == 32){// nao encontrou diretorio
				if(( diretorio[i+1] != '\0' && diretorio[i] != '\0' && diretorio[i] != '\n' && diretorio[i+1] != '\n')){//verifica se é o ultimo
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;
				}else{
					strcpy(dirPai,aux);
					return indexAnterior;
				}
			}else{
				j = 0;
				strcpy(aux, "");
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
