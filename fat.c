#include "tp.h"

int init(){
	int i = 0; 
	FILE *arq = fopen("fat.part","wb");
	if(arq == NULL){
		printf("ERRO ao abrir arquivo fat\n");
		return 0;
	}
	//boot_block
	uint8_t boot_block = 0xbb;
	for(i  ; i < CLUSTER_SIZE; i++){
		fwrite(&boot_block,sizeof(boot_block),1, arq);
	}
	//preencher tabela fat
	fat[0] = 0xfffd;
	i = 1; 
	for(i ; i < 10; i++){
		fat[i] = 0xfffe;
	}
	fat[9] = 0xffff;
	for(i ; i < 4096; i++){//definir o restante das
		fat[i] = 0x00;
	}
	//FAT
	fwrite(fat, sizeof(uint16_t), 4096, arq);//salva o fat no arquivo de 4096 entradas de 16 bits
	//Root dir
	memset(root_dir, 0x00,sizeof(root_dir));//1 cluster 
	fwrite(root_dir, sizeof(dir_entry_t), 32, arq);//salva o root_dir no arquivo (32 entradas de diretório)
	//Data Cluesters
	uint8_t data[CLUSTER_SIZE];
	memset(data, 0x00, CLUSTER_SIZE);//cluster
	i = 0;
	for(i ; i < 4086; i++){// 4086 clusters
		fwrite(data, 1, CLUSTER_SIZE, arq);//salvar cluster no arquivo fat.part
	}
	fclose(arq);
	return 1;
}

int load(){
	FILE *arq = fopen("fat.part", "rb");
	if(arq == NULL) {
		printf("ERRO ao abrir arquivo fat\n");
		return 0;
	}
	fseek(arq,CLUSTER_SIZE, SEEK_SET); //Aponta para o FAT após o boot_block de 1024 bytes
	fread(fat,sizeof(uint16_t),4096,arq);// Ler o FAT com 4096 entradas de 16 bits
	fread(root_dir,sizeof(dir_entry_t),32,arq);// Ler diretorio raiz com 32 entradas
	fclose(arq);
	return 1;
}

// int mkdir(char *dir){

// 	FILE *fat_part = fopen("fat.part", "rb+");

// 	if(fat_part == NULL) {
// 		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
// 		exit(1);
// 	}

// 	char **dir_list = NULL;
// 	int i;
// 	/* chama a função de quebrar o caminho em varias strings */
// 	int retorno = break_dir(dir, &dir_list); 
// 	if(retorno == -1){ //Se ouver algum erro
// 		return -1;
// 	}
// 	int index;
// 	data_cluster data;
// 	switch(dir_nav(dir_list, retorno, &index, WANT_PARENT)){
// 		case DIR_NOT_FOUND:
// 			fprintf(stderr, "Diretorio nao encontrado\n");
// 			break;
// 		case DIR_EXIST:
// 			fprintf(stderr, "Diretorio ja existe\n");
// 			break;
// 		case DIR_READY:
// 			data = read_data_cluster(index);

// 			data_cluster new_dir;
// 			memset(new_dir.dir, 0, 32 * sizeof(dir_entry_t));
// 			int pos = retorno - 1; 

// 			if(pos < 0)
// 				pos = 0;
// 			int i, achou_espaco = 0;
// 			for(int j = 0; j < 32; j++){
// 				/*testa se ja existe um diretorio com o nome desejado*/
// 				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){ 
// 					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
// 					return 1;
// 				}

// 				/*procura uma entrada de diretorio vazia*/
// 				if(achou_espaco == 0 && data.dir[j].first_block == 0) {
// 					achou_espaco = 1;
// 					i = j;
// 				}
// 			}
			
// 			/*coloca o nome no diretorio*/
// 			if(strlen(dir_list[0]) < 18)
// 				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
// 			else 
// 				strncpy(data.dir[i].filename, dir_list[pos], 17);
// 			data.dir[i].attributes = 1; //1 == diretorio, 0 == arquivo

// 			/*procura um cluster vazio no disco*/
// 			int block;
// 			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
// 			data.dir[i].first_block = block;
// 			fat[block] = 0xffff; //fim de arquivo

// 			save_fat();
			

// 			write_data_cluster(index, data); //atualiza o diretorio pai no disco
// 			write_data_cluster(block, new_dir); //grava o diretorio no disco

// 			break;
// 	}

// 	free(dir_list);
// 	fclose(fat_part);
// 	return 0;
// }


  data_cluster lerCluster(int index)
{	
	FILE *arq = fopen("fat.part", "rb");
	data_cluster cluster;
	if(arq == NULL) {
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	memset(&cluster,0x00,CLUSTER_SIZE);
	fseek(arq, index*CLUSTER_SIZE, SEEK_SET);
	fread(&cluster, CLUSTER_SIZE, 1, arq);
	fclose(arq);
	return cluster;
}

void salvarCluster(int index,  data_cluster cluster)
{
	FILE *arq = fopen("fat.part", "rb+");
	if(arq == NULL) {
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, index*CLUSTER_SIZE, SEEK_SET);
	fread(&cluster, index*CLUSTER_SIZE, 1, arq);
	fclose(arq);
}