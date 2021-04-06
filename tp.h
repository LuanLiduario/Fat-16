#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CLUSTER_SIZE 1024 //numero de clusters

/*Informa¸c˜oes sobre o valor das entradas na FAT de 16 bits:
0x0000 -> cluster livre
0x0001 - 0xfffc -> arquivo (ponteiro p/ proximo cluster)
0xfffd -> boot block
0xfffe -> FAT
0xffff -> fim do arquivo
Informa¸c˜oes sobre a estrutura das entradas de diret´orio:
18 bytes -> nome do arquivo
1 byte -> atributo do arquivo
7 bytes -> reservado
2 bytes -> numero do primeiro cluster ocupado
4 bytes -> tamanho do arquivo
Byte de atributo do arquivo - valor: 0 - arquivo, 1 - diret´orio*/

/* entrada de diretorio, 32 bytes cada */
typedef struct
{
	uint8_t filename[18];
	uint8_t attributes;
	uint8_t reserved[7];
	uint16_t first_block;
	uint32_t size;
} dir_entry_t;

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
uint16_t fat[4096];
/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
union
{
	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
	uint8_t data[CLUSTER_SIZE];
} data_cluster;

dir_entry_t root_dir[32]; // diretorio raiz

//assinatura das funções
int init();
int load();
char *getComando(char *linhaComando);
void printDir();