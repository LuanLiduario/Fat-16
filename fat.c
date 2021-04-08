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

void mkdir(char *diretorio){
	if(diretorio[0] != '/'){
		printf("Parametro invalido\n");
		return;
	}
	// FILE *fat_part = fopen("fat.part", "rb+");

	// if(fat_part == NULL) {
	// 	fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
	// 	exit(1);
	// }

	do
	{

	}while(strlen(aux) != 1)

	char **dir_list = NULL;
	int i;
	/* chama a função de quebrar o caminho em varias strings */
	
	int numDir = 0;
	separaDiretorio(diretorio,&numDir); 
	

	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	

	int index;
	data_cluster data;
	switch(dir_nav(dir_list, retorno, &index, WANT_PARENT)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Diretorio ja existe\n");
			break;
		case DIR_READY:
			data = read_data_cluster(index);

			data_cluster new_dir;
			memset(new_dir.dir, 0, 32 * sizeof(dir_entry_t));
			int pos = retorno - 1; 

			if(pos < 0)
				pos = 0;
			int i, achou_espaco = 0;
			for(int j = 0; j < 32; j++){
				/*testa se ja existe um diretorio com o nome desejado*/
				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){ 
					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
					return 1;
				}

				/*procura uma entrada de diretorio vazia*/
				if(achou_espaco == 0 && data.dir[j].first_block == 0) {
					achou_espaco = 1;
					i = j;
				}
			}
			
			/*coloca o nome no diretorio*/
			if(strlen(dir_list[0]) < 18)
				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
			else 
				strncpy(data.dir[i].filename, dir_list[pos], 17);
			data.dir[i].attributes = 1; //1 == diretorio, 0 == arquivo

			/*procura um cluster vazio no disco*/
			int block;
			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
			data.dir[i].first_block = block;
			fat[block] = 0xffff; //fim de arquivo

			save_fat();
			

			write_data_cluster(index, data); //atualiza o diretorio pai no disco
			write_data_cluster(block, new_dir); //grava o diretorio no disco

			break;
	}

	free(dir_list);
	// fclose(fat_part);
	return 0;
}

void separaDiretorio(char *diretorio, int * numDir){

}

int break_dir(char *dir, char ***dir_list){/pasta1/pasta2 ...
	//se for no root
	if(dir == NULL || strcmp(dir, "/") == 0){
		return ROOT_DIR;
	}
	//se o formato estiver invalido
	if(dir[0] != '/')
		return -1;

	//conta quantos diretorios tem na cadeia
	unsigned num_dir = 0;
	for(int i = 0; dir[i] != '\0' && dir[i] != '\n'; i++){
		if(dir[i] == '/'){
			num_dir++;
		}
	}
	
	//se for um diretorio no root
	if(num_dir == 1){
		*dir_list = (char **)malloc(sizeof(char*));
		(*dir_list)[0] = (char*) malloc(18 * sizeof(char));
		(*dir_list)[0] = strtok(dir, "/");
		return 1;
	}
	
	//aloca a memoria
	*dir_list = (char **)malloc(num_dir * sizeof(char*));

	/*quebra o caminho*/
	(*dir_list)[0] = strtok(dir, "/");
	for(int i = 1; i < num_dir; i++){
		(*dir_list)[i] = strtok(NULL, "/");
	}
	return num_dir;
}

/* navega no sistema de arquivos O(n*m)*/
int dir_nav(char **dir_list, int dir_num, int *index, int want){
	if(dir_list == NULL){ //retorna o index do /
		*index = 9;
		return DIR_EXIST;
	}
	
	int i;
	*index = 9;
	data_cluster data = read_data_cluster(*index);// le o diretorio raiz
	int j;

	/*want = 0 retorna o arquivo ou diretorio, want = 1 retorna o diretorio pai*/
	for(i = 0; i < dir_num - want; i++){

		/*navega nas 32 entradas de diretorio*/
		for(j = 0; j < 32; j++){

			/* testa se existe um diretorio com o nome no caminho */
			if(data.dir[j].first_block != 0 && STR_EQUAL(data.dir[j].filename, dir_list[i])) {
				if(data.dir[j].attributes == 1){
					if(i == dir_num - 1 - want) //caso esteja no diretorio especificado
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					break;
				} 
				else{
					if(i == dir_num - 1 - want) //caso esteja no diretorio especificado
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					return NOT_A_DIR;
				}
			}
		}
		if(j == 32)
			return DIR_NOT_FOUND;
	}
	if(want == WANT_PARENT || want == WANT_PARENT)
		return DIR_READY;
	else
		return DIR_EXIST;
}
int save_fat(){
	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	fseek(fat_part, CLUSTER_SIZE, SEEK_SET); // Chega até a FAT.
	fwrite(fat, sizeof(uint16_t), 4096, fat_part);
	fclose(fat_part);
	return 0;
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

void salvarCluster(int index, data_cluster cluster)
{
	FILE *arq = fopen("fat.part", "rb+");
	if (arq == NULL)
	{
		printf("ERRO ao abrir arquivo fat\n");
		exit(1);
	}
	fseek(arq, index * CLUSTER_SIZE, SEEK_SET);
	fread(&cluster, index * CLUSTER_SIZE, 1, arq);
	fclose(arq);
}

void separaString(char *string1, char *string2, char *string3)
{
	//recebe uma string e divide ela em duas: a primeira vai até o primeiro espaço
	// char *aux = (char *)malloc(sizeof(char) * 50);
	// strcpy(aux, string1);
	strcpy(string2, strtok(string1, " "));
	if(strlen(string1) == strlen(string2)){
		strcpy(string3, "");
	}else{
		//a segunda parte do ponto onde parou o último uso da função strtok e vai até o final
		strcpy(string3, strtok(NULL, "\0"));
	}
}
