#include "tp.h"

//diretorios
int procurarDIr(char *diretorio, char *dirAtual, int procura){// 1 - pai, 2 - atual, 3 arquivo
	int index = 9, j =0, k = 0;
	int indexPai = index;
	
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
					if(numDiretorios == 0 &&  procura == 3){
						///printf("E UM ARQUIVO\n");
						return index;
					}
				}
			}
		}

		if(procura == 1){//se for uma procura por um diretorio que será criado não deverá encontrar
			if(k == 32 || numDiretorios == 0){
				if(numDiretorios != 0){
					printf("DIRETORIO NAO ENCONTRADO\n");
					return -1;
				}
				if(k < 32){
					//printf("DIRETORIO JA EXISTE\n");
					return indexPai;
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
			indexPai = index;
		}else{
			dirAtual[j] = diretorio [i];
			j++;
		}
	}
	return -1;
}

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


//clusters
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

//strings

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


data_cluster* quebrarStringClusters(char *string, int *numClusters){
	data_cluster *clusters;
	int tamString, numClustersInteiros, numClustersFalta;
	tamString = strlen(string);
	numClustersInteiros = tamString / CLUSTER_SIZE; 
	numClustersFalta = tamString % CLUSTER_SIZE; 
	
	if(numClustersInteiros != 0){
		int restante = 0, i = i;
		if(numClustersFalta > 0){
			restante = 1;
		}
		clusters = (data_cluster*)malloc((numClustersInteiros+restante)*CLUSTER_SIZE); 
		for(i = 0; i < numClustersInteiros; i++){ //separa a string nos clusters
			memset(&(clusters)[i], 0, CLUSTER_SIZE);
			memcpy(&(clusters)[i].data, &string[(CLUSTER_SIZE)*i], CLUSTER_SIZE);
			((clusters)[i].data)[CLUSTER_SIZE] = '\0';
		}
		if(restante == 1){
			memset(&(clusters)[i], 0, CLUSTER_SIZE);
			memcpy(&(clusters)[i].data, &string[i * (CLUSTER_SIZE)], numClustersFalta);
			(clusters)[i].data[numClustersFalta] = '\0';
		}
		(*numClusters) = numClustersInteiros + restante;
		return clusters;
	}else{
		clusters = (data_cluster*)malloc(CLUSTER_SIZE);
		memset(clusters, 0, CLUSTER_SIZE);
		memcpy(&(clusters)[0], string, tamString);
		(*numClusters) = 1;
		return clusters; 
	}
}

// fat
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





